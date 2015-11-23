#ifndef RAWMSG_PRIVATE_SKILL_HPP
#define RAWMSG_PRIVATE_SKILL_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> db
struct ModifySkillData
{
    RoleId roleId   = 0;
    Job job;
    uint8_t sceneItem = 1;//角色使用默认值,英雄需要赋值为2
    ModifyType modifyType;
    ArraySize size  = 0;
    SkillData data[0];
};

// 缓存pk被动技能状态,不需要存数据库
// world -> db
struct CachePKCdStatus
{
    RoleId roleId = 0;
    Job job;
    uint8_t sceneItem = 1;//角色使用默认值,英雄需要赋值为2
    ArraySize size = 0;
    PKCdStatus data[0];
};

}

#pragma pack()


#endif
