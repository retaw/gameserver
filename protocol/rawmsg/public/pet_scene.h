/*
 * Description: 宠物场景消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_PET_SCENE_HPP
#define PROTOCOL_RAWMSG_PUBLIC_PET_SCENE_HPP

#include "water/common/roledef.h"
#include "water/componet/coord.h"

#pragma pack(1)

using water::componet::Coord1D;

namespace PublicRaw{

struct PetScreenData
{
    char roleName[MAX_NAME_SZIE+1];  //角色名称
    uint8_t ownerSceneItem; //主人sceneItem 关联 SceneItemType
    PKId id;            //召唤兽唯一id
    uint32_t tplId;     //召唤兽配置表id
    Coord1D posx;
    Coord1D posy;
    uint8_t dir;
    uint32_t maxhp;
    uint32_t hp;
    uint32_t pkStatus;
    FactionId factionId; //主角帮派id
};

//s -> c
//周围宠物的九屏数据 (九屏)
struct PetsAroundMe
{
    ArraySize size = 0;
    PetScreenData pets[0];
};

//s -> c
//宠物离开视野 (九屏)
struct PetLeaveInfo
{
    ArraySize size = 0;
	PKId id[0]; 
};

//s -> c
//广播宠物九屏数据 (九屏)
struct BroadCastPetScreenDataToNine
{
	PetScreenData data;
};

//s -> c
//更新宠物位置信息
struct UpdatePetPosToClient
{
	PKId id;
	Coord1D posx;
	Coord1D posy;
	uint8_t dir;
	MoveType type;
};

//c -> s
//宠物要求移动到某个点
struct PetRequestMoveToPos
{
    PKId id = 0;
    Coord1D posx = 0;
    Coord1D posy = 0;
    uint8_t dir  = 1;
    MoveType type = MoveType::walk;
};

//c -> s
//设定宠物ai模式
struct PetAIMode
{
    uint8_t aiMode = 0; // 0 跟随; 2 主动
};


}

#pragma pack()


#endif
