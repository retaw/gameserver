/*
 * Author: zhupengfei
 *
 * Created: 2015-09-08 14:15 +0800
 *
 * Modified: 2015-09-08 14:15 +0800
 *
 * Description: 装备分解
 */

#ifndef PROCESS_WORLD_FEN_JIE_HPP
#define PROCESS_WORLD_FEN_JIE_HPP

#include "fenjie_config.h"

#include "water/common/roledef.h"
#include "water/common/objdef.h"

namespace world{

class Role;

class Fenjie
{
public:
	explicit Fenjie(Role& owner);
	~Fenjie() = default;

public:
	void requestFenjie(const std::vector<uint16_t>& cellVec);


private:
	bool checkObj(const std::vector<uint16_t>& cellVec);
	bool eraseObj(const std::vector<uint16_t>& cellVec);

	std::vector<FenjieConfig::Fenjie::RewardItem> getFenjieObjReward(const std::vector<uint16_t>& cellVec);

	void sendFenjieReward(const std::vector<FenjieConfig::Fenjie::RewardItem>& rewardVec);

private:
	Role& m_owner;
};


}

#endif
