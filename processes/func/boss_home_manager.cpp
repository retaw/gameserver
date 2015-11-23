#include "boss_home_manager.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/boss_home.codedef.public.h"
#include "protocol/rawmsg/public/boss_home.h"
#include "protocol/rawmsg/private/boss_home.codedef.private.h"
#include "protocol/rawmsg/private/boss_home.h" 

#include "water/componet/xmlparse.h"
#include "water/componet/logger.h" 

namespace func{


BossHomeManager& BossHomeManager::me()
{
    static BossHomeManager me;
    return me;
}

void BossHomeManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(BossHome, std::bind(&BossHomeManager::clientmsg_BossHome, this, _1, _2, _3));
    //REG_RAWMSG_PRIVATE(BossHomeInCurrentScene, std::bind(&BossHomeManager::servermsg_BossHomeInCurrentScene, this, _1, _2));
    REG_RAWMSG_PRIVATE(BossHomeToDie, std::bind(&BossHomeManager::servermsg_BossHomeToDie, this, _1, _2));
    REG_RAWMSG_PRIVATE(RefreshBossHome, std::bind(&BossHomeManager::servermsg_RefreshBossHome, this, _1, _2));
}

void BossHomeManager::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/boss_home.xml";
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode) 
    {
        BossHomeTpl::Ptr bossPtr = BossHomeTpl::create();
        bossPtr->bossId = itemNode.getChildNodeText<uint32_t>("bossId");
        bossPtr->mapId = itemNode.getChildNodeText<uint32_t>("mapId");
        bossPtr->refreshTime = itemNode.getChildNodeText<uint32_t>("reliveSec");
        bossPtr->deadTime = 0;
        m_bossHome.emplace(bossPtr->bossId, bossPtr);
    }

}

void BossHomeManager::clientmsg_BossHome(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetBossHome));

    uint16_t i = 0;
    for(auto& it : m_bossHome)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetBossHome::BossHomeData));
        auto msg = reinterpret_cast<PublicRaw::RetBossHome*>(buf.data());
        msg->data[i].bossId = it.first;
        msg->data[i].refreshSeconds = getRefreshTime(it.second);

        i++;
    }
    auto msg = reinterpret_cast<PublicRaw::RetBossHome*>(buf.data());
    msg->mohun = 0;
    msg->size = i;

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetBossHome), buf.data(), buf.size());
}

void BossHomeManager::servermsg_RefreshBossHome(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RefreshBossHome*>(msgData);
    auto it = m_bossHome.find(rev->bossId);
    if(it == m_bossHome.end())
       return;

    auto bossptr = it->second;
    bossptr->deadTime = 0;
    PublicRaw::RefreshBossHome send;
    send.bossId = rev->bossId;
    for(Role::Ptr role : RoleManager::me())
    {
        if(role == nullptr)
            continue;
        role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshBossHome), &send, sizeof(send));
    }
}

void BossHomeManager::servermsg_BossHomeToDie(const uint8_t* msgData, uint32_t msgSize)
{
   auto rev = reinterpret_cast<const PrivateRaw::BossHomeToDie*>(msgData); 
   auto it = m_bossHome.find(rev->bossId);
   if(it == m_bossHome.end())
       return;

   auto role = RoleManager::me().getById(rev->killerId);
   if(role == nullptr)
       return;

   auto bossptr = it->second;
   bossptr->deadTime = rev->deadTime;

   std::vector<uint8_t> buf;
   buf.reserve(1024);
   buf.resize(sizeof(PublicRaw::KillBossHome));
   auto send = reinterpret_cast<PublicRaw::KillBossHome*>(buf.data());
   send->roleId = rev->killerId;
   send->bossId = rev->bossId;
   send->refreshSeconds = getRefreshTime(bossptr);
   std::memset(send->killerName, 0, NAME_BUFF_SZIE);
   role->name().copy(send->killerName, NAME_BUFF_SZIE);
   send->size = rev->size;
   for(uint16_t i = 0; i < rev->size; i++)
   {
        buf.resize(buf.size() + sizeof(uint32_t));
        auto send = reinterpret_cast<PublicRaw::KillBossHome*>(buf.data());
        send->objId[i] = rev->objId[i];
   }

   for(Role::Ptr role : RoleManager::me())
   {
        if(role == nullptr)
            continue;
        role->sendToMe(RAWMSG_CODE_PUBLIC(KillBossHome), buf.data(), buf.size());
   }
}

/*
void BossHomeManager::servermsg_BossHomeInCurrentScene(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::BossHomeInCurrentScene*>(msgData); 
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetBossHomeInCurrentScene));
    uint16_t i = 0;
    for(auto& it : m_bossHome)
    {
        if(it.second->mapId != rev->mapId)
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
*/

uint32_t BossHomeManager::getRefreshTime(BossHomeTpl::Ptr bossHomeTpl)
{
    if(bossHomeTpl->deadTime == 0)
        return bossHomeTpl->deadTime;
    time_t now = std::time(NULL);
    uint32_t deadTime = now - bossHomeTpl->deadTime;

    return (bossHomeTpl->refreshTime > deadTime)?(bossHomeTpl->refreshTime - deadTime):0;

}

}
