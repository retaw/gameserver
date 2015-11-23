#include "scene_manager.h"
#include "world.h"
#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"
#include "water/componet/string_kit.h"
#include "water/componet/format.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include  "protocol/rawmsg/private/role_scene.h"
#include  "protocol/rawmsg/private/role_scene.codedef.private.h"

namespace world{

SceneManager& SceneManager::me()
{
    static SceneManager me;
    return me;
}

void SceneManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(CreateDynamicScene, std::bind(&SceneManager::servemsg_CreateDynamicScene, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(DestroyDynamicScene, std::bind(&SceneManager::servemsg_DestroyDynamicScene, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RetCreateDynamicSceneToOther, std::bind(&SceneManager::servemsg_RetCreateDynamicSceneToOther, this, _1, _2, _3));
    
}

void SceneManager::loadConfig(const std::string& cfgDir)
{
    //加载景主配置
    loadWorldConfig(cfgDir);

    //加载完主配置, 依据主配置的数据, 加载相应的地图资源
    for(auto& item : m_cfg.allMaps)
    {
        loadMapConfig(item.first, cfgDir);
        if(m_cfg.staticMapIds.find(item.first) != m_cfg.staticMapIds.end())
            createStaticScene(item.first);
    }
}

void SceneManager::loadWorldConfig(const std::string& cfgDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgDir + "/world_maps.xml";
    LOG_TRACE("读取world配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    //主配置, 得到本场景相关的全部地图Id
    for(XmlParseNode worldNode = root.getChild("world"); worldNode; ++worldNode)
    {   
        const auto num = worldNode.getAttr<uint16_t>("num");
        process::ProcessIdentity worldId("world", num);
        if(worldId.value() != World::me().getId().value())
            continue;

        LOG_TRACE("找到地图配置节点 {}", worldId);

        //静态图
        std::vector<MapId> staticMaplist;
        componet::fromString(&staticMaplist, worldNode.getAttr<std::string>("staticMaplist"), ",");
        for(MapId mapId : staticMaplist)
        {
            m_cfg.allMaps[mapId];
            m_cfg.staticMapIds.insert(mapId);
        }

        //动态图
        std::vector<MapId> dynamicMaplist;
        componet::fromString(&dynamicMaplist, worldNode.getAttr<std::string>("dynamicMaplist"), ",");
        for(MapId mapId : dynamicMaplist)
        {
            m_cfg.allMaps[mapId];
            m_cfg.dynamicMapIds.insert(mapId);
        }
        break;
    }

}

void SceneManager::loadMapConfig(MapId mapId, const std::string& cfgDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const auto fileName = componet::format("{id}.xml", mapId);
    const std::string cfgFile = cfgDir + "/maps/" + fileName;

    LOG_TRACE("读取map配置文件 {}", cfgFile);
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    //basic节点, 该节点必须存在
    XmlParseNode basicNode = root.getChild("basic");
    if(!basicNode)
        EXCEPTION(componet::ExceptionBase, cfgFile + ", basic节点不存在");

    Cfg::MapCfg& curMapCfg = m_cfg.allMaps[mapId];

    curMapCfg.basic.id     = mapId;
    curMapCfg.basic.name   = basicNode.getAttr<std::string>("name");
    curMapCfg.basic.width  = basicNode.getAttr<Coord1D>("width");
    curMapCfg.basic.height = basicNode.getAttr<Coord1D>("height");


    //block节点, 阻挡, 可选
    XmlParseNode blockAreaNode = root.getChild("blockArea");
    if(blockAreaNode)
        componet::fromString(&curMapCfg.blocks, blockAreaNode.getAttr<std::string>("gridList"), ";");

    //默认复活区
    XmlParseNode defaultReliveAreaNode = root.getChild("defaultReliveArea");
    if(defaultReliveAreaNode)
        componet::fromString(&curMapCfg.defaultReliveArea, defaultReliveAreaNode.getAttr<std::string>("gridList"), ";");

    //经验(泡点)区域
    XmlParseNode expAreaNode = root.getChild("expArea");
    if(expAreaNode)
        componet::fromString(&curMapCfg.expArea, expAreaNode.getAttr<std::string>("gridList"), ";");

    //安全区域
    XmlParseNode safeAreaNode = root.getChild("safeArea");
    if(safeAreaNode)
        componet::fromString(&curMapCfg.safeArea, safeAreaNode.getAttr<std::string>("gridList"), ";");

    //有碰撞(不可叠加)区域
    XmlParseNode collisionAreaNode = root.getChild("collisionArea");
    if(collisionAreaNode)
        componet::fromString(&curMapCfg.collisionArea, collisionAreaNode.getAttr<std::string>("gridList"), ";");

    //transmission节点, 传送点, 可选
    XmlParseNode transmissionNode = root.getChild("transmission");
    if(transmissionNode)
    {
        for(XmlParseNode itemNode = transmissionNode.getChild("item"); itemNode; ++itemNode)
        {
            const int32_t id = itemNode.getAttr<int32_t>("id");
            Scene::Transmission& transmission = curMapCfg.transmissions[id];

            transmission.grid.fromString(itemNode.getAttr<std::string>("grid"));
            transmission.maxDistance = itemNode.getAttr<Coord1D>("maxDistance");
            transmission.destinationMapId = itemNode.getAttr<MapId>("destinationMap");
            transmission.destinationGrid.fromString(itemNode.getAttr<std::string>("destinationGrid"));
        }
    }

    //npc节点, 随地图召唤的npc, 可选
    XmlParseNode npcNode = root.getChild("npc");
    if(npcNode)
    {
        for(XmlParseNode itemNode = npcNode.getChild("item"); itemNode; ++itemNode)
        {
            curMapCfg.npcAreas.emplace_back();
            Cfg::MapCfg::NpcArea& npcArea = curMapCfg.npcAreas.back();

            npcArea.npcTplId = itemNode.getAttr<NpcTplId>("npcTplId");
            npcArea.npcSize = itemNode.getAttr<uint32_t>("npcSize");
            componet::fromString(&npcArea.grids, itemNode.getAttr<std::string>("areaGridList"), ";");
        }
    }

}

void SceneManager::createStaticScene(MapId mapId)
{
    //静态场景, sceneId 的值等于 mapId
    SceneId sceneId = mapId;

    //启动时调用, 不要抓, 让它宕掉
    if(createScene(sceneId) == nullptr)
        EXCEPTION(CreateStaticSceneFailed, "mapID={}", mapId);
}


Scene::Ptr SceneManager::createScene(SceneId sceneId)
{
    MapId mapId = sceneId2MapId(sceneId);
    auto it = m_cfg.allMaps.find(mapId);
    if(it == m_cfg.allMaps.end())
    {
        LOG_TRACE("创建场景, 地图配置没找到, mapId={}", mapId);
        return nullptr;
    }

    const auto& cfg = it->second;
    auto scene = Scene::create(sceneId, cfg.basic);
    scene->setMapTpl(mapId);
    scene->setReliveArea(cfg.defaultReliveArea);
    scene->setTransmissions(cfg.transmissions);
    scene->setMapBlocks(cfg.blocks);
	scene->setArea(cfg.expArea, AreaType::exp);
	scene->setArea(cfg.safeArea, AreaType::security);
	scene->setArea(cfg.collisionArea, AreaType::collision);
	for(const Cfg::MapCfg::NpcArea& area : cfg.npcAreas)
        scene->summonStaticNpcs(area.npcTplId, area.npcSize, area.grids);

    if(m_scenes.insert(std::make_pair(sceneId, scene)).second == false)
    {
        LOG_TRACE("创建场景, id重复, sceneId={}", sceneId);
        return nullptr;
    }
    LOG_TRACE("创建场景, sceneId={}, mapId={}", sceneId, mapId);
    return scene;
}

std::unordered_map<NpcTplId, uint32_t> SceneManager::getNpcTplSize(SceneId sceneId)
{
    std::unordered_map<NpcTplId, uint32_t> ret;
    MapId mapId = sceneId2MapId(sceneId);
    auto it = m_cfg.allMaps.find(mapId);
    if(it == m_cfg.allMaps.end())
    {
        LOG_TRACE("获取场景所有npcTpl对应个数, 地图配置没找到, mapId={}", mapId);
        return ret;
    }
    for(auto npcAreas : it->second.npcAreas)
    {
        ret[npcAreas.npcTplId] += npcAreas.npcSize;
    }

    return ret;
}

Scene::Ptr SceneManager::getById(SceneId sceneId) const
{
    auto it = m_scenes.find(sceneId);
    if(it == m_scenes.end())
        return nullptr;

    return it->second;
}

void SceneManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(60),
                         std::bind(&SceneManager::timerExec, this, StdInterval::min_1, _1));
    World::me().regTimer(std::chrono::seconds(1),
                         std::bind(&SceneManager::timerExec, this, StdInterval::sec_1, _1));
}

void SceneManager::timerExec(StdInterval interval, const componet::TimePoint& now)
{
    for(auto it = m_scenes.begin(); it != m_scenes.end(); )
    {
        auto scene = it->second;
        scene->timerExec(interval, now);

        //场景失效
        if(!scene->avaliable())
        {
            //动态场景
            if(it->second->mapTpl()->type == CopyMap::private_boss)
                destroyDynamicScene(scene->id());
            //静态场景,暂时没有
        }

        ++it;
    }
}

void SceneManager::servemsg_CreateDynamicScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::CreateDynamicScene*>(msgData);
    auto scene = createScene(rev->sceneId);

    PrivateRaw::RetCreateDynamicScene send;

    send.isSuccessful = (scene == nullptr ? false : true);
    send.sceneId = rev->sceneId;

    World::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetCreateDynamicScene), &send, sizeof(send));
    LOG_TRACE("创建副本成功, sceneId={}", rev->sceneId);
}

void SceneManager::servemsg_DestroyDynamicScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::DestroyDynamicScene*>(msgData);

    auto it = m_scenes.find(rev->sceneId);
    if(it == m_scenes.end())
        return;

    it->second->cleanup();
    m_scenes.erase(rev->sceneId);
    LOG_TRACE("销毁副本, sceneId={}", rev->sceneId);

}

void SceneManager::servemsg_RetCreateDynamicSceneToOther(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RetCreateDynamicSceneToOther*>(msgData);

    auto it = m_dynamicSceneFunctions.find(rev->key);
    if(it == m_dynamicSceneFunctions.end())
    {
        LOG_ERROR("动态场景创建请求, 收到不存在的请求恢复key={}", rev->key);
        return;
    }
    
    //执行对应的回调
    it->second(rev->sceneId);
}

void SceneManager::createDynamicScene(MapId mapId, const std::function<void (SceneId)>& callback)
{
    //态场景回调key为最大的key++
    uint64_t key = getDynamicSceneFunctionKey();
    m_dynamicSceneFunctions.insert({key, callback});

    //通知func创建场景
    PrivateRaw::ReqCreateDynamicSceneToSession send;
    send.key = key;
    send.mapId = mapId;
    ProcessIdentity sessionId("session", 1);
    World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(ReqCreateDynamicSceneToSession), &send, sizeof(send));
}

void SceneManager::destroyDynamicScene(SceneId sceneId)
{
    PrivateRaw::ReqDestroyDynamicSceneToSession send;
    send.sceneId = sceneId;
    ProcessIdentity sessionId("session", 1);
    World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(ReqDestroyDynamicSceneToSession), &send, sizeof(send));
}

uint64_t SceneManager::getDynamicSceneFunctionKey()
{
    if(m_dynamicSceneFunctions.empty())
    {
        ProcessIdentity pid = World::me().getId();
        ProcessNum num = pid.num();
        ProcessType type = pid.type();
        uint64_t key = (uint64_t(num) << 48u) + (uint64_t(type) << 32u) +1;
        return key;
    }
    uint32_t key = m_dynamicSceneFunctions.end()->first + 1;
    return key;
}

} //end namespace


