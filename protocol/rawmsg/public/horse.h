#ifndef PROTOCOL_RAWMSG_PUBLIC_HORSE_H
#define PROTOCOL_RAWMSG_PUBLIC_HORSE_H

#include "water/common/roledef.h"
#include "water/common/herodef.h"

#pragma pack(1)

namespace PublicRaw{

struct RaiseRateEle
{
    uint8_t rate = 0;
    uint16_t raiseCount = 0; //培养次数
    uint8_t effectNum = 0;  //已作用次数
};

//c -> s
//请求培养界面信息
struct RequestRaiseInfo
{
};

//s -> c
//返回培养界面信息
struct RetRaiseInfo
{
    uint8_t star;   //阶
    uint16_t curskin; //当前使用的皮肤id
    uint32_t exp;   //当前品阶的经验
    RaiseRateEle ele[4];
};

//c -> s
//请求上/下马
struct RequestRide
{
    uint8_t state = 1; //1 上马 0 下马
};

//s -> c
//上下马状态播放九屏
struct BroadcastRide9
{
    RoleId roleId;
    uint8_t state;
    uint16_t skin;
    
    //以下为上马英雄数据
    Job heroJob;
    Sex heroSex;
    uint32_t heroClother;
};

//c -> s
//请求培养
struct RequestRaise
{
    uint32_t objId; //道具ID
    uint8_t autoyb; //是否道具不足自动扣元宝(0:不是 1:是)
};

//s -> c
//返回培养成功与否
struct RetRaiseResult
{
    uint8_t retcode; //0,fail 非0表示触发倍率
};

//c -> s
//请求幻化皮肤
struct RequestHuanHuaSkin
{
    uint16_t skin;
};

//s -> c
//返回当前使用的皮肤
struct RetCurSkin
{
    uint16_t skin;
};

//s -> c
//幻化皮肤(九屏协议,只有骑乘状态才会有该协议)
struct HuanHuaSkin
{
    RoleId roleId;
    uint16_t skin;
};

//c -> s
//请求激活的皮肤列表
struct RequestActiveSkins
{
};

//s -> c
//返回已经激活的皮肤列表
struct RetActiveSkins
{
    ArraySize size = 0;
    uint16_t skin[0];
};

}

#pragma pack()

#endif
