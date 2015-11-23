#include "reward_manager.h"
#include "role_manager.h"
#include "hero.h"
#include "reward_fixed_config.h"
#include "reward_random_config.h"
#include "object.h"
#include "object_config.h"
#include "mail_manager.h"

#include "water/common/objdef.h"
#include "water/componet/logger.h"
#include "water/componet/random.h"

namespace world{

using namespace std::placeholders;

RewardManager RewardManager::m_me;

RewardManager& RewardManager::me()
{
	return m_me;
}

//C->S 请求打开礼包
void RewardManager::requestOpenGift(uint16_t cell, uint16_t num, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	const uint16_t objNum = role->getObjNumByCell(cell);
	if(num > objNum)
		return;

	Object::Ptr obj = role->getObjByCell(cell);
	if(obj == nullptr)
		return;

	const ObjChildType childType = obj->childType();
	if(childType != ObjChildType::use_gift_fixed && childType != ObjChildType::use_gift_random)
		return;

	const MoneyType needMoneyType = obj->needMoneyType();
	const uint32_t needMoneyNum = obj->needMoneyNum();
	if(needMoneyType != MoneyType::none && 0 != needMoneyNum)
	{
		if(!role->checkMoney(needMoneyType, needMoneyNum))
			return;
	}

	const uint32_t rewardId = obj->spe1();
	if(0 == rewardId)
		return;

	std::vector<ObjItem> objVec;
	if(childType == ObjChildType::use_gift_fixed)	//固定礼包
	{
		if(!getFixReward(rewardId, num, role->level(), role->job(), objVec))
			return;
	}
	else if(childType == ObjChildType::use_gift_random)	//随机礼包
	{
		if(!getRandomReward(rewardId, num, role->level(), role->job(), objVec))
			return;
	}
	
	if(!role->checkPutObj(objVec))
	{
		role->sendSysChat("背包空间不足");
		return;
	}

	if(needMoneyType != MoneyType::none && 0 != needMoneyNum)
	{
		if(!role->reduceMoney(needMoneyType, needMoneyNum, "打开礼包,tplId={}", obj->tplId()))
		{
			LOG_ERROR("礼包, 奖励, 打开礼包, 扣除货币失败! name={}, id={}, tplId={}, rewardId={}",
					  role->name(), role->id(), obj->tplId(), rewardId);
			return;
		}
	}
	role->eraseObjByCell(cell, num, PackageType::role, "打开礼包");
	role->putObj(objVec);
	return;
}


bool RewardManager::getFixReward(uint32_t rewardId, uint16_t num, uint32_t level, Job job, std::vector<ObjItem>& objVec) const
{
	objVec.clear();
	const auto& cfg = RewardFixedConfig::me().m_fixRewardCfg;
	auto pos = cfg.rewardMap.find(rewardId);
	if(pos == cfg.rewardMap.end())
		return false;

	if(pos->second.empty())
		return false;

	for(auto iter = pos->second.begin(); iter != pos->second.end(); ++iter)
	{
		Job needJob = static_cast<Job>(iter->job);
		if(needJob != Job::none && needJob != job)
			continue;

		if(level < iter->minLevel || level > iter->maxLevel)
			continue;

		for(uint16_t i = 0; i < num; ++i)
		{
			ObjItem temp;
			temp.tplId = iter->tplId;
			temp.num = iter->num;
			temp.bind = iter->bind;
			objVec.push_back(temp);
		}
	}

	return true;
}

bool RewardManager::getRandomReward(uint32_t rewardId, uint16_t num, uint32_t level, Job job, std::vector<ObjItem>& objVec) const
{
	const auto& cfg = RewardRandomConfig::me().m_rewardRandomCfg;
	auto pos = cfg.rewardMap.find(rewardId);
	if(pos == cfg.rewardMap.end())
		return false;

	//从奖励配置中随机奖励池ID
	std::vector<uint32_t> poolVec;
	for(uint16_t i = 0; i < num; ++i)
	{
		for(uint32_t i = 0; i < pos->second.size(); ++i)
		{
			const std::vector<uint32_t>& poolSet = pos->second[i];
			if(poolSet.empty())
				continue;

			componet::Random<uint32_t> rand(0, poolSet.size() -1);
			const uint32_t index = rand.get();
			if(index >= poolSet.size())
				continue;

			if(0 == poolSet[index])
				continue;

			poolVec.push_back(poolSet[index]);
		}
	}
	
	if(poolVec.empty())
		return true;

	//奖励池中随机具体的奖励
	for(auto iter = poolVec.begin(); iter != poolVec.end(); ++iter)
	{
		auto posPool = cfg.poolMap.find(*iter);
		if(posPool == cfg.poolMap.end())
		{
			LOG_ERROR("奖励, 随机奖励, 奖励池中未找到poolId, 配置错误! rewardId={}, poolId={}",
					  rewardId, *iter);
			return false;
		}
	
		std::vector<RewardData> tempVec;
		for(auto item = posPool->second.begin(); item != posPool->second.end(); ++item)
		{
			Job needJob = static_cast<Job>(item->job);
			if(needJob != Job::none && needJob != job)
				continue;

			if(level < item->minLevel || level > item->maxLevel)
				continue;
			
			tempVec.push_back(*item);
		}

		if(tempVec.empty())
			continue;

		componet::Random<uint32_t> rand(0, tempVec.size() - 1);
		uint32_t index = rand.get();
		if(index >= tempVec.size())
			continue;
		
		auto item = tempVec[index];
		ObjItem temp;
		temp.tplId = item.tplId;
		temp.num = item.num;
		temp.bind = item.bind;
		objVec.push_back(temp);
	}

	return true;
}


}
