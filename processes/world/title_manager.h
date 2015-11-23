/*
 * Author: zhupengfei
 *
 * Created: 2015-07-24 14:24 +0800
 *
 * Modified: 2015-07-24 14:24 +0800
 *
 * Description: 处理称号相关消息
 */

#ifndef PROCESS_WORLD_TITLE_MANAGER_HPP
#define PROCESS_WORLD_TITLE_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <cstdint>


namespace world{

using water::process::ProcessIdentity;

class TitleManager
{
public:
	~TitleManager() = default;
    static TitleManager& me();
private:
	static TitleManager m_me;

public:
    void regMsgHandler();

private:
	//请求称号列表
	void clientmsg_RequestTitleList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

	//请求佩戴或取下普通称号
	void clientmsg_RequestOperateNormalTitle(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);


};


}

#endif
