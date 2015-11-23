#ifndef PROTOCOL_RAWMSG_PUBLIC_FRIEND_HPP
#define PROTOCOL_RAWMSG_PUBLIC_FRIEND_HPP

#include "water/common/roledef.h"
#include "water/common/frienddef.h"
#include "water/common/commdef.h"

#pragma pack(1)
namespace PublicRaw{
//client->s
//1.请求添加好友
struct RequestFriendToS
{
    ArraySize size = 0; //uint16_t
    struct RequestFriend
    {
        RoleId friendId;    //uint64_t
    }data[0];
};

//s->c
//2.请求添加好友
struct RequestFriendToC
{
    RoleId roleId;
    uint32_t level;
    char name[NAME_BUFF_SZIE];  //33字节
    //...
};

//c->s
//3.请求好友回复,不定长
struct RetRequestFriendToS
{
    RequestFdRely retStatus;    //1, 同意; 0, 不同意; uint8_t...
    ArraySize size = 0;
    struct RetRequestFriend
    {
        RoleId roleId;
    }data[0];
};

//s->c
//4.通知客户端添加了一个好友
struct RetRequestFriendToC
{
    RoleId friendId;
    uint32_t level;
    char name[NAME_BUFF_SZIE];
    //...
};

//c->s
//5.删除好友
struct EraseFriendToS
{
    ArraySize size = 0;
    struct EraseFriend
    {
        RoleId roleId;    
    }data[0];
};
//6.通知客户端删除了一个好友
struct RetEraseFriendToC
{
    RoleId roleId;
};

//c->s
//7.添加黑名单
struct InsertBlackList
{
   RoleId roleId; 
};
//s->c
//8.添加黑名单回复
struct RetInsertBlackList
{
    RoleId blackId;
    char name[NAME_BUFF_SZIE];
    uint32_t level;
};

//c->s
//9.取消黑名单
struct EraseBlackList
{
    RoleId roleId;
};
//s->c
//10.取消黑名单回复
struct RetEraseBlackList
{
    RoleId blackId;
};

//c->s
//11.打开好友界面请求所有有关系的role信息(好友、黑名单、仇人)
struct RequestAllRelationerInfo
{
    RoleId roleId;
};

//s->c
//12.回复所有有关系的role信息
struct RetAllRelationerInfo
{
    ArraySize friendSize = 0;
    ArraySize blackSize = 0;
    ArraySize enemySize = 0;
    struct RelationerInfo
    {
        IsOnln isOnln = IsOnln::no; //uint8_t
        RoleId roleId;
        uint32_t level; 
        char name[NAME_BUFF_SZIE];
    }data[0];
};
//s->c
//13.添加仇人
struct AddEnemy
{
    RoleId enemyId;
    char name[NAME_BUFF_SZIE];
    uint32_t level;
    BeKilledType beKilledType;  //1：死的是角色，2：死的是英雄
};
//c->s
//14.删除仇人
struct EraseEnemy
{
    RoleId enemyId;
};
//s->c
//15.删除仇人回复
struct RetEraseEnemy
{
    RoleId enemyId;
};

//c->s
//16.推荐好友request
struct RequestRecommendFriend
{
};

//s->c
//17.发给客户端推荐好友列表
struct RetRecommendFriend
{
    ArraySize friendSize = 0;
    struct Friend
    {
        RoleId friendId;
        char name[NAME_BUFF_SZIE];
        uint32_t level;
    }data[0];
};

}
#pragma pack()

#endif
