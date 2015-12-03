/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-23 09:55 +0800
 *
 * Modified: 2015-03-23 09:55 +0800
 *
 * Description: 客户端登录相关的消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_LOGIN_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_LOGIN_MSG_H


#include <cstring>
#include <cstdint>

#include "../commdef.h"

#pragma pack(1)

namespace PublicRaw{

//s -> c
//发送角色列表给客户端
struct LoginRetRoleList
{
    LoginRetRoleList()
    {
        std::memset(roleList, sizeof(roleList), 0);
    }

    ArraySize listSize = 0;
    RoleBasicData roleList[MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE];
};


//c -> s
//选择要登录的角色
struct SelectRole
{
    RoleId rid = 0;
};

//c -> s
//创建角色
struct CreateRole
{
    CreateRole()
    {
        std::memset(&basicInfo, sizeof(basicInfo), 0);
    }

    RoleBasicData basicInfo;
};

//s -> c
//建角成功后的新角色列表
struct RetCreateRole
{
    ArraySize listSize = 0;
    RoleBasicData roleList[MAX_ROLE_NUM_PER_ACCOUNT_ONE_ZONE];
};

//s -> c
//登陆流程出错
struct LoginRet
{
    LoginRetCode ret = LoginRetCode::successful;
//    RoleId rid;
};

//c -> s
//请求随机名
struct GetRandName
{
    Sex sex;    
};

//s -> c
//返回随机名
struct RetRandName
{
    char name[NAME_BUFF_SZIE];    
};

//s -> c
//被动下线
struct UnexpectedLogout
{
    enum class Code: uint8_t
    {
        repetionLogin = 0, //同账号异地登陆, 被挤下线
    };

    Code code = Code::repetionLogin;
};

}

#pragma pack()

#endif
