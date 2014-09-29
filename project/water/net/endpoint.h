/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-28 19:05 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_ENDPOINT_HPP
#define WATER_NET_ENDPOINT_HPP

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
    void fromString(const std::string& str);
    static IpV4 getAddrByIfrName(const std::string& ifrName);
};
std::ostream& operator << (std::ostream& os, const IpV4& ip);
std::istream& operator >> (std::istream& is, IpV4& ip);

struct Endpoint
{
    IpV4 ip;
    uint16_t port = 0;
};


}} //namespace water::net

#endif //ifndef WATER_NET_BASE_HPP


