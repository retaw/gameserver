/*
 * Author: zhupengfei
 *
 * Created: 2015-05-25 15:43 +0800
 *
 * Modified: 2015-05-25 15:43 +0800
 *
 * Description: 处理角色操作背包及场景物品相关的消息逻辑
 */

#ifndef PROCESS_WORLD_ROLE_PACKAGE_HPP
#define PROCESS_WORLD_ROLE_PACKAGE_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class RolePackage
{
public:
	~RolePackage() = default;
    static RolePackage& me();
private:
	static RolePackage m_me;

public:
    void regMsgHandler();

private:
	//请求背包物品列表
	void clientmsg_RequestObjList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求背包已解锁的格子数
	void clientmsg_RequestUnlockCellNum(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求摧毁一定数量的某类型物品, 且指定格子
	void clientmsg_RequestDestoryObjByCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
		
	//请求丢弃一定数量的某类型物品(且指定格子)
	void clientmsg_RequestDiscardObjByCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求拆分物品数量
	void clientmsg_RequestSplitObjNum(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求交换格子, 支持同一背包内及跨背包
	void clientmsg_RequestExchangeCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求存取物品，跨背包, 不指定格子
	void clientmsg_RequestMoveObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求整理物品
	void clientmsg_RequestSortObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //请求出售道具
	void clientmsg_RoleRequestSellObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    //请求回购道具
	void clientmsg_RoleRequestRepurchaseObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求正在解锁的格子剩余解锁时间
	void clientmsg_RequestUnlockCellNeedSec(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求解锁格子需要的元宝数
	void clientmsg_RequestUnlockCellNeedMoney(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求解锁格子
	void clientmsg_RequestUnlockCell(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);


	/************************************* 场景物品 *************************************/
	//请求拾取物品
	void clientmsg_RequestPickupObject(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool isBelongHeroPackage(PackageType packageType);
	bool isHeroEquipPackage(PackageType packageType);
	bool isHeroStonePackage(PackageType packageType);

	//跨背包集移动物品
	void moveObjCrossPackageSet(RoleId roleId, uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage);

	//将物品移到英雄装备包, 特殊处理
	void moveObjToHeroStonePackage(RoleId roleId, uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage);
};


}

#endif
