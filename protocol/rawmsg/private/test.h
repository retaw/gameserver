#ifndef PROTOCOL_RAWMSG_PRIVATE_LOGIN_MSG_H
#define PROTOCOL_RAWMSG_PRIVATE_LOGIN_MSG_H

#include "water/common/commdef.h"

#include <cstring>

#pragma pack(1)

namespace PrivateRaw{

//g <-> d
struct TestMsg1
{
    uint32_t num = 32353;
};

//g -> d
struct QuestRoleList1
{
    QuestRoleList()
    {
        std::memset(account, 0, sizeof account);
    }

    uint64_t loginId = 0;
    char account[ACCOUNT_BUFF_SZIE];
};

//d -> g
struct RetRoleList1
{
    RetRoleList()
    {
        std::memset(roleList, 0, sizeof roleList );
    }

    uint64_t loginId = 0;
    ArraySize listSize = 0;
    BasicUserInfo roleList[MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE];
};

//g -> d
struct SelectRole1
{
    SelectRole()
    {
        std::memset(account, 0, sizeof(account));
    }

    uint64_t loginId = 0;
    char account[ACCOUNT_BUFF_SZIE];
    RoleId rid = 0;
};

//d -> g
//这里返回意味着登陆流结束
struct RetSelectRole1
{
    uint64_t loginId;
    BasicUserInfo basicInfo;  //所选角色信息
};

//g -> d
struct CreateRole1
{
    CreateRole()
    {
        std::memset(account, 0, sizeof(account));
        std::memset(&basicInfo, 0, sizeof(basicInfo));
    }

    uint64_t loginId = 0;
    char account[ACCOUNT_BUFF_SZIE];
    BasicUserInfo basicInfo;
};

//d -> g
struct RetCreateRole1
{
    RetCreateRole()
    {
        std::memset(roleList, 0, sizeof roleList );
    }


    uint64_t loginId = 0;
    LoginRetCode code = LoginRetCode::successful;
    ArraySize listSize = 0;
    BasicUserInfo roleList[MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE];
};

enum class OfflineType : uint8_t
{
    client_offline       = 1,
    server_error_db      = 2,
    server_error_session = 3,
    server_error_fun     = 4,
    server_error_scene   = 5,
    server_kick_role     = 6,
};

//d -> g
struct RoleOffline1
{
    RoleId rid = 0;
    OfflineType type;
};

struct RetRoleOffline1
{
    RoleId rid = 0;
    OfflineType type;
};

//d -> g
struct NewClientSelectedTheRole1
{
    uint64_t rid = 0;
    uint64_t newGatewayId = 0;
};

//s -> g
struct SceneSyncRoleCroodinate1
{
    uint64_t rid = 0;
};

}

#pragma pack()

#endif
