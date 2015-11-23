/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:40 +0800
 *
 * Description: 
 */

#ifndef ROUTER_ROUTER_H
#define ROUTER_ROUTER_H

#include "water/process/process.h"

namespace router{

using namespace water;
using namespace process;

class Router : public Process
{
private:
    Router(int32_t num, const std::string& configDir, const std::string& logDir);

    void tcpPacketHandle(TcpPacket::Ptr packet,
                         TcpConnectionManager::ConnectionHolder::Ptr conn,
                         const componet::TimePoint& now) override;
    void init() override;

public:
    static void init(int32_t num, const std::string& configDir, const std::string& logDir);
    static Router& me();
private:
    static Router* m_me;
};

}

#endif

