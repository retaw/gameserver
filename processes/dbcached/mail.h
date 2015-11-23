#ifndef PROCESS_DBCACHE_MAIL_H
#define PROCESS_DBCACHE_MAIL_H

#include "water/common/maildef.h"
#include "water/common/roledef.h"
#include "protocol/rawmsg/private/mail.h"
#include "protocol/rawmsg/private/mail.codedef.private.h"
#include <vector>

namespace dbcached{

class Mail
{
public:
    Mail() = default;
    ~Mail() = default;

    static Mail& me();

public:
    void regMsgHandler();
    std::vector<MailInfo> load(RoleId roleId, uint32_t& curMailIndex);

private:
    void writeMail(RoleId roleId, const MailInfo& mailInfo);

    void servermsg_WriteMailByName(const uint8_t* msgData);
    void servermsg_WriteMailById(const uint8_t* msgData);
    void servermsg_UpdateMail(const uint8_t* msgData);
    void servermsg_EraseMail(const uint8_t* msgData);
    
    bool insertDB(RoleId roleId, MailInfo& info);
    bool updateDB(RoleId roleId, const PrivateRaw::UpdateMail::MailModify& data);
    bool eraseDB(RoleId roleId, uint32_t mailIndex);

    RoleId getRoleId(const std::string& receiver) const;
    uint32_t getMailIndex(RoleId roleId) const;
    uint16_t getMailCount(RoleId roleId) const;
    uint32_t eraseEarliestMail(RoleId roleId) const;
};

}

#endif

