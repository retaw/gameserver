#include "scene_dispenser.h"

#include "session.h"

#include "componet/logger.h"
#include "componet/xmlparse.h"
#include "componet/format.h"
#include "componet/string_kit.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace session{

SceneDispenser& SceneDispenser::me()
{
    static SceneDispenser me;
    return me;
}


void SceneDispenser::loadConfig(const std::string& cfgDir)
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;


    const std::string configFile = cfgDir + "/world_maps.xml";

    LOG_TRACE("读取配置文件 {}", configFile);

    XmlParseDoc doc(configFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, configFile + " parse root node failed");

    
    for(XmlParseNode worldNode = root.getChild("world"); worldNode; ++worldNode)
    {
        const auto num = worldNode.getAttr<uint16_t>("num");
        process::ProcessIdentity worldId("world", num);

        //静态图
        std::vector<MapId> staticMaplist;
        componet::fromString(&staticMaplist, worldNode.getAttr<std::string>("staticMaplist"), ",");
        for(MapId mid : staticMaplist)
            m_cfg.staticMapId2WorldId[mid] = worldId;

        //动态图
        std::vector<MapId> dynamicMaplist;
        componet::fromString(&dynamicMaplist, worldNode.getAttr<std::string>("dynamicMaplist"), ",");
        for(MapId mid : dynamicMaplist)
            m_cfg.dynamicMapId2WorldId[mid].insert(worldId);
    }

    //配置文件加载完成, 依据配置生成静态地图的进程映射
    for(const auto& item : m_cfg.staticMapId2WorldId)
    {
        SceneId staticSceneId = item.first; //静态场景id == 地图id
        m_sceneInWorld[staticSceneId] = item.second; 
        LOG_DEBUG("静态场景映射: {} -> {}", staticSceneId, item.second);
    }
}

process::ProcessIdentity SceneDispenser::sceneId2WorldId(SceneId sceneId) const
{
    auto it = m_sceneInWorld.find(sceneId);
    if(it == m_sceneInWorld.end())
        return ProcessIdentity(process::INVALID_PROCESS_IDENDITY_VALUE);

    return it->second;
}

void SceneDispenser::createDynamicScene(MapId mapId, const std::function<void (SceneId)>& callback)
{
    //可以用这个mapId创建scene的worlds
    auto it = m_cfg.dynamicMapId2WorldId.find(mapId);
    if(it == m_cfg.dynamicMapId2WorldId.end())
    {
        LOG_TRACE("创建副本失败, mapId非法, mapId={}", mapId);
        callback(INVALID_SCENE_ID);
        return;
    }
    if(it->second.empty())
    {
        LOG_TRACE("创建副本失败, 没有允许建立副本的world, mapId={}", mapId);
        callback(INVALID_SCENE_ID);
        return;
    }
    //随机挑选一个world
    std::vector<process::ProcessIdentity> availableWorlds(it->second.begin(), it->second.end());
    componet::Random<decltype(availableWorlds.size())> rander(0, availableWorlds.size() - 1);
    const process::ProcessIdentity worldId = availableWorlds.at(rander.get());

    //生成sceneId
    const SceneId sceneId = /*(componet::Clock::now() << 32u) + */(SceneId(++m_lastDynamicSceneCounter) << 16u) + mapId;

    //发消息到所选的world
    PrivateRaw::CreateDynamicScene send;
    send.sceneId = sceneId;
    Session::me().sendToPrivate(worldId, RAWMSG_CODE_PRIVATE(CreateDynamicScene), &send, sizeof(send));

    m_createDynamicSceneCallbacks[sceneId] = callback;
}

void SceneDispenser::destroyDynamicScene(SceneId sceneId)
{
    auto it = m_sceneInWorld.find(sceneId);
    if(it == m_sceneInWorld.end())
        return;

    //通知world销毁已创建的副本实体
    PrivateRaw::DestroyDynamicScene send;
    send.sceneId = sceneId;
    Session::me().sendToPrivate(it->second, RAWMSG_CODE_PRIVATE(DestroyDynamicScene), &send, sizeof(send));

    m_sceneInWorld.erase(it);
    return;
}

void SceneDispenser::servermsg_RetCreateDynamicScene(const uint8_t* data, uint32_t size, ProcessIdentity worldId)
{
    auto rev = reinterpret_cast<const PrivateRaw::RetCreateDynamicScene*>(data);

    auto it = m_createDynamicSceneCallbacks.find(rev->sceneId);
    if(it == m_createDynamicSceneCallbacks.end())
    {
        if(rev->isSuccessful)
        {
            //通知world销毁已创建的副本实体
            PrivateRaw::DestroyDynamicScene send;
            send.sceneId = rev->sceneId;
            Session::me().sendToPrivate(worldId, RAWMSG_CODE_PRIVATE(DestroyDynamicScene), &send, sizeof(send));
        }
        LOG_ERROR("创建副本失败, world创建完成后, session上却没有请求创建的记录, sceneId={}", rev->sceneId);
        return;
    }

    if(rev->isSuccessful)
    {
        if(m_sceneInWorld.insert(std::make_pair(rev->sceneId, worldId)).second == false)
        {
            //通知world销毁已创建的副本实体
            PrivateRaw::DestroyDynamicScene send;
            send.sceneId = rev->sceneId;
            Session::me().sendToPrivate(worldId, RAWMSG_CODE_PRIVATE(DestroyDynamicScene), &send, sizeof(send));
            LOG_ERROR("创建副本失败, sceneId索引已存在, sceneId={}", rev->sceneId);

            it->second(INVALID_SCENE_ID);
            return;
        }
        LOG_TRACE("创建副本成功, sceneId={}", rev->sceneId);
        it->second(rev->sceneId);
    }
    else
    {
        LOG_ERROR("创建副本失败, world创建失败 sceneId={}", rev->sceneId);
        it->second(INVALID_SCENE_ID);
    }
}

void SceneDispenser::servermsg_ReqCreateDynamicSceneToSession(const uint8_t* data, uint32_t size, ProcessIdentity worldId)
{
    auto rev = reinterpret_cast<const PrivateRaw::ReqCreateDynamicSceneToSession*>(data);
    auto key = rev->key;

    //设置回调函数
    auto func = [worldId, key] (SceneId sceneId) 
    {
        PrivateRaw::RetCreateDynamicSceneToOther send;
        send.key = key;
        send.sceneId = sceneId;
        Session::me().sendToPrivate(worldId, RAWMSG_CODE_PRIVATE(RetCreateDynamicSceneToOther), &send, sizeof(send));
    };

    //创建
    createDynamicScene(rev->mapId, func);
}

void SceneDispenser::servermsg_ReqDestroyDynamicSceneToSession(const uint8_t* data, uint32_t size, ProcessIdentity worldId)
{
    auto rev = reinterpret_cast<const PrivateRaw::ReqDestroyDynamicSceneToSession*>(data);

    //直接销毁即可
    destroyDynamicScene(rev->sceneId);
}

void SceneDispenser::regTimer()
{
    using namespace std::placeholders;
    Session::me().regTimer(std::chrono::seconds(10), std::bind(&SceneDispenser::timerExec, this, _1));
}

void SceneDispenser::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(RetCreateDynamicScene, std::bind(&SceneDispenser::servermsg_RetCreateDynamicScene, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(ReqCreateDynamicSceneToSession, std::bind(&SceneDispenser::servermsg_ReqCreateDynamicSceneToSession, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(ReqDestroyDynamicSceneToSession, std::bind(&SceneDispenser::servermsg_ReqDestroyDynamicSceneToSession, this, _1, _2, _3));
}

void SceneDispenser::timerExec(const componet::TimePoint& now)
{
}

}
