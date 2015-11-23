#include "private_connection_checker.h"

#include "componet/logger.h"

namespace water{
namespace process{

PrivateConnectionChecker::PrivateConnectionChecker(ProcessIdentity processId)
: m_processId(processId)
{
}

void PrivateConnectionChecker::addUncheckedConnection(net::PacketConnection::Ptr conn, ConnType type)
{
    try
    {
        conn->setNonBlocking();

        ConnInfo connInfo;
        if(type == ConnType::in)
        {
            connInfo.state = ConnState::recvId;
            conn->setRecvPacket(net::Packet::create(sizeof(ProcessIdentity)));
        }
        else
        {
            connInfo.state = ConnState::sendId;
            conn->setSendPacket(net::Packet::create(&m_processId, sizeof(m_processId)));
        }
        connInfo.conn = conn;

        LockGuard lock(m_mutex);
        m_conns.push_back(connInfo);
    }
    catch(const componet::ExceptionBase& ex)
    {
        const char* typeStr = (type == ConnType::in) ? "in" : "out";
        LOG_ERROR("检查新建私网{}连接出错, remoteEp={}, {}", 
                  typeStr, conn->getRemoteEndpoint(), ex);
    }
}

void PrivateConnectionChecker::checkConn()
{
    while(checkSwitch())
    {
        m_mutex.lock();
        for(auto it = m_conns.begin(); it != m_conns.end(); )
        {
            try
            {
                switch (it->state)
                {
                case ConnState::recvId:
                    {
                        if( !it->conn->tryRecv() )
                            break;

                        //记下接入者id
                        auto packet = it->conn->getRecvPacket();
                        it->remoteId = *reinterpret_cast<const ProcessIdentity*>(packet->data());
                        //检查接入者id是否为有效Id
                        if(it->remoteId.isValid())
                        {
                            //检查通过，回复本进程的id
                            it->conn->setSendPacket(net::Packet::create(&m_processId, sizeof(m_processId)));
                        }
                        else
                        {
                            //检查未通过，将对方的id置为无效值，并回复无效id表示拒绝
                            it->remoteId.clear();
                            it->conn->setSendPacket(net::Packet::create(&it->remoteId, sizeof(it->remoteId)));
                        }
                        it->state = ConnState::sendRet;
                    }
                    //这里故意不要break
                case ConnState::sendRet:
         
                    {
                        if( !it->conn->trySend() )
                            break;

                        //发完验证结果，处理验证通过的连接
                        if(it->remoteId.isValid())
                        {
                            e_connConfirmed(it->conn, it->remoteId);
                            LOG_TRACE("来自 {} 的私网连接验证通过", it->remoteId);
                        }
                        else
                        {
                            LOG_TRACE("来自 {} 的私网连接验证失败", it->remoteId);
                        }

                        it = m_conns.erase(it);
                        continue;
                    }
                    break;
                case ConnState::sendId:
                    {
                        //发送自己的id给监听进程
                        if( !it->conn->trySend() )
                            break;

                        //发完自己的id，进入等待对方回复的状体
                        it->conn->setRecvPacket(net::Packet::create(sizeof(ProcessIdentity)));
                        it->state = ConnState::recvRet;
                    }
                    //这里故意不要break
                case ConnState::recvRet:
                    {
                        if( !it->conn->tryRecv() )
                            break;

                        //记下对方回复的id
                        auto packet = it->conn->getRecvPacket();
                        it->remoteId = *reinterpret_cast<const ProcessIdentity*>(packet->data());
                        //检查回复内容
                        if(it->remoteId.isValid())
                        {
                            e_connConfirmed(it->conn, it->remoteId);
                            LOG_TRACE("到 {} 的私网连接已被接受", 
                                      it->remoteId, it->conn->getRemoteEndpoint());
                        }
                        else
                        {
                            LOG_TRACE("到 {} 的私网连接被拒绝{}", it->conn->getRemoteEndpoint());
                        }

                        //done
                        it = m_conns.erase(it);
                        continue;
                    }
                    break;
                default:
                    break;
                }
            }
            catch (const net::NetException& ex)
            {
                const char* typeStr = (it->state == ConnState::recvId ||
                                       it->state == ConnState::sendRet) ? "in" : "out";
                LOG_ERROR("检查 {} 连接时出错 remoteEp={}, {}", 
                          typeStr, it->conn->getRemoteEndpoint(), ex.what());
                it = m_conns.erase(it);
                continue;
            }
            ++it;
        }
        m_mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

}}
