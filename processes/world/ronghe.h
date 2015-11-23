/*
 * Author: zhupengfei
 *
 * Created: 2015-08-10 16:52 +0800
 *
 * Modified: 2015-08-10 16:52 +0800
 *
 * Description: 处理装备融合逻辑
 */

#ifndef PROCESS_WORLD_RONGHE_HPP
#define PROCESS_WORLD_RONGHE_HPP

#include "equip_package.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class Ronghe
{
public:
	~Ronghe() = default;
    static Ronghe& me();
private:
	static Ronghe m_me;

public:
    void regMsgHandler();

private:
	//请求融合装备
	void clientmsg_RequestRongheEquip(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool reduceMaterial(RoleId roleId, uint8_t strongLevel, bool autoBuy);
	
	EquipPackage::Ptr getEquipPackagePtr(RoleId roleId, PackageType packageType);

	bool isHeroEquipPackage(PackageType packageType);

	void sendRongheResult(RoleId roleId, OperateRetCode code);


};

}

#endif
