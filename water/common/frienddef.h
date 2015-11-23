#ifndef WATER_COMMON_FRIENDDEF_HPP
#define WATER_COMMON_FRIENDDEF_HPP

//杀人者的类型
enum class BeKilledType : uint8_t
{
    none = 0,
    role = 1,
    hero = 2,
};
//是否同意好友添加请求
enum class RequestFdRely : uint8_t
{
    refuse = 0,
    agree = 1,
};

//标识关系类型
enum class RelationType : uint8_t
{
    fd           = 1,
    balackList   = 2,
    enemy        = 3, 
};

//标识,是否在线
enum class IsOnln : uint8_t
{
    no    = 0,
    yes   = 1,
};

//是否存在
enum class Exist : uint8_t
{
    no = 0,
    yes = 1,
};
//是否在黑名单中
enum class IsInBlack : uint8_t
{
    no = 0,
    yes = 1,
    inopposite = 2,//在对方的黑名单中
};

/****************************组队*************************/
typedef uint64_t TeamId;

enum class ApplyJoinTeamFailType : uint8_t
{
    refuse = 1, //队长拒绝
    full = 2,   //队伍已满
};


#endif
