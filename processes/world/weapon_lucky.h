/*
 * Author: zhupengfei
 *
 * Created: 2015-08-01 13:58 +0800
 *
 * Modified: 2015-08-01 13:58 +0800
 *
 * Description: 处理武器幸运消息及逻辑
 */

#ifndef PROCESS_WORLD_WEAPON_LUCKY_HPP
#define PROCESS_WORLD_WEAPON_LUCKY_HPP

#include "equip_package.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class WeaponLucky
{
public:
	~WeaponLucky() = default;
    static WeaponLucky& me();
private:
	static WeaponLucky m_me;

public:
    void regMsgHandler();

private:
	//请求提升武器幸运
	void clientmsg_RequestWeaponLucky(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool reduceMaterial(RoleId roleId, uint8_t nextLevel, bool bProtect, bool autoBuy);

	EquipPackage::Ptr getEquipPackagePtr(RoleId roleId, PackageType packageType);
	
	bool isHeroEquipPackage(PackageType packageType);
	
	void sendWeaponLuckyResult(RoleId roleId, PackageType packageType, uint8_t oldLevel, uint8_t newLevel);


};

}

#endif
