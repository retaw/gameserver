#ifndef PROCESS_WORLD_MAIL_H
#define PROCESS_WORLD_MAIL_H

#include "water/common/maildef.h"
#include "water/common/roledef.h"
#include "water/componet/datetime.h"
#include <vector>
#include <set>
#include <deque>

namespace world{

using namespace water;
using namespace water::componet;

class Role;
class Mail
{
public:
    Mail(Role& me);
    ~Mail() = default;

public:
    void load(std::vector<MailInfo>& mailQueue);
    //通知玩家有新的为读取邮件
    void notifyNewMail();
    //db同步新的邮件到world role
    void addNewMail(const MailInfo& info);
    //请求邮件列表
    void retMailList();
    //查看邮件
    void readMail(uint32_t index);
    //提取附件
    void requestGetMailObj(const uint8_t* msgData, uint32_t msgSize);
    //玩家请求删除邮件
    void requestDeleteMail(const uint8_t* msgData, uint32_t msgSize);
    //db请求删除(超过邮件存储上限)
    void serverEraseMail();

    //定时检测邮件过期
    void checkMailOverdue(const TimePoint& now);

private:
    void refreshMailState(std::set<uint32_t>& posList);
    void refreshMailState(const MailInfo& info);

    void updateMailDB(std::set<uint32_t>& posList);
    void updateMailDB(const MailInfo& info);

    void eraseMailDB(std::set<uint32_t>& posList);
    void eraseMailDB(uint32_t mailIndex);
    void eraseMailDB(std::vector<uint32_t>& index);

private:
    Role& m_owner;
    std::deque<MailInfo> m_mailQueue;
};

}

#endif

