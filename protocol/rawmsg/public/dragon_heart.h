#ifndef PROTOCOL_RAWMSG_PUBLIC_DRAGON_HEART_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_DRAGON_HEART_MSG_H

#include "water/common/commdef.h"
#pragma pack(1)

namespace PublicRaw{

//c -> s
//请求炼心技能界面数据
struct RequestDragonSkills
{
};

//s -> c
//返回炼心技能列表
struct RetDragonSkills
{
    uint16_t energeLimit = 0;   //龙心上限
    uint16_t energe = 0;        //当前龙心值
    ArraySize size = 0;
    struct DragonSkillInfo
    {
        uint16_t id;
        uint16_t level;
    } data[0];
};

//s -> c
//更新单个炼心技能
struct RefreshDragonSkill
{
    uint16_t id;
    uint16_t level;
};

//c -> s
//请求提升炼心技能
struct RequestUpgradeDragonSkill
{
    uint16_t id;
};

//c -> s
//请求充能
struct RequestAddEnerge
{
    uint8_t autoyb = 0; //1:一键自动(道具不足扣除元宝)  0:普通
};

//s -> c
//一键自动充能失败
struct AutoAddEnergeFail
{
};

//s -> c
//刷新龙心能量
struct RetEnerge
{
    uint16_t energe = 0;
};


//s -> c
//刷新怒气值
struct RefreshAnger
{
    uint16_t angerLimit;    //怒气上限
    uint16_t anger;         //当前怒气值
};

}

#pragma pack()

#endif

