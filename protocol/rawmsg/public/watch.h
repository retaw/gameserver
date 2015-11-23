/*
 * Author: zhupengfei
 *
 * Created: 2015-07-13 15:45 +0800
 *
 * Modified: 2015-07-13 15:45 +0800
 *
 * Description: 查看相关消息
 */

#ifndef PROTOCOL_RAWMSG_PUBLIC_WATCH_HPP
#define PROTOCOL_RAWMSG_PUBLIC_WATCH_HPP

#include "package.h"
#include "water/common/roledef.h"
#include "water/common/herodef.h"

#pragma pack(1)


namespace PublicRaw{

//c -> s
//请求查看角色
struct RequestWatchRole
{
	RoleId roleId;		//被观察者ID
};

//s -> c
//返回查看角色
struct RetWatchRole
{
	RoleMainData info;
	RetPackageObjList equipList;
	RetPackageObjList stoneList;
};

//c -> s
//请求查看英雄
struct RequestWatchHero
{
	RoleId roleId;
};

//s -> c
//返回查看英雄
struct RetWatchHero
{
	HeroMainData info;
	RetPackageObjList equipList;
	RetPackageObjList stoneList;
};


}

#pragma pack()


#endif
