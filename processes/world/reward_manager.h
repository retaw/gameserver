/*
 * Author: zhupengfei
 *
 * Created: 2015-08-31 17:07 +0800
 *
 * Modified: 2015-08-31 17:07 +0800
 *
 * Description: 奖励管理器
 */

#ifndef PROCESS_WORLD_REWARD_MANAGER_HPP
#define PROCESS_WORLD_REWARD_MANAGER_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"

#include <memory> 
#include <cstdint>

namespace world{

using water::process::ProcessIdentity;

class RewardManager
{
public:
	~RewardManager() = default;
    static RewardManager& me();
private:
	static RewardManager m_me;

public:
	//C->S 请求打开礼包 
	void requestOpenGift(uint16_t cell, uint16_t num, RoleId roleId);

public:
	bool getFixReward(uint32_t rewardId, uint16_t num, uint32_t level, Job job, std::vector<ObjItem>& objVec) const;

	bool getRandomReward(uint32_t rewardId, uint16_t num, uint32_t level, Job job, std::vector<ObjItem>& objVec) const;

};


}

#endif
