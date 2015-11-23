#include "fenjie.h"
#include "role.h"
#include "role_manager.h"
#include "object.h"
#include "mail_manager.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/fenjie.h"
#include "protocol/rawmsg/public/fenjie.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

Fenjie::Fenjie(Role& owner)
: m_owner(owner)
{
}

void Fenjie::requestFenjie(const std::vector<uint16_t>& cellVec)
{
	if(cellVec.empty())
		return;

	if(!checkObj(cellVec))
		return;

	//必须放在eraseObj之前
	const std::vector<FenjieConfig::Fenjie::RewardItem>& rewardVec = getFenjieObjReward(cellVec);
	if(!eraseObj(cellVec))
		return;

	//返回分解产出结果
	sendFenjieReward(rewardVec);

	std::vector<FenjieConfig::Fenjie::RewardItem> putFailedVec;
	for(auto iter = rewardVec.begin(); iter != rewardVec.end(); ++iter)
	{
		if(!m_owner.checkPutObj(iter->tplId, iter->num, iter->bind))
		{
			putFailedVec.push_back(*iter);
			continue;
		}

		m_owner.putObj(iter->tplId, iter->num, iter->bind);
	}

	//发邮件
	if(!putFailedVec.empty())
	{
		std::vector<ObjItem> objVec;
		for(auto iter = putFailedVec.begin(); iter != putFailedVec.end(); ++ iter)
		{
			ObjItem temp;
			temp.tplId = iter->tplId;
			temp.num = iter->num;
			temp.bind = iter->bind;
			objVec.push_back(temp);
		}

		std::string text = "您进行了装备分解，由于背包空间不足，剩余材料通过邮件发放";
		MailManager::me().send(m_owner.id(), "装备分解收益", text, objVec);
		m_owner.sendSysChat("背包空间不足, 部分材料通过邮件发放");
	}

	return;
}

bool Fenjie::checkObj(const std::vector<uint16_t>& cellVec)
{
	if(cellVec.empty())
		return false;

	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		Object::Ptr obj = m_owner.getObjByCell(*iter);
		if(obj == nullptr)
			return false;

		if(obj->fenjieRewardVec().empty())
		{
			m_owner.sendSysChat("{}不可分解", obj->name());
			return false;
		}
	}

	return true;
}

bool Fenjie::eraseObj(const std::vector<uint16_t>& cellVec)
{
	if(cellVec.empty())
		return false;

	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		if(nullptr == m_owner.eraseObjByCell(*iter, PackageType::role, "装备分解"))
		{
			LOG_ERROR("装备分解, 从背包中删除失败! name={}, roleId={}, cell={}, cellVec={}",
					  m_owner.name(), m_owner.id(), *iter, cellVec.size());
			return false;
		}
	}

	return true;
}

std::vector<FenjieConfig::Fenjie::RewardItem> Fenjie::getFenjieObjReward(const std::vector<uint16_t>& cellVec)
{
	std::vector<FenjieConfig::Fenjie::RewardItem> rewardVec;
	rewardVec.clear();

	const auto& cfg = FenjieConfig::me().fenjieCfg;
	for(auto iter = cellVec.begin(); iter != cellVec.end(); ++iter)
	{
		Object::Ptr obj = m_owner.getObjByCell(*iter);
		if(obj == nullptr)
			continue;

		const auto& rewardPoolVec = obj->fenjieRewardVec();
		if(rewardPoolVec.empty())
			continue;

		for(auto it = rewardPoolVec.begin(); it != rewardPoolVec.end(); ++it)
		{
			componet::Random<uint32_t> rand(1, 100);
			if(it->prob < rand.get())
				continue;

			FenjieConfig::Fenjie::RewardItem temp;
			temp.tplId = it->tplId;
			temp.num = it->num;
			temp.bind = it->bind;

			rewardVec.push_back(temp);
		}
	
		//强化，额外产出
		if(0 != obj->strongLevel())
		{
			auto pos = cfg.m_strongRewardMap.find(obj->strongLevel());
			if(pos != cfg.m_strongRewardMap.end())
			{
				for(auto item = pos->second.begin(); item != pos->second.end(); ++item)
				{
					FenjieConfig::Fenjie::RewardItem temp;
					temp.tplId = item->tplId;
					temp.num = item->num;
					temp.bind = item->bind;

					rewardVec.push_back(temp);
				}
			}
		}

		//武器幸运，额外产出
		if(0 != obj->luckyLevel())
		{
			auto pos = cfg.m_luckyRewardMap.find(obj->strongLevel());
			if(pos != cfg.m_luckyRewardMap.end())
			{
				for(auto item = pos->second.begin(); item != pos->second.end(); ++item)
				{
					FenjieConfig::Fenjie::RewardItem temp;
					temp.tplId = item->tplId;
					temp.num = item->num;
					temp.bind = item->bind;

					rewardVec.push_back(temp);
				}
			}
		}
	}

	return rewardVec;
}

void Fenjie::sendFenjieReward(const std::vector<FenjieConfig::Fenjie::RewardItem>& rewardVec)
{
	std::vector<uint8_t> buf;
	buf.reserve(1024);
	buf.resize(sizeof(PublicRaw::RetFenJieReward));

	auto* msg = reinterpret_cast<PublicRaw::RetFenJieReward*>(buf.data());
	msg->size = 0;

	for(auto iter = rewardVec.begin(); iter != rewardVec.end(); ++iter)
	{
		buf.resize(buf.size() + sizeof(msg->data[0]));
		auto* msg = reinterpret_cast<PublicRaw::RetFenJieReward*>(buf.data());

		msg->data[msg->size].tplId = iter->tplId;
		msg->data[msg->size].num = iter->num;
		msg->data[msg->size].bind = iter->bind;

		++msg->size;
	}

	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetFenJieReward), buf.data(), buf.size());		
	return;
}


}
