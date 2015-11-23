#include "mail_manager.h"
#include "role_manager.h"
#include "world.h"
#include "object_config.h"

#include "water/componet/logger.h"

#include "protocol/rawmsg/private/mail.h"
#include "protocol/rawmsg/private/mail.codedef.private.h"

#include "protocol/rawmsg/public/mail.h"
#include "protocol/rawmsg/public/mail.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

MailManager& MailManager::me()
{
    static MailManager me;
    return me;
}

void MailManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(NewMailToWorld, std::bind(&MailManager::servermsg_NewMailToWorld, this, _1));
    REG_RAWMSG_PRIVATE(ServerEraseMail, std::bind(&MailManager::servermsg_ServerEraseMail, this, _1));
    REG_RAWMSG_PUBLIC(RequestMailList, std::bind(&MailManager::clientmsg_RequestMailList, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestMailDetailInfo, std::bind(&MailManager::clientmsg_RequestMailDetailInfo, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestDeleteMail, std::bind(&MailManager::clientmsg_RequestDeleteMail, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestGetMailObj, std::bind(&MailManager::clientmsg_RequestGetMailObj, this, _1, _2, _3));
}

void MailManager::servermsg_NewMailToWorld(const uint8_t* msgData)
{
    auto rev = reinterpret_cast<const PrivateRaw::NewMailToWorld*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
        return;

    role->m_mail.addNewMail(rev->info);
}

void MailManager::servermsg_ServerEraseMail(const uint8_t* msgData)
{
    auto rev = reinterpret_cast<const PrivateRaw::ServerEraseMail*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
        return;

    role->m_mail.serverEraseMail();
}

void MailManager::clientmsg_RequestMailList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    role->m_mail.retMailList();
}

void MailManager::clientmsg_RequestMailDetailInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    auto rev = reinterpret_cast<const PublicRaw::RequestMailDetailInfo*>(msgData);
    role->m_mail.readMail(rev->index);
}

void MailManager::clientmsg_RequestDeleteMail(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto rev = reinterpret_cast<const PublicRaw::RequestDeleteMail*>(msgData);
    if(msgSize != sizeof(PublicRaw::RequestDeleteMail) + rev->size * sizeof(uint32_t))
    {
        LOG_ERROR("邮件, 请求删除邮件发现消息长度不对, msgSize={},size={}", 
                  msgSize, sizeof(PublicRaw::RequestDeleteMail) + rev->size * sizeof(uint32_t));
        return;
    }

    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    role->m_mail.requestDeleteMail(msgData, msgSize);
}

void MailManager::clientmsg_RequestGetMailObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto rev = reinterpret_cast<const PublicRaw::RequestGetMailObj*>(msgData);
    if(msgSize != sizeof(PublicRaw::RequestGetMailObj) + rev->size * sizeof(uint32_t))
    {
        LOG_ERROR("邮件, 提取附件发现消息长度不对, msgSize={},size={}", 
                  msgSize, sizeof(PublicRaw::RequestGetMailObj) + rev->size * sizeof(uint32_t));
        return;
    }

    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    role->m_mail.requestGetMailObj(msgData, msgSize);
}


void MailManager::send(const std::string& receiver, const std::string& title, const std::string& text, const std::vector<ObjItem>& mailObjs)
{
    PrivateRaw::WriteMailByName send;
    std::memset(&send, 0, sizeof(send));
    receiver.copy(send.receiver, sizeof(send.receiver)-1);
    title.copy(send.info.title, sizeof(send.info.title)-1);
    text.copy(send.info.text, sizeof(send.info.text)-1);
    send.info.time= toUnixTime(Clock::now());
    send.info.state = static_cast<uint8_t>(MailState::unopen);

    ProcessIdentity dbcachedId("dbcached", 1);
    if(mailObjs.empty())
    {
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);
        return;
    }

    uint32_t index = 0;
    for(const auto& iter : mailObjs)
    {
        if(iter.tplId == 0 || iter.num < 1)
            continue;
        if(ObjectConfig::me().getMaxStackNum(iter.tplId) <= 1
           && iter.num > 1)
        {
            for(uint32_t objcount = 0; objcount < iter.num; ++objcount)
            {
                send.info.obj[index] = iter;
                send.info.obj[index].num = 1;
                if(++index >= MAX_MAIL_OBJ_NUM)
                {
                    send.info.state |= static_cast<uint8_t>(MailState::existobj);
                    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
                    LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);

                    std::memset(send.info.obj, 0, sizeof(send.info.obj));
                    index = 0;
                }
            }
        }
        else
        {
            send.info.obj[index] = iter;
            if(++index >= MAX_MAIL_OBJ_NUM)
            {
                send.info.state |= static_cast<uint8_t>(MailState::existobj);
                World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
                LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);

                std::memset(send.info.obj, 0, sizeof(send.info.obj));
                index = 0;
            }
        }

    }
    //有剩余的,再发一次
    if(index > 0)
    {
        send.info.state |= static_cast<uint8_t>(MailState::existobj);
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);
    }
}

void MailManager::send(const std::string& receiver, const std::string& title, const std::string& text, const ObjItem& mailObj)
{
    PrivateRaw::WriteMailByName send;
    std::memset(&send, 0, sizeof(send));
    receiver.copy(send.receiver, sizeof(send.receiver)-1);
    title.copy(send.info.title, sizeof(send.info.title)-1);
    text.copy(send.info.text, sizeof(send.info.text)-1);
    send.info.time= toUnixTime(Clock::now());
    send.info.state = static_cast<uint8_t>(MailState::unopen);

    ProcessIdentity dbcachedId("dbcached", 1);
    if(mailObj.tplId == 0 || mailObj.num < 1)
    {
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);
        return;
    }

    uint16_t index = 0;
    send.info.state |= static_cast<uint8_t>(MailState::existobj);
    if(ObjectConfig::me().getMaxStackNum(mailObj.tplId) <= 1 && mailObj.num > 1)
    {
        for(uint32_t objcount = 0; objcount < mailObj.num; ++objcount)
        {
            send.info.obj[index] = mailObj;
            send.info.obj[index].num = 1;
            if(++index >= MAX_MAIL_OBJ_NUM)
            {
                World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
                LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);

                std::memset(send.info.obj, 0, sizeof(send.info.obj));
                index = 0;
            }
        }
    }
    else
        send.info.obj[index++] = mailObj;

    if(index > 0)
    {
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailByName), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 receiver={}, title={}", receiver, title);
    }
}

void MailManager::send(RoleId roleId, const std::string& title, const std::string& text, const std::vector<ObjItem>& mailObjs)
{
    PrivateRaw::WriteMailById send;
    std::memset(&send, 0, sizeof(send));
    send.roleId = roleId;
    title.copy(send.info.title, sizeof(send.info.title)-1);
    text.copy(send.info.text, sizeof(send.info.text)-1);
    send.info.time= toUnixTime(Clock::now());
    send.info.state = static_cast<uint8_t>(MailState::unopen);

    ProcessIdentity dbcachedId("dbcached", 1);
    if(mailObjs.empty())
    {
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);
        return;
    }

    uint32_t index = 0;
    for(const auto& iter : mailObjs)
    {
        if(iter.tplId == 0 || iter.num < 1)
            continue;
        if(ObjectConfig::me().getMaxStackNum(iter.tplId) <= 1
           && iter.num > 1)
        {
            for(uint32_t objcount = 0; objcount < iter.num; ++objcount)
            {
                send.info.obj[index] = iter;
                send.info.obj[index].num = 1;
                if(++index >= MAX_MAIL_OBJ_NUM)
                {
                    send.info.state |= static_cast<uint8_t>(MailState::existobj);
                    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
                    LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);

                    std::memset(send.info.obj, 0, sizeof(send.info.obj));
                    index = 0;
                }
            }
        }
        else
        {
            send.info.obj[index] = iter;
            if(++index >= MAX_MAIL_OBJ_NUM)
            {
                send.info.state |= static_cast<uint8_t>(MailState::existobj);
                World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
                LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);

                std::memset(send.info.obj, 0, sizeof(send.info.obj));
                index = 0;
            }
        }

    }
    //有剩余的,再发一次
    if(index > 0)
    {
        send.info.state |= static_cast<uint8_t>(MailState::existobj);
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);
    }
}

void MailManager::send(RoleId roleId, const std::string& title, const std::string& text, const ObjItem& mailObj)
{
    PrivateRaw::WriteMailById send;
    send.roleId = roleId;
    title.copy(send.info.title, sizeof(send.info.title)-1);
    text.copy(send.info.text, sizeof(send.info.text)-1);
    send.info.time= toUnixTime(Clock::now());
    send.info.state = static_cast<uint8_t>(MailState::unopen);

    ProcessIdentity dbcachedId("dbcached", 1);
    if(mailObj.tplId == 0 || mailObj.num < 1)
    {
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);
        return;
    }

    uint16_t index = 0;
    send.info.state |= static_cast<uint8_t>(MailState::existobj);
    if(ObjectConfig::me().getMaxStackNum(mailObj.tplId) <= 1 && mailObj.num > 1)
    {
        for(uint32_t objcount = 0; objcount < mailObj.num; ++objcount)
        {
            send.info.obj[index] = mailObj;
            send.info.obj[index].num = 1;
            if(++index >= MAX_MAIL_OBJ_NUM)
            {
                World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
                LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);

                std::memset(send.info.obj, 0, sizeof(send.info.obj));
                index = 0;
            }
        }
    }
    else
        send.info.obj[index++] = mailObj;

    if(index > 0)
    {
        World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WriteMailById), &send, sizeof(send));
        LOG_TRACE("邮件, 发送成功, 接收者 roleId={}, title={}", roleId, title);
    }
}

}

