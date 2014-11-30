#include "connection_checker.h"

#include "componet/log.h"

namespace water{

ConnectionChecker::ConnectionChecker(ProcessIdentity processId)
: m_processId(processId)
{
}

void ConnectionChecker::setPrivateWhiteList(const std::set<ProcessIdentity>& acceptWhiteList)
{
    m_acceptWhiteList = acceptWhiteList;
}

void ConnectionChecker::addUncheckedPrivateConnection(net::PacketConnection::Ptr conn, ConnType type)
{
    try
    {
        conn->setNonBlocking();

        ConnInfo connInfo;
        if(type == ConnType::in)
        {
            LOG_TRACE("new private conn in, remoteEp={}", conn->getRemoteEndpoint());
            connInfo.state = ConnState::recvId;
            conn->setRecvBuffer(sizeof(ProcessIdentity));
        }
        else
        {
            connInfo.state = ConnState::sendId;
            conn->setSendBuffer(&m_processId, sizeof(m_processId));
        }
        connInfo.conn = conn;

        m_privateConns.push_back(connInfo);
    }
    catch(const componet::ExceptionBase& ex)
    {
        const char* typeStr = (type == ConnType::in) ? "in" : "out";
        LOG_ERROR("new private conn {}, error, remoteEp={}", type, conn->getRemoteEndpoint());
    }
}

void ConnectionChecker::addUncheckedPublicConnection(net::PacketConnection::Ptr, ConnType type)
{
}

bool ConnectionChecker::ConnectionChecker::exec()
{
    while(checkSwitch())
    {
        checkPublicConn();
        checkPrivateConn();
    }
    return true;
}

void ConnectionChecker::checkPrivateConn()
{
    while(checkSwitch())
    {
        LockGuard lock(m_mutex);
        for(auto it = m_privateConns.begin(); it != m_privateConns.end(); )
        {
            try
            {
                switch (it->state)
                {
                case ConnState::recvId:
                    {
                        if( !it->conn->tryRecvPacket() )
                            continue;

                        //接收到一份完整的packet
                        auto packet = it->conn->getRecvBuffer();
                        //记下接入者id
                        it->remoteId = *reinterpret_cast<const ProcessIdentity*>(packet->getContent());
                        //检查接入者id
                        if(m_acceptWhiteList.empty() || 
                           m_acceptWhiteList.find(it->remoteId) != m_acceptWhiteList.end())
                        {
                            //检查通过，回复本进程的id
                            LOG_TRACE("private in verify failed，remtoeId={}", it->remoteId);
                            it->state = ConnState::sendRet;
                            it->conn->setSendBuffer(&m_processId, sizeof(m_processId));
                        }
                        else
                        {
                            //检查未通过，将对方的id置为无效值，并回复无效id表示拒绝
                            LOG_TRACE("private in verify ok，remtoeId={}", it->remoteId);
                            it->remoteId.value = INVALID_PROCESS_IDENDITY_VALUE;
                            it->conn->setSendBuffer(&it->remoteId, sizeof(it->remoteId));
                        }
                    }
                    //break; 这里故意不要break
                case ConnState::sendRet:
                    {
                        if( !it->conn->trySendPacket() )
                            continue;

                        //发完验证结果，处理验证通过的连接
                        if(it->remoteId.value != INVALID_PROCESS_IDENDITY_VALUE)
                            e_connConfirmed(it->conn, it->remoteId);

                        //done
                        it = m_privateConns.erase(it);
                        continue;
                    }
                    break;
                case ConnState::sendId:
                    {
                        //发送自己的id给监听进程
                        if( !it->conn->trySendPacket() )
                            continue;

                        //发完自己的id，进入等待对方回复的状体
                        it->state = ConnState::recvRet;
                        it->conn->setRecvBuffer(sizeof(ProcessIdentity));
                    }
                    //break; 这里故意不要break
                case ConnState::recvRet:
                    {
                        if( !it->conn->tryRecvPacket() )
                            continue;

                        //收到对方的完整回复
                        auto packet = it->conn->getRecvBuffer();
                        //记下对方回复的id
                        it->remoteId = *reinterpret_cast<const ProcessIdentity*>(packet->getContent());
                        //检查回复内容
                        if(it->remoteId.value != INVALID_PROCESS_IDENDITY_VALUE)
                        {
                            LOG_TRACE("private out ok, remtoeId={}", it->remoteId);
                            e_connConfirmed(it->conn, it->remoteId);
                        }
                        else
                        {
                            LOG_TRACE("private out denied remtoeEndpoint={}", it->conn->getRemoteEndpoint());
                        }

                        //done
                        it = m_privateConns.erase(it);
                        continue;
                    }
                    break;
                default:
                    break;
                }
                ++it;
            }
            catch (const net::NetException& ex)
            {
                const char* typeStr = (it->state == ConnState::recvId ||
                                       it->state == ConnState::sendRet) ? "in" : "out";
                LOG_ERROR("check {} connection error, remoteEp={}", typeStr, it->conn->getRemoteEndpoint());
                it = m_privateConns.erase(it);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void ConnectionChecker::checkPublicConn()
{
}

}
