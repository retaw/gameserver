#include "connection_checker.h"

namespace water{

ConnectionChecker::ConnectionChecker(const std::set<std::pair<ProcessType, int32_t>>& acceptWhiteList)
: m_acceptWhiteList(acceptWhiteList)
{
}


void addUncheckedPrivateConnection(componet::TcpConnection::Ptr conn, ConnType type)
{
    try
    {
        conn->setNonBlocking();

        ConnInfo connInfo;
        connInfo.state = (type == ConnType::in) ? ConnState::newIn : ConnState::newOut;
        connInfo.conn = conn;
        m_privateConns.push_back(connInfo);
    }
    catch(const componet::ExceptionBase& ex)
    {
        LOG_ERROR("检测private conn时出现异常, type={} remote={}", type, conn->getRemoteEndpoint().toString());
    }
}

void addUncheckedPublicConnection(componet::TcpConnection::Ptr ConnType type)
{
}

bool ConnectionChecker::exec()
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
    LockGuard lock(m_mutex);
    for(auto it = m_privateConns.begin(); it != m_privateConns.end(); ++it)
    {
        switch (it->state)
        {
        case ConnState::newIn:
            conn->
            break;
        case ConnState::newOut:
            break;
        case ConnState::sendId:
            break;
        case ConnState::ok:
            break;
        case ConnState::close:
            break;
        default:
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void ConnectionChecker::checkPublicConn()
{
    return true;
}
