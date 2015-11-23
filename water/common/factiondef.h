#ifndef WATER_COMMON_FACTIONDEF_H
#define WATER_COMMON_FACTIONDEF_H

#include <stdint.h>

typedef uint64_t FactionId;

//FactionId组成, 从高到低位分割：| 8bit reserve | 8bits platform | 16bits zone | 32bits 自增值 |
const FactionId FACTION_PLATFORM_MASK = 0x00ff000000000000;
const FactionId FACTION_ZONE_MASK     = 0x0000ffff00000000;
const FactionId FACTION_AUTO_INC_MASK = 0x00000000ffffffff;
const uint32_t FACTION_NOTICE_SIZE = 500;

//无效的factionId
const FactionId INVALID_FACTION_ID = 0;

enum class FactionPosition : uint8_t
{
    none = 0,
    leader = 1, //会长
    viceLeader = 2, //副会长
    warriorLeader = 3,  //战士首席
    magicianLeader = 4, //法师首席
    taoistLeader = 5,   //道士首席
    ordinary = 6,   //普通成员
};

enum class CreateFactionRet : uint8_t
{
    none = 0,
    success = 1,
    existName = 2,  //帮派名字已经存在
};

enum class LogType : uint8_t
{
   none = 0,
   addMember = 1,
   subMember = 2,
   contributeResource =3,
   levelUp = 4,
   activity = 5,
};

//沙巴克职位
enum class ShabakePosition : uint8_t
{
    none            = 0, //普通成员
    king            = 1, //城主
    leftViceKing    = 2, //左副城主
    rightViceKing   = 3, //右副城主
    dragonLeader    = 4, //龙卫团统领
    tigerLeader     = 5, //虎卫团统领
    eagleLeader     = 6, //鹰卫团统领
};

//沙巴克奖励类型
enum class ShabakeAward : uint8_t
{
    none        = 0,
    king        = 1, //城主奖
    win         = 2, //占领奖
    parttake    = 4, //参与奖
    daily       = 8, //占领日常奖
};

#endif
