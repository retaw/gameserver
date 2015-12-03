#ifndef PROTOCOL_RAWMSG_PRIVATE_LOGIN_MSG_H
#define PROTOCOL_RAWMSG_PRIVATE_LOGIN_MSG_H

#include "../commdef.h"

#pragma pack(1)

namespace PrivateRaw{


//g <-> d
struct TestMsg
{
    uint32_t num = 32353;
};

//g -> d
struct QuestRoleList
{
    QuestRoleList()
    {
        std::memset(account, 0, sizeof account);
    }

    uint64_t loginId = 0;
    char account[ACCOUNT_BUFF_SZIE];
};

//d -> g
struct RetRoleList
{
    RetRoleList()
    {
        std::memset(roleList, 0, sizeof roleList );
    }

    uint64_t loginId = 0;
    ArraySize listSize = 0;
    RoleBasicData roleList[MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE];
};

//g -> d
struct CreateRole
{
    CreateRole()
    {
        std::memset(account, 0, sizeof(account));
        std::memset(&basicInfo, 0, sizeof(basicInfo));
    }

    uint64_t loginId = 0;
    char account[ACCOUNT_BUFF_SZIE];
    RoleBasicData basicInfo;
};

//d -> g
struct RetCreateRole
{
    RetCreateRole()
    {
        std::memset(roleList, 0, sizeof roleList );
    }


    uint64_t loginId = 0;
    LoginRetCode code = LoginRetCode::successful;
    ArraySize listSize = 0;
    RoleBasicData roleList[MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE];
};

//gateway -> session
//上线
//发往session而不是db的原因, 是因为可能要sesion处理挤下线的逻辑
struct RoleOnline
{
    LoginId loginId;
    RoleId rid = 0;
};

enum class OfflineType : uint8_t
{
    logout           = 1,
    dbError          = 2,
    sessionError     = 3,
    worldError       = 4,
    kickRole         = 5,
    replace          = 6,
};

//gateway->session
//session->gateway
//下线
struct RoleOffline
{
    RoleId rid = 0;
    OfflineType type;
    LoginId loginId = 0; //只有type是正常下线时, 消息从网关发出, 这个字段才有意义
};

// -> gateway
// 使gateway知晓role所在的world
struct UpdateRoleWorldId
{

    RoleId rid;
    uint64_t worldId;
};

//gateway -> dbcached
//请求随机名
struct GetRandName
{
    uint64_t loginId = 0;
    Sex sex;
};
//dbcached -> gateway
//回随机名
struct RetRandName
{
    uint64_t loginId = 0;
    char name[NAME_BUFF_SZIE];
};

}

#pragma pack()

#endif
