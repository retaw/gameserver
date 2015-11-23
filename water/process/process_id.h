/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-27 15:45 +0800
 *
 * Description: 进程标识
 */


#ifndef WATER_PROCESS_ID_H
#define WATER_PROCESS_ID_H

//#include "zone_id.h"

#include <string>
#include <vector>
#include <map>


namespace water{
namespace process{

enum class Platform : uint8_t
{
    superRouter = 0,
    dev = 1,
};


typedef uint32_t ZoneId;
const ZoneId INVALID_ZONE_ID = 0;

// ProcessType 
typedef uint16_t ProcessType;
const ProcessType INVALID_PROCESS_TYPE = 0;

// ProcessNum 
typedef uint16_t ProcessNum;
// ProcessIdentityValue =  |- 32bits ZoneId -|- 16bits ProcessType -|- 16bits ProcessNum -|
typedef uint64_t ProcessIdentityValue;
const int64_t INVALID_PROCESS_IDENDITY_VALUE = 0;

class ProcessIdentity
{
public:


    ProcessIdentity(ZoneId zone, const std::string& typeStr, int8_t num);
    ProcessIdentity(const std::string& typeStr, int8_t num);
    ProcessIdentity(ProcessIdentityValue value_ = INVALID_PROCESS_IDENDITY_VALUE);

    void clear();
    bool isValid() const;

    std::string toString() const;

    void setValue(ProcessIdentityValue value);
    ProcessIdentityValue value() const;

    ProcessType& type();
    ProcessType type() const;

    ProcessNum& num();
    ProcessNum num() const;

    ZoneId& zoneId();
    ZoneId zoneId() const;

    uint64_t zoneAndType() const;
    static uint64_t makeZoneAndType(ZoneId zone, ProcessType type);

private:
    ZoneId m_zoneId = 0;
    ProcessType m_type = 0;
    ProcessNum m_num = 0;

public:
    static std::string typeToString(ProcessType type);
    static ProcessType stringToType(const std::string& str);

private:
    friend class ProcessConfig;
    static std::vector<std::string> s_type2Name;
    static std::map<std::string, ProcessType> s_name2Type;
    static ZoneId s_zoneId;

};

bool operator==(const ProcessIdentity& pid1, const ProcessIdentity& pid2);
bool operator!=(const ProcessIdentity& pid1, const ProcessIdentity& pid2);
bool operator<(const ProcessIdentity& pid1, const ProcessIdentity& pid2);

}}

#endif
