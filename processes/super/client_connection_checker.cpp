#include "client_connection_checker.h"

#include "water/componet/logger.h"

#include "super.h"
//#include "protocol/rawmsg/public/login.codedef.public.h"
//#include "protocol/rawmsg/public/login.h"

#include "zone_manager.h"

//#include "protocol/rawmsg/private/login.codedef.private.h"
//#include "protocol/rawmsg/private/login.h"
//#include "protocol/rawmsg/rawmsg_manager.h"




//token验证消息的结构
#pragma pack(1)
struct AccountAndPassword
{
    water::process::Platform platform;
    char account[ACCOUNT_BUFF_SZIE];
    char password[0];
};
#pragma pack()



namespace super{

using namespace water;
using namespace process;

ClientConnectionChecker::ClientConnectionChecker()
: m_lastLoginIdCounter(0)//ProcessIdentity::MAX_VALUE)
{
}

LoginId ClientConnectionChecker::getLoginId()
{
    return (++m_lastLoginIdCounter) << 32;
}

void ClientConnectionChecker::addUncheckedConnection(net::PacketConnection::Ptr conn)
{
    if(conn == nullptr)
        return;

    try
    {
        conn->setNonBlocking();
        conn->setRecvPacket(process::TcpPacket::create());

        ClientInfo client;
        client.state   = ClientState::recvingToken;
        client.conn    = conn;

        LockGuard lock(m_clientsLock);
        m_clients.push_back(client);
    }
    catch (const net::NetException& ex)
    {
        LOG_ERROR("客户端连接验证, conn加入检查失败, {}, {}", conn->getRemoteEndpoint(), ex);
    }
}

void ClientConnectionChecker::timerExec(const componet::TimePoint& now)
{
    LockGuard lock(m_clientsLock);


    for(auto it = m_clients.begin(); it != m_clients.end(); )
    {
        try
        {
            {//一切连接直接视为合法
                LOG_TRACE("客户端连接验证, 通过, ep={}", it->conn->getRemoteEndpoint());
                const LoginId loginId = getLoginId();
                e_clientConfirmed(it->conn, loginId); //本行执行后, 才能通过Super::sendToClient发消息
//                ClientManager::me().clientConnReady(loginId);
                it = m_clients.erase(it);
                continue; //下面的代码永不执行
            }

            if(!it->conn->tryRecv())
                continue;

            auto packet = it->conn->getRecvPacket();
            TcpPacket::Ptr tcpPacket = std::static_pointer_cast<TcpPacket>(packet);
            if(tcpPacket->contentSize() < sizeof(AccountAndPassword))
            {
                LOG_TRACE("客户端连接验证, 失败, 收到错误长度的消息, 外挂, revSize={}, ep={}", tcpPacket->contentSize(), it->conn->getRemoteEndpoint());
                it = m_clients.erase(it);
                continue;
            }

            auto recvMsg = reinterpret_cast<const AccountAndPassword*>(tcpPacket->content());
            if(Super::me().platform() != recvMsg->platform)
            {
                LOG_TRACE("客户端连接验证, 失败, 非法平台{}, ep={}", recvMsg->platform, it->conn->getRemoteEndpoint());
                it = m_clients.erase(it);
                continue;
            }

            //依据实际平台来处理各自的登录验证
            switch(recvMsg->platform)
            {
            case Platform::clw:
                {//内网测试平台, 不验证, 直接通过
  /*                  LOG_TRACE("客户端连接验证, 通过, platform={}, ep={}, account={}, token={}", recvMsg->platform, it->conn->getRemoteEndpoint(), recvMsg->account, recvMsg->token);
                    const LoginId loginId = getLoginId();
                    //以下3行, 顺序不能变
                    LoginProcessor::me().newClient(loginId, recvMsg->account);
                    e_clientConfirmed(it->conn, loginId);
                    LoginProcessor::me().clientConnReady(loginId);
                    it = m_clients.erase(it);*/
                    continue;
                }
                break;
            default:
                LOG_TRACE("客户端连接验证, 失败, 未知平台{}, ep={}", recvMsg->platform, it->conn->getRemoteEndpoint());
                break;
            }
        }
        catch (const net::ReadClosedConnection& ex) 
        {
            LOG_TRACE("客户端连接验证, 失败, 对方提前断开了连接, ep={}", it->conn->getRemoteEndpoint());
            it = m_clients.erase(it);
            continue;
        } 
        catch (const net::NetException& ex)
        {
            LOG_ERROR("客户端连接验证, 出错, ep={}, {}", it->conn->getRemoteEndpoint(), ex);
            it = m_clients.erase(it);
            continue;

        }
        ++it;
    }
}

}
