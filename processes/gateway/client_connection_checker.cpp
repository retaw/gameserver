#include "client_connection_checker.h"

#include "water/componet/logger.h"

#include "gateway.h"
#include "role.h"
//#include "protocol/rawmsg/public/login.codedef.public.h"
//#include "protocol/rawmsg/public/login.h"

#include "login_processor.h"

#include "protocol/rawmsg/private/login.codedef.private.h"
#include "protocol/rawmsg/private/login.h"
//#include "protocol/rawmsg/rawmsg_manager.h"


//token验证消息的结构
#pragma pack(1)
struct TokenAndAccountMsg
{
    water::process::Platform platform;
    char account[ACCOUNT_BUFF_SZIE];
    uint8_t token[0];
};
#pragma pack()



namespace gateway{

using namespace water;
using namespace process;

ClientConnectionChecker::ClientConnectionChecker()
: m_lastLoginIdCounter(0)
{
}

LoginId ClientConnectionChecker::getLoginId()
{
    LoginId ret = ++m_lastLoginIdCounter;
    return (ret << 32u) + Gateway::me().getId().value();
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
    //超时处理,暂空
    LockGuard lock(m_clientsLock);
    for(auto it = m_clients.begin(); it != m_clients.end(); )
    {
        try
        {
            if(!it->conn->tryRecv())
			{
				++it;
				continue;
			}

            auto packet = it->conn->getRecvPacket();
            TcpPacket::Ptr tcpPacket = std::static_pointer_cast<TcpPacket>(packet);
            if(tcpPacket->contentSize() < sizeof(TokenAndAccountMsg))
            {
                LOG_TRACE("客户端连接验证, 失败, 收到错误长度的消息, 外挂, revSize={}, ep={}", tcpPacket->contentSize(), it->conn->getRemoteEndpoint());
                it = m_clients.erase(it);
                continue;
            }

            auto recvMsg = reinterpret_cast<const TokenAndAccountMsg*>(tcpPacket->content());
            if(Gateway::me().platform() != Platform::dev && Gateway::me().platform() != recvMsg->platform)  //测试平台不验证平台类型
            {
                LOG_TRACE("客户端连接验证, 失败, 非法平台{}, ep={}", recvMsg->platform, it->conn->getRemoteEndpoint());
                it = m_clients.erase(it);
                continue;
            }

            //依据实际平台来处理各自的登录验证
            switch(recvMsg->platform)
            {
            case Platform::dev:
            default:
                {//内网测试平台, 不验证, 直接通过
                    LOG_TRACE("客户端连接验证, 通过, platform={}, ep={}, account={}, token={}", recvMsg->platform, it->conn->getRemoteEndpoint(), recvMsg->account, recvMsg->token);
                    const LoginId loginId = getLoginId();
                    //以下3行, 顺序不能变
                    LoginProcessor::me().newClient(loginId, recvMsg->account);
                    e_clientConfirmed(it->conn, loginId);
                    LoginProcessor::me().clientConnReady(loginId);
                    it = m_clients.erase(it);
                    continue;
                }
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
