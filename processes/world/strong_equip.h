/*
 * Author: zhupengfei
 *
 * Created: 2015-07-18 09:13 +0800
 *
 * Modified: 2015-07-18 09:13 +0800
 *
 * Description: 处理装备强化逻辑
 */

#ifndef PROCESS_WORLD_STRONG_EQUIP_HPP
#define PROCESS_WORLD_STRONG_EQUIP_HPP

#include "equip_package.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class StrongEquip
{
public:
	~StrongEquip() = default;
    static StrongEquip& me();
private:
	static StrongEquip m_me;

public:
    void regMsgHandler();

private:
	//请求强化装备
	void clientmsg_RequestStrongEquip(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool isHeroEquipPackage(PackageType packageType);

	bool reduceMaterial(RoleId roleId, uint8_t nextLevel, bool bProtect, bool autoBuy);
	
	EquipPackage::Ptr getEquipPackagePtr(RoleId roleId, PackageType packageType);

	void sendStrongEquipResult(RoleId roleId, uint16_t cell, PackageType packageType, uint8_t oldLevel, uint8_t newLevel);


};

}

#endif
