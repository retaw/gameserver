/*
 * Author: zhupengfei
 *
 * Created: 2015-08-15 14:30 +0800
 *
 * Modified: 2015-08-15 14:30 +0800
 *
 * Description: 处理翅膀晋阶、注灵相关消息
 */

#ifndef PROCESS_WORLD_WING_MANAGER_HPP
#define PROCESS_WORLD_WING_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class WingManager
{
public:
	~WingManager() = default;
    static WingManager& me();
private:
	static WingManager m_me;

public:
    void regMsgHandler();

private:
	//请求翅膀晋阶
	void clientmsg_RequestWingLevelUp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求翅膀注灵
	void clientmsg_RequestWingZhuling(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool isHeroEquipPackage(PackageType packageType);

};


}

#endif
