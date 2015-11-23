/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 19:06 +0800
 *
 * Description: 
 */

#include "endpoint.h"

#include "../componet/scope_guard.h"
#include "../componet/string_kit.h"

#include <string>
#include <cstring>
#include <iostream>
//#include <regex>

#include <sys/socket.h>      //socket
#include <sys/ioctl.h>       //ioctl
#include <unistd.h>          //close
#include <linux/sockios.h>   //SIOCGIFADDR
#include <net/if.h>          //struct ifreq
#include <netinet/in.h>     //struct sockaddr_in

namespace water{
namespace net{

IpV4::IpV4(const std::string& ipStr)
{
    fromString(ipStr);
}

IpV4::IpV4(uint32_t ipValue)
:value(ipValue)
{
}

std::string IpV4::toString() const
{
    std::stringstream ss;
    ss
    << (int)bytes[0] << "." 
    << (int)bytes[1] << "." 
    << (int)bytes[2] << "." 
    << (int)bytes[3];

    return ss.str();
}

void IpV4::appendToString(std::string* str) const
{
    componet::formatAndAppend(str, "{}.{}.{}.{}",
                              (int)bytes[0],
                              (int)bytes[1],
                              (int)bytes[2],
                              (int)bytes[3]);
}

void IpV4::fromString(const std::string& str)
{
    std::vector<std::string> items = componet::splitString(str, ".");
    items.resize(4);

    bytes[0] = componet::fromString<int>(items[0]);
    bytes[1] = componet::fromString<int>(items[1]);
    bytes[2] = componet::fromString<int>(items[2]);
    bytes[3] = componet::fromString<int>(items[3]);
}

IpV4 IpV4::getAddrByIfrName(const std::string& ifrName)
{
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    ON_EXIT_SCOPE_DO( ::close(fd) );

    if(fd == -1)
        SYS_EXCEPTION(NetException, "::socket");

    struct ifreq ifr;
    std::memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    ifrName.copy(ifr.ifr_name, sizeof(ifr.ifr_name));

    if(::ioctl(fd, SIOCGIFADDR, &ifr) == -1)
        SYS_EXCEPTION(NetException, "::ioctl");

    IpV4 ip;
    ip.value = ((const struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr.s_addr;
    return ip;
}

std::ostream& operator << (std::ostream& os, const IpV4& ip)
{
    os << ip.toString();
    return os;
}

std::istream& operator >> (std::istream& is, IpV4& ip)
{
    std::string ipStr;
    is >> ipStr;
    ip.fromString(ipStr);
    return is;
}

Endpoint::Endpoint(const std::string& str)
{
    fromString(str);
}

std::string Endpoint::toString() const
{
    return ip.toString() + ":" + componet::toString(port);
}

void Endpoint::fromString(const std::string& str)
{
    //std::vector<std::string> strs = componet::splitString(str, ":");
    std::vector<std::string> strs;
    componet::splitString(&strs, str, ":");
    if(strs.size() < 2)
        EXCEPTION(InvalidEndpointStringFormat, "std:\"{}\"]", str);

//#if GCC_VERSION > 40902
//    std::regex ipRegex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
//    if(!std::regex_match(strs[0], ipRegex))
//        EXCEPTION(InvalidEndpointStringFormat, "std:\"{}\"]", str);
//#endif

    ip = componet::fromString<IpV4>(strs[0]);
    port = componet::fromString<uint16_t>(strs[1]);
}

void Endpoint::appendToString(std::string* str) const
{
    componet::formatAndAppend(str, "{}:{}", ip, port);
}

bool operator==(const Endpoint& ep1, const Endpoint& ep2)
{
    return ep1.ip.value == ep2.ip.value && ep1.port == ep2.port;
}

bool operator<(const Endpoint& ep1, const Endpoint& ep2)
{
    if(ep1.ip.value == ep2.ip.value)
        return ep1.port < ep2.port;

    return ep1.ip.value < ep2.ip.value;
}


}} //namespace water::net


