/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-21 14:51 +0800
 *
 * Modified: 2015-04-21 14:51 +0800
 *
 * Description:角色pk相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_ROLE_PK_MSG_H
#define PROTOCOL_RAWMSG_PUBLIC_ROLE_PK_MSG_H

#include "water/common/scenedef.h"
#include "water/common/commdef.h"
#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{


//s -> c
//下发技能列表()
struct SkillListToClient
{
    uint8_t sceneItem;
    Job job;
    ArraySize size      = 0;
    SkillData data[0];
};

//c -> s
//升级/学习主角技能
struct RoleRequestUpgradeSkill
{
    uint32_t skillId = 0;
};


//c -> s
//升级/学习英雄技能
struct HeroRequestUpgradeSkill
{
    uint32_t skillId = 0;
};


//c -> s
//强化主角技能
struct RoleRequestStrengthenSkill
{
    uint32_t skillId = 0;
};


//c -> s
//强化英雄技能
struct HeroRequestStrengthenSkill
{
    uint32_t skillId = 0;
};

//s -> c
//刷新单个技能数据
struct RefreshSkill
{
    SkillData data;
};


//s -> c
//通知前端设置合击准备状态
struct NotifyReadyState
{
    bool clear = false; //false:设置准备状态 true:清除准备状态
    uint16_t readyTime = 0;  //准备状态时长
};


//c -> s
//请求攻击
struct RoleRequestAttack
{
    PKId atkId          = 0;    //攻击者ID
    uint8_t atkModel    = 0;    //攻击者模型
    uint8_t step        = 0;    //0:只是播放攻击动作  1:计算伤害  2:动作和伤害都一起 3:合击准备
    uint32_t skillId    = 0;
    uint8_t dir         = 0;    //方向
    Coord1D posX        = 0;
    Coord1D posY        = 0;

    PKId beAtkId        = 0;    //被攻击者id
    uint8_t beAtkModel  = 0;    //被攻击者model
};


//s -> c
//攻击成功返回
struct RetAttackSucess
{
    uint32_t skillId    = 0;
    uint32_t level      = 0;
    uint8_t doubleEffect= 0; //0:表示一次攻击特效, 1:两次特效
    PKId  attId         = 0;
    uint8_t sceneItem     = 1;
    uint8_t dir         = 1;
    Coord1D posX        = 0;
    Coord1D posY        = 0;
};


//s -> c
//冲锋返回
struct ChongfengMsgToNine
{
    MoveType type;
    uint32_t skillId    = 0;
    uint32_t level      = 0;
    PKId  attId         = 0;
    uint8_t attSceneItemType  = 1;
    PKId  defId         = 0; 
    uint8_t defSceneItemType  = 1; 
    uint8_t dir         = 1;
    Coord1D posX        = 0; //终点坐标
    Coord1D posY        = 0;
};


//s -> c
//攻击失败返回
struct RetAttackFail
{
    uint32_t skillId    = 0;
    uint8_t retcode     = 0;
};


//s -> c
//返回触发的被动技能
struct RetReleasedPassiveSkill
{
    ArraySize size = 0;
    uint32_t skillId[0];
};



//s -> c
//玩家血量变化(九屏伤害包)
struct HpChangedToNine
{
    PKId id         = 0;
    uint8_t sceneItem = 1;
    uint16_t type   = 0;//关联 HPChangeType
    uint32_t hp     = 0;//当前血量
    PKId attackId   = 0;      //攻击者id
    uint8_t attackSceneItemType = 0;//攻击者sceneItem
    uint8_t continued = 0; //1:持续性伤害掉血 0:单次伤害
};


//s -> c
//玩家魔法变化(九屏伤害包)
struct MpChangedToNine
{
    PKId id         = 0;
    uint8_t sceneItem = 1;
    uint32_t mp     = 0;//当前mp
    PKId attackId   = 0;
    uint8_t attackSceneItemType = 0;
};


//s -> c
//更新玩家当前mp值
struct UpdateSelfMp
{
    uint8_t sceneItem = 1;
    uint32_t mp     = 0;//当前mp
};


//s -> c
//刷新技能CD
struct RefreshSkillCD
{
    PKId id             = 0;
    uint8_t sceneItem     = 1;
    uint32_t skillId    = 0; //技能ID
};

//s -> c
//通知某个技能下次释放有两次技能特效
struct NotifyDoubleSkillEffect
{
    uint32_t skillId    = 0;
};


//c -> s
//请求切换攻击模式
struct RequestChangeAttackMode
{
    uint8_t mode;
};

//s -> c
//通知端设置攻击模式
struct NotifyAttackMode
{
    uint8_t mode;
};

//s -> c
//刷新罪恶值
struct RefreshPKValue
{
    uint16_t val;
};

//s -> c
//sceneItem名字颜色更新(九屏)
struct SyncNameColorToNine
{
    PKId    id;
    uint8_t sceneItem;
    uint8_t namecolor;
};


/****************************经验 死亡复活 begin****************************/
//s -> c
//更新角色当前等级获得的经验
struct RetRoleCurLevelGotExp
{
	uint64_t gotExp;	//当前等级获得的经验
    uint16_t expRate;
};

//s -> c
//返回角色死亡
struct RetRoleDie
{
	RoleId roleId = 0;		
	char attackerName[NAME_BUFF_SZIE];	//攻击者名字
	uint64_t dieTimer = 0;				//死亡时间戳
	uint32_t sec = 0;					//回城复活倒计时
};

//s -> c
//返回角色死亡(九屏消息)
struct RetRoleDieToNine
{
	RoleId roleId = 0;		
};

//c -> s
//请求复活角色
struct RequestReliveRole
{
	RoleId roleId = 0;
	ReliveType type = ReliveType::reliveArea;	//复活方式 0复活点复活 1完美原地复活、2虚弱原地复活
};


//s -> c
//服务器告诉前端播放一个假死的死亡动作(不弹复活面板)
//struct RoleFeignDeath
//{
//};


/****************************经验 死亡复活 end****************************/

}

#pragma pack()


#endif
