/*
 * Author: zhupengfei
 *
 * Created: 2015-08-15 15:45 +0800
 *
 * Modified: 2015-08-15 15:45 +0800
 *
 * Description: 翅膀晋阶、注灵
 */

#ifndef PROCESS_WORLD_WING_HPP
#define PROCESS_WORLD_WING_HPP

#include "equip_package.h"

#include "water/common/roledef.h"


namespace world{

class PK;
class Role;

class Wing
{
public:
	explicit Wing(SceneItemType sceneItem, RoleId roleId, PK& owner);
	~Wing() = default;

public:
	void requestWingLevelUp(uint16_t cell, PackageType packageType, bool useYuanbao);
	void requestWingZhuling(uint8_t type, uint16_t cell, PackageType packageType);

private:
	std::shared_ptr<Role> getRole() const;
	
	EquipPackage::Ptr getEquipPackagePtr(PackageType packageType);
	bool isHeroEquipPackage(PackageType packageType);
	
	bool checkLevel(TplId sourceTplId, TplId destTplId);
	bool reduceMaterialOfLevelUp(TplId sourceTplId, bool useYuanbao);
	bool reduceMaterialOfZhuling(uint8_t type);	

public:
	uint8_t getLingliLevel() const;

private:
	void sendWingLevelUpResult(OperateRetCode code);
	void sendWingZhulingResult(OperateRetCode code);

private:
	const SceneItemType m_sceneItem;
	const RoleId m_roleId;
	const PK& m_owner;
};


}

#endif
