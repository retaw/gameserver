/*
 * Author: zhupengfei
 *
 * Created: 2015-07-14 14:46 +0800
 *
 * Modified: 2015-07-14 14:46 +0800
 *
 * Description: 查看相关消息
 */

#ifndef PROTOCOL_RAWMSG_PRIVATE_WATCH_HPP
#define PROTOCOL_RAWMSG_PRIVATE_WATCH_HPP

#include "protocol/rawmsg/public/package.h"

#include "water/common/roledef.h"
#include "water/common/herodef.h"

#pragma pack(1)

using namespace PublicRaw;

namespace PrivateRaw{

//func -> world
//请求查看角色
struct RequestWatchRoleToWorld
{
	RoleId watcherId;	//观察者Id
	RoleId roleId;		//被观察者ID
};

//world -> Func
//返回角色主数据及装备信息
struct RetWatchRoleToFunc
{
	RoleId watcherId;	//观察者Id
	ArraySize equipBufSize;
	ArraySize stoneBufSize; 
	RoleMainData info;
	RetPackageObjList equipList;
	RetPackageObjList stoneList;
};

//func -> world
//请求查看英雄
struct RequestWatchHeroToWorld
{
	RoleId watcherId;	//观察者Id
	RoleId roleId;
};

//world -> func
//返回查看英雄主数据及装备信息
struct RetWatchHeroToFunc
{
	RoleId watcherId;	//观察者Id
	bool summon = false;
	ArraySize equipBufSize;
	ArraySize stoneBufSize; 
	HeroMainData info;
	RetPackageObjList equipList;
	RetPackageObjList stoneList;
};


}

#pragma pack()


#endif
