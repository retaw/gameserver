/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:05 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_ENDPOINT_HPP
#define WATER_NET_ENDPOINT_HPP

#include "net_exception.h"

#include <cstdint>
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>

namespace water{
namespace net{

struct IpV4
{
    union
    {
        uint32_t value;
        uint8_t bytes[4];
    };

    IpV4(const std::string& ipStr = "0,0,0,0");
    explicit IpV4(uint32_t ipValue);

    std::string toString() const;
    void appendToString(std::string* str) const;
    void fromString(const std::string& str);
    static IpV4 getAddrByIfrName(const std::string& ifrName);
};

std::ostream& operator << (std::ostream& os, const IpV4& ip);
std::istream& operator >> (std::istream& is, IpV4& ip);

DEFINE_EXCEPTION(InvalidEndpointStringFormat, NetException);
struct Endpoint
{
    Endpoint() = default;
    explicit Endpoint(const std::string& str);

    IpV4 ip;
    uint16_t port = 0;

    //字符串转换 格式：xxx.xxx.xxx.xxx:port
    std::string toString() const;
    void fromString(const std::string& str);
    void appendToString(std::string* str) const;
};

bool operator==(const Endpoint& ep1, const Endpoint& ep2);
bool operator<(const Endpoint& ep1, const Endpoint& ep2);


}} //namespace water::net

#endif //ifndef WATER_NET_BASE_HPP


