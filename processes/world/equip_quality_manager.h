/*
 * Author: zhupengfei
 *
 * Created: 2015-08-14 11:30 +0800
 *
 * Modified: 2015-08-14 11:30 +0800
 *
 * Description: 处理装备升品逻辑
 */

#ifndef PROCESS_WORLD_EQUIP_QUALITY_MANAGER_HPP
#define PROCESS_WORLD_EQUIP_QUALITY_MANAGER_HPP

#include "equip_package.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class EquipQualityManager
{
public:
	~EquipQualityManager() = default;
    static EquipQualityManager& me();
private:
	static EquipQualityManager m_me;

public:
    void regMsgHandler();

private:
	//请求装备升品
	void clientmsg_RequestImproveEquipQuality(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool reduceMaterial(RoleId roleId, uint32_t sourceTplId, bool autoBuy);
	
	Package::Ptr getPackagePtr(RoleId roleId, PackageType packageType);
	bool isBelongRolePackage(PackageType packageType);
	bool isHeroEquipPackage(PackageType packageType);

	void sendImproveResult(RoleId roleId, OperateRetCode code);
};

}

#endif
