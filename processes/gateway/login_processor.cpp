#include "login_processor.h"

#include "water/componet/logger.h"

#include "gateway.h"
#include "role_manager.h"
#include "protocol/rawmsg/public/login.codedef.public.h"
#include "protocol/rawmsg/public/login.h"

#include "protocol/rawmsg/private/login.codedef.private.h"
#include "protocol/rawmsg/private/login.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace gateway{

using namespace water;
using namespace process;

LoginProcessor& LoginProcessor::me()
{
    static LoginProcessor me;
    return me;
}

LoginProcessor::LoginProcessor()
: m_lastLoginId(0)
{
}

LoginId LoginProcessor::getLoginId()
{
    LoginId ret = ++m_lastLoginId;
    return (ret << 16u) + Gateway::me().getId().value();
}

void LoginProcessor::newClient(LoginId loginId, const std::string& account)
{
    auto client = ClientInfo::Ptr(new ClientInfo);
    client->loginId = loginId;
    client->account = account;

    LockGuard lock(m_clientsLock);
    m_clients.insert({loginId, client});
}

void LoginProcessor::clientConnReady(LoginId loginId)
{
    ClientInfo::Ptr client = getClientByLoginId(loginId);
    if(client == nullptr)
        return;

    PrivateRaw::QuestRoleList send;
    send.loginId = client->loginId;
    client->account.copy(send.account, sizeof(send.account));
    send.account[sizeof(send.account) - 1] = 0;

    ProcessIdentity receiver("dbcached", 1); 
    Gateway::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(QuestRoleList), &send, sizeof(send));
    LOG_DEBUG("登录, 向db请求角色列表, account={}", client->account);
}

void LoginProcessor::delClient(LoginId loginId)
{
    LockGuard lock(m_disconnectedClientsLock);
    m_disconnectedClients.push_back(loginId);
    LOG_DEBUG("登录, 下线, login={}", loginId);
}

void LoginProcessor::timerExec(const componet::TimePoint& now)
{
    //删掉已经断开的clients
    m_disconnectedClientsLock.lock();
    std::list<LoginId> temp = m_disconnectedClients;
    m_disconnectedClients.clear();
    m_disconnectedClientsLock.unlock();

    LockGuard lock(m_clientsLock);
    for(auto loginId : temp)
    {
        auto it = m_clients.find(loginId);
        if(it == m_clients.end()) //已经不在了, 正常的话, 这个角色已经登陆完成, 在rolemanager中了
        {
            RoleManager::me().onClientDisconnect(loginId);
            continue;
        }
        m_clients.erase(loginId);
    }
}

LoginProcessor::ClientInfo::Ptr LoginProcessor::getClientByLoginId(LoginId loginId)
{
    LockGuard lock(m_clientsLock);
    auto it = m_clients.find(loginId);
    if(it == m_clients.end())
        return nullptr;
    return it->second;
}

void LoginProcessor::clientmsg_CreateRole(const uint8_t* msgData, uint32_t msgSize, LoginId loginId)
{
    LOG_DEBUG("登录, 收到client 建角请求");
    auto rev = reinterpret_cast<const PublicRaw::CreateRole*>(msgData);
    if(sizeof(*rev) > msgSize)
        return;

    ClientInfo::Ptr client = getClientByLoginId(loginId);
    if(client == nullptr)
        return;

    const std::string name = componet::format("{}.{}", Gateway::me().zoneId(), rev->basicInfo.name);
    if(name.size() > MAX_NAME_SZIE)
    //if(!NameChecker::me().check(name))  //未来要换成一个名字检查器
    {
        PublicRaw::LoginRet clientMsg;
        clientMsg.ret = LoginRetCode::invalidRoleName;
        Gateway::me().sendToClient(client->loginId, RAWMSG_CODE_PUBLIC(LoginRet), &clientMsg, sizeof(clientMsg));
        return;
    }

    PrivateRaw::CreateRole send;
    send.loginId = client->loginId;
    client->account.copy(send.account, sizeof(send.account));
    std::memcpy(&send.basicInfo, &rev->basicInfo, sizeof(send.basicInfo));
    //名字
    std::memset(&send.basicInfo.name, 0, sizeof(send.basicInfo.name));
    name.copy(send.basicInfo.name, sizeof(send.basicInfo.name));

    ProcessIdentity receiver("dbcached", 1);
    Gateway::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(CreateRole), &send, sizeof(send));
    LOG_DEBUG("登录, 转发建角请求去db, name={}, basicName, account={}", name, rev->basicInfo.name, client->account);

}
void LoginProcessor::clientmsg_GetRandName(const uint8_t* msgData, uint32_t msgSize, LoginId loginId)
{
    LOG_DEBUG("登录, 收到客户端随机名请求");
    auto rev = reinterpret_cast<const PublicRaw::GetRandName*>(msgData);
    if(sizeof(*rev) > msgSize)
        return;
    ClientInfo::Ptr client = getClientByLoginId(loginId);
    if(client == nullptr)
        return;
    PrivateRaw::GetRandName send;
    send.loginId = client->loginId;
    send.sex = rev->sex;
    ProcessIdentity receiver("dbcached", 1);
    Gateway::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(GetRandName), &send, sizeof(send));
    LOG_DEBUG("登录, 转发请求随机名去db, loginId={}",loginId);

}

void LoginProcessor::clientmsg_SelectRole(const uint8_t* msgData, uint32_t msgSize, LoginId loginId)
{
    LOG_DEBUG("登录, recv clientmsg 选择角色");
    auto rev = reinterpret_cast<const PublicRaw::SelectRole*>(msgData);
    if(sizeof(*rev) > msgSize)
        return;

    ClientInfo::Ptr client = getClientByLoginId(loginId);
    if(client == nullptr)
        return;

    auto it = client->roleList.find(rev->rid);
    if(it == client->roleList.end())
    {
        LOG_DEBUG("登录, recv 选择角色, 所选的rid不存在, account={}", client->account);
        return;
    }

    Role::Ptr role = RoleManager::me().newLoginRole(loginId, rev->rid, it->second, client->account);

    PublicRaw::LoginRet clientMsg;
    if(role == nullptr)
    {
        LOG_DEBUG("登录, 选角后, gateRoleManager创建 Role失败, roleInfo=[ {}, {}, {}, {} ]", loginId, rev->rid, it->second, client->account);
        return;
    }

    //通知端登陆成功
    LOG_DEBUG("登录,  选择角色成功, 通知端加载资源");
    clientMsg.ret = LoginRetCode::successful;
    Gateway::me().sendToClient(client->loginId, RAWMSG_CODE_PUBLIC(LoginRet), &clientMsg, sizeof(clientMsg));


    //至此, 登陆流结束, 以后的client msg, 都是正常在线role的业务逻辑, 都交由roleManager来处理
    LockGuard lock(m_clientsLock);
    m_clients.erase(client->loginId);
    return;
}


void LoginProcessor::servermsg_RetRoleList(const uint8_t* msgData, uint32_t msgSize)
{
    LOG_DEBUG("登录, 收到db的角色列表");
    auto rev = reinterpret_cast<const PrivateRaw::RetRoleList*>(msgData);

    ClientInfo::Ptr client = getClientByLoginId(rev->loginId);
    if(client == nullptr)
        return;

    PublicRaw::LoginRetRoleList send;
    send.listSize = rev->listSize;
    std::memcpy(send.roleList, rev->roleList, sizeof(send.roleList));
    Gateway::me().sendToClient(client->loginId, RAWMSG_CODE_PUBLIC(LoginRetRoleList), &send, sizeof(send));

    LOG_DEBUG("登录, 转发送角色列表给client, listSize={}", send.listSize);

    //记下角色列表
    for(uint32_t i = 0; i < rev->listSize; ++i)
        client->roleList[rev->roleList[i].id] = rev->roleList[i].name;
}

void LoginProcessor::servermsg_RetCreateRole(const uint8_t* msgData, uint32_t msgSize)
{
    LOG_DEBUG("登录, 收到db的建角结果");
    auto rev =  reinterpret_cast<const PrivateRaw::RetCreateRole*>(msgData);
    ClientInfo::Ptr client = getClientByLoginId(rev->loginId);
    if(client == nullptr)
        return;


    if(rev->code != LoginRetCode::successful)
    {
        PublicRaw::LoginRet send;
        send.ret = rev->code;
        Gateway::me().sendToClient(rev->loginId, RAWMSG_CODE_PUBLIC(LoginRet), &send, sizeof(send));
        LOG_DEBUG("登录, db建角色失败, 通知端");
        return;
    }
        
    PublicRaw::RetCreateRole send;
    send.listSize = rev->listSize;
    std::memcpy(send.roleList, rev->roleList, sizeof(send.roleList));
    Gateway::me().sendToClient(rev->loginId, RAWMSG_CODE_PUBLIC(RetCreateRole), &send, sizeof(send));
    LOG_DEBUG("登录, db建角色成功, 发送角色列表到端");

    //记下角色列表
    for(uint32_t i = 0; i < rev->listSize; ++i)
        client->roleList[rev->roleList[i].id] = rev->roleList[i].name;
}

void LoginProcessor::servermsg_RetRandName(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev =  reinterpret_cast<const PrivateRaw::RetRandName*>(msgData);
    LOG_DEBUG("登录, 收到db的随机名,name={}",rev->name);
    ClientInfo::Ptr client = getClientByLoginId(rev->loginId);
    if(client == nullptr)
        return;
    PublicRaw::RetRandName send;
    std::memcpy(send.name, rev->name, sizeof(send.name));
    Gateway::me().sendToClient(rev->loginId, RAWMSG_CODE_PUBLIC(RetRandName), &send, sizeof(send));
    LOG_DEBUG("登录, db请求随机名成功, 发送随机名到端");
}

void LoginProcessor::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(SelectRole, std::bind(&LoginProcessor::clientmsg_SelectRole, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(CreateRole, std::bind(&LoginProcessor::clientmsg_CreateRole, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(GetRandName, std::bind(&LoginProcessor::clientmsg_GetRandName, this, _1, _2, _3));

    REG_RAWMSG_PRIVATE(RetRoleList, std::bind(&LoginProcessor::servermsg_RetRoleList, this, _1, _2));
    REG_RAWMSG_PRIVATE(RetCreateRole, std::bind(&LoginProcessor::servermsg_RetCreateRole, this, _1, _2));//, _3, _4));
    REG_RAWMSG_PRIVATE(RetRandName, std::bind(&LoginProcessor::servermsg_RetRandName, this, _1, _2));
}

}
