#ifndef PROTOCOL_RAWMSG_PUBLIC_BUFF_SCENE_H
#define PROTOCOL_RAWMSG_PUBLIC_BUFF_SCENE_H

#include "water/common/roledef.h"

#pragma pack(1)


namespace PublicRaw{

//s -> c
//添加buff(九屏)
struct AddBuff
{
    PKId    id;
    uint8_t sceneItem;
    ArraySize size  = 0;
    uint32_t buffId[0];
};


//s -> c
//删除某个buff
struct EraseBuff
{
    PKId    id;
    uint8_t sceneItem;
    uint32_t buffId;
};


//c -> s
//请求选中对象的buff列表
struct RoleRequestSelectedBuff
{
    PKId    id; //目标id
    uint8_t sceneItem;//目标pk类型(如角色,npc,英雄等等)
};

//s -> c
//返回选中对象的buff列表
struct RetSelectedBuff
{
    PKId    id; //目标id
    uint8_t sceneItem;//目标pk类型(如角色,npc,英雄等等)
    ArraySize size  = 0;
    uint32_t buffId[0];
};


//c -> s
//请求某个buff的Tips信息(服务器记载的buff数据,如倒计时等等)
struct RoleRequestSelectedBuffTips
{
    PKId    id;
    uint8_t sceneItem;
    uint32_t buffId;
};

//s -> c
//返回指定buff的Tips信息
struct RetSelectedBuffTips
{
    PKId    id;
    uint8_t sceneItem;
    uint32_t buffId;
    uint32_t time;
    uint32_t dur;
};
}

#pragma pack()

#endif
