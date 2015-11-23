#include "field_boss_manager.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include "protocol/rawmsg/public/field_boss.codedef.public.h"
#include "protocol/rawmsg/public/field_boss.h"

#include "protocol/rawmsg/private/field_boss.codedef.private.h"
#include "protocol/rawmsg/private/field_boss.h"

#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"

namespace func{

FieldBossManager& FieldBossManager::me()
{
    static FieldBossManager me;
    return me;
}

void FieldBossManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(FieldBoss, std::bind(&FieldBossManager::clientmsg_FieldBoss, this, _1,_2, _3));
    REG_RAWMSG_PRIVATE(FieldBossToDie, std::bind(&FieldBossManager::servermsg_FieldBossToDie, this, _1, _2));
    REG_RAWMSG_PRIVATE(RefreshFieldBoss, std::bind(&FieldBossManager::servermsg_RefreshFieldBoss, this, _1, _2));
}

void FieldBossManager::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;
    const std::string cfgFile = cfgdir + "/field_boss.xml";
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");
    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        FieldBossTpl::Ptr bossPtr = FieldBossTpl::create();
        bossPtr->bossId = itemNode.getChildNodeText<uint32_t>("bossId");
        bossPtr->refreshTime = itemNode.getChildNodeText<uint32_t>("reliveSec");
        bossPtr->deadTime = 0;
        m_fieldBoss.emplace(bossPtr->bossId, bossPtr);
    }

}

void FieldBossManager::clientmsg_FieldBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetFieldBoss));
    uint16_t i = 0;
    for(auto& it : m_fieldBoss)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetFieldBoss::BossData));
        auto msg = reinterpret_cast<PublicRaw::RetFieldBoss*>(buf.data());
        msg->data[i].bossId = it.first;
        msg->data[i].refreshSeconds = getRefreshTime(it.second);

        i++;
    }
    auto msg = reinterpret_cast<PublicRaw::RetFieldBoss*>(buf.data());
    msg->mohun = 0;///role->mohun();
    msg->size = i;

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetFieldBoss), buf.data(), buf.size());
}

void FieldBossManager::servermsg_RefreshFieldBoss(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RefreshFieldBoss*>(msgData);
    auto it = m_fieldBoss.find(rev->bossId);
    if(it == m_fieldBoss.end())
        return;
    //同步数据
    auto bossptr = it->second;
    bossptr->deadTime = 0;
    PublicRaw::RefreshFieldBoss send;
    send.bossId = rev->bossId;
    for(Role::Ptr role : RoleManager::me())
    {
        if(role == nullptr)
            continue;
        role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshFieldBoss), &send, sizeof(send));
    }
}

void FieldBossManager::servermsg_FieldBossToDie(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::FieldBossToDie*>(msgData);
    auto it = m_fieldBoss.find(rev->bossId);
    if(it == m_fieldBoss.end())
        return;

    auto role = RoleManager::me().getById(rev->killerId);
    if(role == nullptr)
        return;
    //同步数据
    auto bossptr = it->second;
    bossptr->deadTime = std::time(NULL);
    {
        std::vector<uint8_t> buf;
        buf.reserve(1024);
        buf.resize(sizeof(PublicRaw::KillFieldBoss));
        auto send = reinterpret_cast<PublicRaw::KillFieldBoss*>(buf.data());
        send->roleId = rev->killerId; 
        send->bossId = rev->bossId;
        send->refreshSeconds = getRefreshTime(bossptr);
        std::memset(send->killerName, 0, NAME_BUFF_SZIE);
        role->name().copy(send->killerName, NAME_BUFF_SZIE);
        send->size = rev->size;
        for(uint16_t i = 0; i < rev->size; i++)
        {
            buf.resize(buf.size() + sizeof(uint32_t));
            auto send = reinterpret_cast<PublicRaw::KillFieldBoss*>(buf.data());
            send->objId[i] = rev->objId[i];
        }
        
        for(Role::Ptr role : RoleManager::me())
        {
            if(role == nullptr)
                continue;

            role->sendToMe(RAWMSG_CODE_PUBLIC(KillFieldBoss), buf.data(), buf.size());
        }
    }
}

uint32_t FieldBossManager::getRefreshTime(FieldBossTpl::Ptr fieldBossTpl)
{
    if(fieldBossTpl->deadTime == 0)
        return fieldBossTpl->deadTime;
    time_t now = std::time(NULL);
    uint32_t deadTime = now - fieldBossTpl->deadTime;

    return (fieldBossTpl->refreshTime > deadTime)?(fieldBossTpl->refreshTime - deadTime):0;
}

}
