#include "process_id.h"

#include "process_config.h"
#include "componet/format.h"



namespace water{
namespace process{

std::vector<std::string> ProcessIdentity::s_type2Name = {"none"};
std::map<std::string, ProcessType> ProcessIdentity::s_name2Type;
ZoneId ProcessIdentity::s_zoneId;

std::string ProcessIdentity::typeToString(ProcessType type)
{
    uint32_t index = static_cast<uint32_t>(type);
    if(index >= s_type2Name.size())
        return "none";

    return s_type2Name[index];
}

ProcessType ProcessIdentity::stringToType(const std::string& str)
{
    auto it = s_name2Type.find(str);
    if(it == s_name2Type.end())
        return INVALID_PROCESS_TYPE;

    return it->second;
}

uint64_t ProcessIdentity::makeZoneAndType(ZoneId zone, ProcessType type)
{
    return (static_cast<uint64_t>(zone) << 32) | (static_cast<uint64_t>(type) << 16);
}

ProcessIdentity::ProcessIdentity(ZoneId zoneId, const std::string& typeStr, int8_t num)
{
    ProcessType type = ProcessIdentity::stringToType(typeStr);
    if(type == INVALID_PROCESS_TYPE)
    {
        m_type = 0;
        m_num = 0;
        return;
    }

    m_zoneId = zoneId;
    m_type   = type;
    m_num    = num;//(uint32_t(platform) << 24) | (uint32_t(zoneId) << 8) | uint32_t(num);
}

ProcessIdentity::ProcessIdentity(const std::string& typeStr, int8_t num)
{
    ProcessType type = ProcessIdentity::stringToType(typeStr);
    if(type == INVALID_PROCESS_TYPE)
    {
        m_type = 0;
        m_num = 0;
        return;
    }

    m_zoneId = s_zoneId;
    m_type = type;
    m_num  = num; //(uint32_t(s_platform) << 24) | (uint32_t(num) <<  16) | (uint32_t(s_zoneId));
}

ProcessIdentity::ProcessIdentity(ProcessIdentityValue value)
{
    m_zoneId = value >> 32;
    m_type   = (value & 0xffff0000) >> 16;
    m_num    = value & 0xffff;
}

std::string ProcessIdentity::toString() const
{
    std::string ret;
    /*
    componet::appendToString(&ret, m_num);
    ret.append("-");
    */
    ret.append(typeToString(m_type));
    ret.append("-");
    componet::appendToString(&ret, m_num);
    return ret;
}

void ProcessIdentity::clear()
{
    m_zoneId = 0;
    m_type = 0;
    m_num = 0;
}

bool ProcessIdentity::isValid() const
{
    return typeToString(m_type) != "none" && m_num != 0;
}

ProcessIdentityValue ProcessIdentity::value() const
{
    return (ProcessIdentityValue(m_zoneId) << 32) | (m_type << 16) | m_num;
}

void ProcessIdentity::setValue(ProcessIdentityValue value)
{
    m_zoneId = value >> 32;
    m_type   = (value & 0xffff0000) >> 16;
    m_num    = value & 0xffff;
}

ProcessType& ProcessIdentity::type() 
{
    return m_type;
}

ProcessType ProcessIdentity::type() const
{
    return m_type;
}

ProcessNum& ProcessIdentity::num()
{
    return m_num;
}

ProcessNum ProcessIdentity::num() const
{
    return m_num;
}

ZoneId& ProcessIdentity::zoneId()
{
    return m_zoneId;
}

ZoneId ProcessIdentity::zoneId() const
{
    return m_zoneId;
}

uint64_t ProcessIdentity::zoneAndType() const
{
    return value() & 0xffffffffffff0000;
}

bool operator==(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value() == pid2.value();
}

bool operator!=(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value() != pid2.value();
}

bool operator<(const ProcessIdentity& pid1, const ProcessIdentity& pid2)
{
    return pid1.value() < pid2.value();
}


}}

