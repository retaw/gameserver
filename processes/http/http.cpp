#include "http.h"
#include "http_callback.h"
#include "http_parse.h"
#include "water/componet/logger.h"

namespace http{

Http::Http(int32_t num, const std::string& configDir, const std::string& logDir)
: Process("http", num, configDir, logDir)
{
}

void Http::extendInit()
{
    if (m_httpServer)
    {
        m_httpcons.e_packetrecv.reg(std::bind(&Http::dealPacket, this, std::placeholders::_1, std::placeholders::_2));
        HttpCallBackManager::getMe().initCallBack();
    }
}

void Http::dealPacket(HttpConnectionManager::ConnectionHolder::Ptr connHolder, net::Packet::CPtr packet)
{
    HttpParse parse;
    parse.parseHttp(connHolder, packet);
}

}

