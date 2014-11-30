/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-27 15:45 +0800
 *
 * Description: 进程标识
 */


#ifndef WATER_PROCESS_ID_H
#define WATER_PROCESS_ID_H

#include<string>


namespace water{

enum class ProcessType : int32_t
{
    none = -1,
    router = 0,
    function = 1,
    gateway = 2,
};

std::string processTypeToString(ProcessType type);
ProcessType stringToProcessType(std::string& str);

const int64_t INVALID_PROCESS_IDENDITY_VALUE = -1;

struct ProcessIdentity
{
    ProcessIdentity() = default;
    ProcessIdentity(ProcessType type_, int32_t num_);

    void appendToString(std::string* str) const;

    union
    {
        struct
        {
            union
            {
                ProcessType type;
                int32_t typeValue;
            };
            int32_t num;
        };
        int64_t value = INVALID_PROCESS_IDENDITY_VALUE;
    };

};

bool operator==(const ProcessIdentity& pid1, const ProcessIdentity& pid2);
bool operator<(const ProcessIdentity& pid1, const ProcessIdentity& pid2);

}

#endif
