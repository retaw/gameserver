#include "merge_manager.h"
#include "merge_config.h"
#include "role_manager.h"

#include "water/common/commdef.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/public/merge.h"
#include "protocol/rawmsg/public/merge.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

MergeManager MergeManager::m_me;

MergeManager& MergeManager::me()
{
	return m_me;
}


void MergeManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestMergeObj, std::bind(&MergeManager::clientmsg_RequestMergeObj, this, _1, _2, _3));
	
}

//请求合成物品
void MergeManager::clientmsg_RequestMergeObj(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestMergeObj*>(msgData);
	if(!rev)
		return;

	const auto& cfg = MergeConfig::me().mergeCfg;
	auto pos = cfg.m_mergeMap.find(rev->mergeTplId);
	if(pos == cfg.m_mergeMap.end())
		return;

	const uint32_t mergeTplId = rev->mergeTplId;
	const uint8_t num = rev->num;
	const Bind bind = pos->second.bind;
	if(0 == num)
		return;

	if(!role->checkPutObj(mergeTplId, num, bind, PackageType::role))
	{
		role->sendSysChat("背包空间不足");
		return;
	}

	if(!reduceMaterial(roleId, mergeTplId, num))
		return;

	componet::Random<uint32_t> sucess_prob(1, 1000);
	if(pos->second.prob >= sucess_prob.get()) //合成成功
	{
		role->putObj(mergeTplId, num, bind, PackageType::role);
		sendMergeResult(roleId, OperateRetCode::sucessful);
		role->sendSysChat("合成成功");
	}
	else
	{
		sendMergeResult(roleId, OperateRetCode::failed);
		role->sendSysChat("合成失败");
	}
	
	return;
}

bool MergeManager::reduceMaterial(RoleId roleId, uint32_t mergeTplId, uint8_t num)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return false;

	const auto& cfg = MergeConfig::me().mergeCfg;
	auto pos = cfg.m_mergeMap.find(mergeTplId);
	if(pos == cfg.m_mergeMap.end())
		return false;

	//验证货币
	MoneyType needMoneyType = pos->second.needMoneyType;
	const uint32_t needMoneyNum = pos->second.needMoneyNum * num;
	if(!role->checkMoney(needMoneyType, needMoneyNum))
	{
		role->sendSysChat("{}不足", role->getMoneyName(needMoneyType));
		return false;
	}

	//验证合成材料
	const auto& needObjVec = pos->second.needObjVec;
	if(needObjVec.empty())
		return false;

	for(auto iter = needObjVec.begin(); iter != needObjVec.end(); ++iter)
	{
		const uint32_t needObjNum = iter->second * num;
		const uint16_t objNum = role->m_packageSet.getObjNum(iter->first, PackageType::role);
		if(needObjNum > objNum)
		{
			role->sendSysChat("材料不足");
			sendMergeResult(roleId, OperateRetCode::materialNotEnough);
			return false;
		}
	}

	//扣合成材料
	for(auto iter = needObjVec.begin(); iter != needObjVec.end(); ++iter)
	{
		const uint32_t needObjNum = iter->second * num;
		if(!role->m_packageSet.eraseObj(iter->first, needObjNum, PackageType::role, "合成物品"))
			return false;
	}

	//扣钱
	if(!role->reduceMoney(needMoneyType, needMoneyNum, "合成物品"))
		return false; 

	return true;
}


void MergeManager::sendMergeResult(RoleId roleId, OperateRetCode code)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	PublicRaw::RetObjMergeResult send;
	send.code = code;
	role->sendToMe(RAWMSG_CODE_PUBLIC(RetObjMergeResult), &send, sizeof(send));
}


}
