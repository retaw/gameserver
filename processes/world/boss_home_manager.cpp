#include "boss_home_manager.h"
#include "npc.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/boss_home.codedef.public.h"
#include "protocol/rawmsg/public/boss_home.h"
#include "protocol/rawmsg/public/field_boss.codedef.public.h"
#include "protocol/rawmsg/public/field_boss.h"

#include "protocol/rawmsg/private/boss_home.codedef.private.h"
#include "protocol/rawmsg/private/boss_home.h"


#include "roles_and_scenes.h"
#include "scene_manager.h"
#include "water/componet/string_kit.h"
#include "water/componet/coord.h"

namespace world{

BossHomeManager& BossHomeManager::me()
{
    static BossHomeManager me;
    return me;
}

void BossHomeManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(TransFerToBossHome, std::bind(&BossHomeManager::clientmsg_TransFerToBossHome, this, _1, _2, _3));
    //REG_RAWMSG_PUBLIC(BossHomeInCurrentScene, std::bind(&BossHomeManager::clientmsg_BossHomeInCurrentScene, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(LeaveBossHome, std::bind(&BossHomeManager::clientmsg_LeaveBossHome, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(TransFerToNextBossHome, std::bind(&BossHomeManager::clientmsg_TransFerToNextBossHome, this, _1, _2, _3));
}

void BossHomeManager::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    std::string cfgFile = cfgdir + "/boss_home.xml";
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    std::vector<std::string> transferVec;
    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        BossHomeTpl::Ptr bossPtr = BossHomeTpl::create();
        bossPtr->bossId = itemNode.getChildNodeText<uint32_t>("bossId");
        bossPtr->npcId = itemNode.getChildNodeText<uint32_t>("npcId");
        bossPtr->mapId = itemNode.getChildNodeText<uint16_t>("mapId");
        bossPtr->transferMapId = itemNode.getChildNodeText<uint16_t>("transfer");
        bossPtr->refreshTime = itemNode.getChildNodeText<uint32_t>("reliveSec");
        bossPtr->deadTime = 0;
        m_bossHome.emplace(bossPtr->bossId, bossPtr);
        m_npcTplIdMapId2BossId.emplace(std::make_pair(bossPtr->npcId, bossPtr->mapId), bossPtr->bossId);
    }

    cfgFile = cfgdir + "/boss_home_map.xml";
    XmlParseDoc mapDoc(cfgFile);
    root = mapDoc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        m_mapId.insert(itemNode.getChildNodeText<uint16_t>("mapId"));
    }
    if(m_mapId.size() != 3)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

}

void BossHomeManager::afterEnterScene(Role::Ptr role)
{
    if(role->scene()->mapTpl()->type != CopyMap::boss_home)
        return;

    //返回右侧面板(当前场景的所有boss数据)
    auto mapId = role->scene()->mapTpl()->id;
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetBossHomeInCurrentScene));
    uint16_t i = 0;
    for(auto& it : m_bossHome)
    {
        if(it.second->mapId != mapId)
            continue;
        buf.resize(buf.size() + sizeof(PublicRaw::RetBossHomeInCurrentScene::BossHomeInCurrentSceneData));
        auto msg = reinterpret_cast<PublicRaw::RetBossHomeInCurrentScene*>(buf.data());
        msg->data[i].bossId = it.first;
        msg->data[i].refreshSeconds = getRefreshTime(it.second);
        i++;
    }
    auto msg = reinterpret_cast<PublicRaw::RetBossHomeInCurrentScene*>(buf.data());
    msg->size = i;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetBossHomeInCurrentScene), buf.data(), buf.size());
}

void BossHomeManager::clientmsg_TransFerToBossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto rev = reinterpret_cast<const PublicRaw::TransFerToBossHome*>(msgData);
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    
    auto it = m_bossHome.find(rev->bossId);
    if(it == m_bossHome.end())
        return;

    auto newScene = SceneManager::me().getById(it->second->transferMapId);
    auto newPos = Coord2D(0, 0);
    if(newScene != nullptr)
    {
        newPos = newScene->randAvailablePos(SceneItemType::role);
    }
    //传送
    bool tansFerSuccess = RolesAndScenes::me().gotoOtherScene(roleId, it->second->transferMapId, newPos);
    if(tansFerSuccess)
    {   
        //成功跳场景返回给客户端
        PublicRaw::TransFerToFieldBossSuccess send;
        role->sendToMe(RAWMSG_CODE_PUBLIC(TransFerToFieldBossSuccess), &send, 0);
    }   
}

void BossHomeManager::clientmsg_TransFerToNextBossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->scene() == nullptr)
        return;

    //判断传到哪一地图
    auto mapId = role->scene()->mapTpl()->id;
    auto it = m_mapId.find(mapId);
    SceneId toSceneId;
    if(it == m_mapId.end())     //说明不在boss之家，需要读取
    {
        return;
    }
    else    //要传到2,3层
    {
        if(*it == *m_mapId.rbegin())  //在第三层还要传递，非法
            return;
        toSceneId = *(++it);
    }

    auto newScene = SceneManager::me().getById(toSceneId);
    auto newPos = Coord2D(0, 0);
    if(newScene != nullptr)
    {
        newPos = newScene->randAvailablePos(SceneItemType::role);
    }

    //传送
    RolesAndScenes::me().gotoOtherScene(roleId, toSceneId, newPos);
}

void BossHomeManager::toDie(const uint32_t npcTplId, const MapId mapId, const std::vector<uint32_t>& objIdVec, Role::Ptr role)
{
    auto it = m_npcTplIdMapId2BossId.find(std::make_pair(npcTplId, mapId));
    if(it == m_npcTplIdMapId2BossId.end())
    {
        LOG_ERROR("boss之家boss死亡, 在boss表中未找到该boss, npcId={}, mapId={}", npcTplId, mapId);
        return;
    } 
    //设置死亡时间
    auto bossPair = m_bossHome.find(it->second);
    if(bossPair == m_bossHome.end())
        return;
    bossPair->second->deadTime = std::time(NULL);

    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PrivateRaw::BossHomeToDie));
    auto msg = reinterpret_cast<PrivateRaw::BossHomeToDie*>(buf.data());
    msg->bossId = it->second;
    msg->killerId = role->id();
    msg->size = objIdVec.size();
    msg->deadTime = bossPair->second->deadTime;
    for(uint16_t i = 0; i < objIdVec.size(); i++)
    {
        buf.resize(buf.size() + sizeof(uint32_t));
        msg = reinterpret_cast<PrivateRaw::BossHomeToDie*>(buf.data());
        msg->objId[i] = objIdVec[i];
    }
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(BossHomeToDie), buf.data(), buf.size());
}

void BossHomeManager::refreshbossHome(const uint32_t npcTplId, MapId mapId)
{
    auto it = m_npcTplIdMapId2BossId.find(std::make_pair(npcTplId, mapId));
    if(it == m_npcTplIdMapId2BossId.end())
    {
        LOG_ERROR("boss之家boss刷新, 在boss表中未找到该boss, npcId={}, mapId={}", npcTplId, mapId);
        return; 
    }
    
    PrivateRaw::RefreshBossHome send;
    send.bossId = it->second; 
                                
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(RefreshBossHome), &send, sizeof(send));
}

void BossHomeManager::clientmsg_LeaveBossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    RolesAndScenes::me().gotoOtherScene(roleId, role->preSceneId(), role->prePos());
}

uint32_t BossHomeManager::getRefreshTime(BossHomeTpl::Ptr bossHomeTpl)
{
    if(bossHomeTpl->deadTime == 0)
        return bossHomeTpl->deadTime;
    time_t now = std::time(NULL);
    uint32_t deadTime = now - bossHomeTpl->deadTime;

    return (bossHomeTpl->refreshTime > deadTime)?(bossHomeTpl->refreshTime - deadTime):0;
}

}
