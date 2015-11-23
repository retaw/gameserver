#ifndef PROCESS_WORLD_MAIL_MGR_H
#define PROCESS_WORLD_MAIL_MGR_H

#include "water/common/maildef.h"
#include "water/common/roledef.h"

namespace world{

class MailManager
{
public:
    MailManager() = default;
    ~MailManager() = default;

    static MailManager& me();

public:
    void regMsgHandler();

    void servermsg_NewMailToWorld(const uint8_t* msgData);
    void servermsg_ServerEraseMail(const uint8_t* msgData);

    void clientmsg_RequestMailList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestMailDetailInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestDeleteMail(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestGetMailObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //邮件发送接口
    //通过名字发送
    void send(const std::string& receiver, const std::string& title, const std::string& text, const std::vector<ObjItem>& mailObjs);
    void send(const std::string& receiver, const std::string& title, const std::string& text, const ObjItem& mailObj);
    //通过roleId发送
    void send(RoleId roleId, const std::string& title, const std::string& text, const std::vector<ObjItem>& mailObjs);
    void send(RoleId roleId, const std::string& title, const std::string& text, const ObjItem& mailObj);
};

}

#endif
