#include "mail.h"
#include "mail_table_structure.h"
#include "role_table_structure.h"
#include "role_manager.h"
#include "water/componet/logger.h"
#include "water/componet/datetime.h"
#include "water/componet/string_kit.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace dbcached{
using namespace water;
using namespace water::componet;

Mail& Mail::me()
{
    static Mail me;
    return me;
}

void Mail::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(WriteMailByName, std::bind(&Mail::servermsg_WriteMailByName, this, _1));
    REG_RAWMSG_PRIVATE(WriteMailById, std::bind(&Mail::servermsg_WriteMailById, this, _1));
    REG_RAWMSG_PRIVATE(UpdateMail, std::bind(&Mail::servermsg_UpdateMail, this, _1));
    REG_RAWMSG_PRIVATE(EraseMail, std::bind(&Mail::servermsg_EraseMail, this, _1));
}

void Mail::writeMail(RoleId roleId, const MailInfo& mailInfo)
{
    uint32_t deleteIndex = 0;
    if(getMailCount(roleId) > MAX_MAIL_DB_NUM)
    {
        deleteIndex = eraseEarliestMail(roleId);
        LOG_TRACE("邮件, roleId={} 邮件超出接收上限{}, 服务器删除邮件mailIndex={}", roleId, MAX_MAIL_DB_NUM, deleteIndex);
    }

    MailInfo info;
    info = mailInfo;
    info.mailIndex = getMailIndex(roleId);
    if((uint32_t)-1 == info.mailIndex)
    {
        LOG_ERROR("邮件, roleId={} 无法接收, 邮件索引异常", roleId);
        return;
    }

    if(insertDB(roleId, info))
    {
        LOG_TRACE("邮件, roleId={} 就收新邮件, mailIndex={}, title={}", roleId, info.mailIndex, info.title);
        auto role = RoleManager::me().m_contrRoles.getById(roleId);
        if(nullptr == role)
            return;

        //同步world
        if(0 != deleteIndex)
        {
            //删掉缓存中数据
            RoleManager::me().m_contrRoles.eraseMail(deleteIndex, roleId);
            //通知world删除
            PrivateRaw::ServerEraseMail erase;
            erase.roleId = roleId;
            role->sendToWorld(RAWMSG_CODE_PRIVATE(ServerEraseMail), &erase, sizeof(erase));
        }

        //添加邮件缓存
        RoleManager::me().m_contrRoles.insertMail(info, roleId);
        //通知world有新的邮件
        PrivateRaw::NewMailToWorld syncMail;
        syncMail.roleId = roleId;
        syncMail.info = info;
        role->sendToWorld(RAWMSG_CODE_PRIVATE(NewMailToWorld), &syncMail, sizeof(syncMail));
    }
}

void Mail::servermsg_WriteMailByName(const uint8_t* msgData)
{
    auto rev = reinterpret_cast<const PrivateRaw::WriteMailByName*>(msgData);
    RoleId roleId = getRoleId(rev->receiver);
    if(0 == roleId)
    {
        LOG_ERROR("邮件, 写数据库时找不到角色({})", rev->receiver);
        return;
    }

    writeMail(roleId, rev->info);
}

void Mail::servermsg_WriteMailById(const uint8_t* msgData)
{
    auto rev = reinterpret_cast<const PrivateRaw::WriteMailById*>(msgData);
    writeMail(rev->roleId, rev->info);
}

void Mail::servermsg_UpdateMail(const uint8_t* msgData)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateMail*>(msgData);
    for(ArraySize i = 0; i < rev->size; ++i)
    {
        if(updateDB(rev->roleId, rev->data[i]))
            RoleManager::me().m_contrRoles.updateMail(rev->data[i], rev->roleId);
    }
}

void Mail::servermsg_EraseMail(const uint8_t* msgData)
{
    auto rev = reinterpret_cast<const PrivateRaw::EraseMail*>(msgData);
    for(ArraySize i = 0; i < rev->size; ++i)
    {
        if(eraseDB(rev->roleId, rev->mailIndex[i]))
        {
            RoleManager::me().m_contrRoles.eraseMail(rev->mailIndex[i], rev->roleId);
            LOG_TRACE("邮件, roleId={} 玩家主动请求删除邮件, mailIndex={}", rev->roleId, rev->mailIndex[i]);
        }
    }
}

std::vector<MailInfo> Mail::load(RoleId roleId, uint32_t& curMailIndex)
{
    std::vector<MailInfo> mailQueue;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select mailIndex, title, text, state, time, obj from mail where roleId = " << roleId;
        std::vector<RowOfMail> res;
        query.storein(res);

        if(res.size() > 0)
        {
            for(const auto& iter : res)
            {
                MailInfo info;
                std::memset(&info, 0, sizeof(info));
                info.mailIndex = iter.mailIndex;
                iter.title.copy(info.title, sizeof(info.title)-1);
                iter.text.copy(info.text, sizeof(info.text)-1);
                info.state = iter.state;
                info.time = iter.time;

                uint32_t index = 0;
                std::vector<std::string> vs = splitString(iter.obj, ",");
                for(const auto& subiter : vs)
                {
                    if(index >= MAX_MAIL_OBJ_NUM)
                        break;
                    std::vector<std::string> subvs = splitString(subiter, "-");
                    if(subvs.size() < 3)
                        continue;
                    info.obj[index].tplId = atoi(subvs[0].c_str());
                    info.obj[index].num = atoi(subvs[1].c_str());
                    info.obj[index].bind = static_cast<Bind>(atoi(subvs[2].c_str()));
                }

                mailQueue.push_back(info);
                if(info.mailIndex > curMailIndex)
                    curMailIndex = info.mailIndex;
            }
        }
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::load 错误, error={}", er.what());
    }

    return mailQueue;
}


bool Mail::insertDB(RoleId roleId, MailInfo& info)
{
    try
    {
        std::string objstr;
        for(uint32_t i = 0; i < MAX_MAIL_OBJ_NUM; ++i)
        {
            if(info.obj[i].tplId > 0)
            {
                objstr += toString(info.obj[i].tplId);
                objstr += "-";
                objstr += toString(info.obj[i].num);
                objstr += "-";
                objstr += toString(info.obj[i].bind);
                objstr += ",";
            }
        }

        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfMail mailrow(roleId, info.mailIndex, info.title, info.text, info.state, info.time, objstr);
        query.insert(mailrow);
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::insertDB 错误, error={}", er.what());
        return false;
    }

    return true;
}

bool Mail::updateDB(RoleId roleId, const PrivateRaw::UpdateMail::MailModify& data)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update mail set state = " << (uint16_t)data.state << " where roleId = " << roleId << " and mailIndex = " << data.mailIndex;
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::updateDB 错误, error={}", er.what());
        return false;
    }
    return true;
}

bool Mail::eraseDB(RoleId roleId, uint32_t mailIndex)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from mail where roleId = " << roleId << " and mailIndex = " << mailIndex;
        query.execute();
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::erase 错误, error={}", er.what());
        return false;
    }
    return true;
}

RoleId Mail::getRoleId(const std::string& receiver) const
{
    Role::Ptr role = RoleManager::me().m_contrRoles.getByName(receiver);
    if(nullptr != role)
        return role->id();

    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select id from roleRarelyUp where name = " << mysqlpp::quote << receiver << " limit 1";
        std::vector<RowOfRoleRarelyUp> res;
        query.storein(res);
        if(res.size() > 0)
            return res[0].id;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::getRoleId 错误, recevier={}, error={}", receiver, er.what());
    }

    return 0;
}

uint32_t Mail::getMailIndex(RoleId roleId) const
{
    Role::Ptr role = RoleManager::me().m_contrRoles.getById(roleId);
    if(nullptr != role)
        return role->curMailIndex();

    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from mail where roleId = " << roleId << " order by mailIndex desc limit 1";
        std::vector<RowOfMail> res;
        query.storein(res);

        uint32_t mailIndex = 0;
        if(res.size() > 0)
            mailIndex = res[0].mailIndex;
        return ++mailIndex;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::getMailIndex 错误, error={}", er.what());
    }
    return (uint32_t)-1;
}

uint16_t Mail::getMailCount(RoleId roleId) const
{
    uint16_t count = 0;
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select count(*) from mail where roleId = " << roleId;
        mysqlpp::StoreQueryResult res = query.store();
        count = res[0][0];
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::getMailCount 错误, error={}", er.what());
    }

    return count;
}

uint32_t Mail::eraseEarliestMail(RoleId roleId) const
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from mail where roleId = " << roleId << " order by time asc limit 1";
        std::vector<RowOfMail> res;
        query.storein(res);
        if(res.size() < 1)
            return 0;

        mysqlpp::Query queryDelete = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        queryDelete << "delete from mail where roleId = " << roleId << " and mailIndex = " << res[0].mailIndex;
        queryDelete.execute();
        return res[0].mailIndex;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("邮件, Mail::eraseEarliestMail 错误, error={}", er.what());
        return 0;
    }
}

}

