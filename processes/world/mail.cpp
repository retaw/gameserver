#include "mail.h"
#include "role.h"
#include "world.h"

#include "protocol/rawmsg/private/mail.h"
#include "protocol/rawmsg/private/mail.codedef.private.h"

#include "protocol/rawmsg/public/mail.h"
#include "protocol/rawmsg/public/mail.codedef.public.h"

namespace world{

Mail::Mail(Role& me): m_owner(me)
{
}

void Mail::addNewMail(const MailInfo& info)
{
    for(const auto& iter : m_mailQueue)
    {
        if(iter.mailIndex == info.mailIndex)
            return;
    }

    m_mailQueue.push_front(info);
    notifyNewMail();
}

void Mail::notifyNewMail()
{
    for(const auto& info : m_mailQueue)
    {
        if(info.state & static_cast<uint8_t>(MailState::unopen))
        {
            PublicRaw::NotifyNewMail send;
            m_owner.sendToMe(RAWMSG_CODE_PUBLIC(NotifyNewMail), &send, sizeof(send));
            return;
        }
    }
}

void Mail::retMailList()
{
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetMailList));
    for(const auto& info : m_mailQueue)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetMailList::MailData));
        auto msg = reinterpret_cast<PublicRaw::RetMailList*>(buf.data());
        msg->data[msg->mailSize].index = info.mailIndex;
        strncpy(msg->data[msg->mailSize].title, info.title, sizeof(info.title)-1);
        msg->data[msg->mailSize].state = info.state;
        msg->data[msg->mailSize].sendtime = info.time;
        ++msg->mailSize;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetMailList), buf.data(), buf.size());
}

void Mail::readMail(uint32_t index)
{
    for(auto& info : m_mailQueue)
    {
        if(info.mailIndex == index)
        {
            PublicRaw::RetMailDetailInfo send;
            send.index = index;
            strncpy(send.title, info.title, sizeof(info.title)-1);
            strncpy(send.text, info.text, sizeof(info.text)-1);
            send.state = info.state;
            send.sendtime = info.time;
            for(auto i = 0; i < MAX_MAIL_OBJ_NUM; ++i)
            {
                send.obj[i].id = info.obj[i].tplId;
                send.obj[i].num = info.obj[i].num;
                send.obj[i].bind = static_cast<uint8_t>(info.obj[i].bind);
            }

            m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetMailDetailInfo), &send, sizeof(send));

            if(info.state & static_cast<uint8_t>(MailState::unopen))
            {
                info.state &= ~static_cast<uint8_t>(MailState::unopen);
                info.state |= static_cast<uint8_t>(MailState::opened);

                refreshMailState(info);
                updateMailDB(info);
            }
            return;
        }
    }
}

void Mail::requestGetMailObj(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PublicRaw::RequestGetMailObj*>(msgData);
    std::set<uint32_t> posList;
    std::set<uint32_t> putInPosList;
    for(ArraySize i = 0; i < rev->size; ++i)
    {
        uint32_t pos = 0;
        for(const auto& info : m_mailQueue)
        {
            if(info.mailIndex == rev->index[i] &&
               0 != (info.state & static_cast<uint8_t>(MailState::existobj)))
            {
                posList.insert(pos);
                break;
            }
            ++pos;
        }
    }

    if(0 == posList.size())
    {
        m_owner.sendSysChat("没有可以领取的附件");
        return;
    }

    bool full = false;
    for(const auto& pos : posList)
    {
        std::vector<ObjItem> objItems;
        MailInfo& info = m_mailQueue[pos];
        for(uint8_t objIndex = 0; objIndex < MAX_MAIL_OBJ_NUM; ++objIndex)
        {
            const ObjItem& obj = info.obj[objIndex];
            if(obj.tplId > 0 && obj.num > 0)
            {
                objItems.push_back(obj);
            }
        }

        if(m_owner.checkPutObj(objItems))
            m_owner.putObj(objItems);
        else
        {
            full = true;
            break;
        }

        putInPosList.insert(pos);
        //更新邮件状态
        info.state &= ~static_cast<uint8_t>(MailState::existobj);
    }


    if(full)
        m_owner.sendSysChat("背包没有足够的空余格子! 无法提取!");
    else
        m_owner.sendSysChat("成功领取邮件附件");
    LOG_TRACE("邮件, role ({}, {}) 成功提取{}封邮件附件", m_owner.name(), m_owner.id(), putInPosList.size());

    refreshMailState(putInPosList);
    updateMailDB(putInPosList);
}

void Mail::requestDeleteMail(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PublicRaw::RequestDeleteMail*>(msgData);
    std::set<uint32_t> posList;
    for(ArraySize i = 0; i < rev->size; ++i)
    {
        uint32_t pos = 0;
        for(auto& info : m_mailQueue)
        {
            if(info.mailIndex == rev->index[i])
            {
                posList.insert(pos);
                break;
            }
            ++pos;
        }
    }

    LOG_TRACE("邮件, role ({}, {}) 删除{}个邮件", m_owner.name(), m_owner.id(), rev->size);
    eraseMailDB(posList);
    for(const auto& pos : posList)
        m_mailQueue.erase(m_mailQueue.begin()+pos);

    retMailList();
}

void Mail::serverEraseMail()
{
    m_mailQueue.pop_back();
    retMailList();
}

void Mail::checkMailOverdue(const TimePoint& now)
{
    std::set<uint32_t> expireMail;
    uint32_t unix_now = toUnixTime(now);
    uint32_t pos = 0;
    for(const auto& info : m_mailQueue)
    {
        if(SAFE_SUB(unix_now, info.time) >= 7 * 24 * 60 * 60) // 7 Day
            expireMail.insert(pos);
        ++pos;
    }

    eraseMailDB(expireMail);
    for(const auto& pos : expireMail)
        m_mailQueue.erase(m_mailQueue.begin() + pos);
}

void Mail::load(std::vector<MailInfo>& mailQueue)
{
    uint32_t now = toUnixTime(Clock::now());
    std::vector<uint32_t> expireMail;
    for(const auto& iter : mailQueue)
    {
        if(SAFE_SUB(now, iter.time) >= 7 * 24 * 60 * 60) // 7 Day
        {
            expireMail.push_back(iter.mailIndex);
            continue;
        }

        MailInfo info;
        info.time = iter.time;
        info.mailIndex = iter.mailIndex;
        strncpy(info.title, iter.title, sizeof(info.title)-1);
        strncpy(info.text, iter.text, sizeof(info.text)-1);
        info.state = iter.state;

        for(auto i = 0; i < MAX_MAIL_OBJ_NUM; ++i)
        {
            info.obj[i] = iter.obj[i];
        }

        m_mailQueue.push_back(info);
    }

    struct MailOrder
    {
        bool operator () (const MailInfo& lhs, const MailInfo& rhs) const
        {
            return lhs.time > rhs.time;
        }
    };
    std::sort(m_mailQueue.begin(), m_mailQueue.end(), MailOrder());

    //过期删除
    eraseMailDB(expireMail);
}

void Mail::updateMailDB(std::set<uint32_t>& posList)
{
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::UpdateMail));
    auto msg = reinterpret_cast<PrivateRaw::UpdateMail*>(buf.data());
    msg->roleId = m_owner.id();
    for(const auto& pos : posList)
    {
        buf.resize(buf.size() + sizeof(PrivateRaw::UpdateMail::MailModify));
        auto msg = reinterpret_cast<PrivateRaw::UpdateMail*>(buf.data());
        msg->data[msg->size].mailIndex = m_mailQueue[pos].mailIndex;
        msg->data[msg->size].state = m_mailQueue[pos].state;
        ++msg->size;
    }

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateMail), buf.data(), buf.size());
}

void Mail::updateMailDB(const MailInfo& info)
{
    std::vector<uint8_t> buf;
    buf.reserve(64);
    buf.resize(sizeof(PrivateRaw::UpdateMail) + sizeof(PrivateRaw::UpdateMail::MailModify));
    auto msg = reinterpret_cast<PrivateRaw::UpdateMail*>(buf.data());
    msg->roleId = m_owner.id();
    msg->size = 1;
    msg->data[0].mailIndex = info.mailIndex;
    msg->data[0].state = info.state;

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateMail), buf.data(), buf.size());
}

void Mail::eraseMailDB(std::set<uint32_t>& posList)
{
    if(posList.empty())
        return;
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::EraseMail));
    auto msg = reinterpret_cast<PrivateRaw::EraseMail*>(buf.data());
    msg->roleId = m_owner.id();
    for(const auto& pos : posList)
    {
        buf.resize(buf.size() + sizeof(uint32_t));
        auto msg = reinterpret_cast<PrivateRaw::EraseMail*>(buf.data());
        msg->mailIndex[msg->size] = m_mailQueue[pos].mailIndex;
        ++msg->size;
    }

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(EraseMail), buf.data(), buf.size());
}

void Mail::eraseMailDB(uint32_t mailIndex)
{
    std::vector<uint8_t> buf;
    buf.reserve(64);
    buf.resize(sizeof(PrivateRaw::EraseMail) + sizeof(uint32_t));
    auto msg = reinterpret_cast<PrivateRaw::EraseMail*>(buf.data());
    msg->roleId = m_owner.id();
    msg->size = 1;
    msg->mailIndex[0] = mailIndex;

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(EraseMail), buf.data(), buf.size());
}

void Mail::eraseMailDB(std::vector<uint32_t>& index)
{
    if(index.empty())
        return;
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PrivateRaw::EraseMail));
    auto msg = reinterpret_cast<PrivateRaw::EraseMail*>(buf.data());
    msg->roleId = m_owner.id();
    for(const auto& it : index)
    {
        buf.resize(buf.size() + sizeof(uint32_t));
        auto msg = reinterpret_cast<PrivateRaw::EraseMail*>(buf.data());
        msg->mailIndex[msg->size] = it;
        ++msg->size;
    }

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(EraseMail), buf.data(), buf.size());
}

void Mail::refreshMailState(std::set<uint32_t>& posList)
{
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PublicRaw::RefreshMailState));
    for(const auto& pos : posList)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshMailState::MailStateInfo));
        auto msg = reinterpret_cast<PublicRaw::RefreshMailState*>(buf.data());
        msg->data[msg->size].mailIndex = m_mailQueue[pos].mailIndex;
        msg->data[msg->size].state = m_mailQueue[pos].state;
        ++msg->size;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshMailState), buf.data(), buf.size());
}

void Mail::refreshMailState(const MailInfo& info)
{
    std::vector<uint8_t> buf;
    buf.reserve(64);
    buf.resize(sizeof(PublicRaw::RefreshMailState) + sizeof(PublicRaw::RefreshMailState::MailStateInfo));
    auto msg = reinterpret_cast<PublicRaw::RefreshMailState*>(buf.data());
    msg->size = 1;
    msg->data[0].mailIndex = info.mailIndex;
    msg->data[0].state = info.state;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshMailState), buf.data(), buf.size());
}

}

