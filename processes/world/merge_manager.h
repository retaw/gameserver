/*
 * Author: zhupengfei
 *
 * Created: 2015-08-13 19:32 +0800
 *
 * Modified: 2015-08-13 19:32 +0800
 *
 * Description: 处理合成逻辑
 */

#ifndef PROCESS_WORLD_MERGE_MANAGER_HPP
#define PROCESS_WORLD_MERGE_MANAGER_HPP

#include "equip_package.h"

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class MergeManager
{
public:
	~MergeManager() = default;
    static MergeManager& me();
private:
	static MergeManager m_me;

public:
    void regMsgHandler();

private:
	//请求合成物品
	void clientmsg_RequestMergeObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

private:
	bool reduceMaterial(RoleId roleId, uint32_t mergeTplId, uint8_t num);
	
	void sendMergeResult(RoleId roleId, OperateRetCode code);


};

}

#endif
