#include "title.h"
#include "title_config.h"
#include "guanzhi_config.h"
#include "role.h"

#include "water/common/commdef.h"

#include "protocol/rawmsg/public/title.h"
#include "protocol/rawmsg/public/title.codedef.public.h"

#include "protocol/rawmsg/private/title.h"
#include "protocol/rawmsg/private/title.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

Title::Title(Role& owner)
: m_owner(owner)
{
}

void Title::loadFromDB(const std::vector<TitleInfo>& titleVec)
{
	m_ownTitleMap.clear();
	for(auto iter = titleVec.begin(); iter != titleVec.end(); ++iter)
	{
		m_ownTitleMap.insert(std::make_pair(iter->titleId, *iter));
	}
	
	calcAttribute();
	return;
}

void Title::addTitle(uint32_t titleId)
{
	const auto& cfg = TitleConfig::me().titleCfg;
	auto pos = cfg.m_titleMap.find(titleId);
	if(pos == cfg.m_titleMap.end())
		return;

	uint32_t now = toUnixTime(Clock::now());
	uint32_t disableTime = 0;
	if(0 != pos->second.lastSec)
	{
		disableTime = now + pos->second.lastSec;
	}

	if(m_ownTitleMap.find(titleId) != m_ownTitleMap.end())
		return;

	std::vector<TitleInfo> modifyVec;
	TitleInfo temp;
	temp.titleId = titleId;
	temp.titleType = static_cast<TitleType>(pos->second.titleType);
	temp.createTime = now;
	temp.disableTime = disableTime;
	temp.used = false;

	m_ownTitleMap.insert(std::make_pair(temp.titleId, temp));
	modifyVec.push_back(temp);
	updateTitleToDB(modifyVec);

	if(temp.titleType == TitleType::special)
	{
		autoUseTitle(TitleType::special);
	}
	else if(temp.titleType == TitleType::guanzhi)
	{
		autoUseTitle(TitleType::guanzhi);
	}
	else if(temp.titleType == TitleType::normal)
	{
		sendGotNormalTitle(titleId);
	}
	
	sendTitleList();
	return;
}

void Title::sendTitleList()
{
	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PublicRaw::RetTitleList));

	uint32_t now = toUnixTime(Clock::now());
	auto* msg = reinterpret_cast<PublicRaw::RetTitleList*>(buf.data());
	msg->size = 0;

	for(auto pos = m_ownTitleMap.begin(); pos != m_ownTitleMap.end(); ++pos)
	{
		buf.resize(buf.size() + sizeof(PublicRaw::RetTitleList::TitleItem));
		auto* msg  = reinterpret_cast<PublicRaw::RetTitleList*>(buf.data());

		uint32_t sec = 0;
		if(0 == pos->second.disableTime)
			sec = (uint32_t)-1;
		else
			sec = SAFE_SUB(pos->second.disableTime, now);

		msg->data[msg->size].titleId = pos->second.titleId;
		msg->data[msg->size].sec = sec;

		msg->size++;
	}
	
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetTitleList), buf.data(), buf.size());		
	return;
}

void Title::sendGotNormalTitle(uint32_t titleId)
{
	const auto& cfg = TitleConfig::me().titleCfg;
	auto pos = cfg.m_titleMap.find(titleId);
	if(pos == cfg.m_titleMap.end())
		return;

	if(static_cast<TitleType>(pos->second.titleType) != TitleType::normal)
		return;

	PublicRaw::RetGotNormalTitle send;
	send.titleId = titleId;
	m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetGotNormalTitle), &send, sizeof(send));		
	return;
}

void Title::requestUseNormalTitle(uint32_t titleId)
{
	auto pos = m_ownTitleMap.find(titleId);
	if(pos == m_ownTitleMap.end())
		return;

	const uint32_t usedTitleId = getUsedTitleIdByType(TitleType::normal);
	if(usedTitleId == titleId)
		return;

	if(isTitleIdOverdue(titleId))
	{
		m_owner.sendSysChat("称号已过期, 不可佩戴");
		return;
	}

	std::vector<TitleInfo> modifyVec;
	
	if(0 == usedTitleId)
	{
		pos->second.used = true;
		modifyVec.push_back(pos->second);
	}
	else if(usedTitleId != titleId)
	{
		pos->second.used = true;
		m_ownTitleMap[usedTitleId].used = false;
	
		modifyVec.push_back(pos->second);
		modifyVec.push_back(m_ownTitleMap[usedTitleId]);
	}

	calcAttribute();
	updateTitleToDB(modifyVec);
	m_owner.syncScreenDataTo9();
	m_owner.sendMainToMe();
	m_owner.sendSysChat("佩戴成功");

	return;
}

void Title::requestTakeOffNormalTitle(uint32_t titleId)
{
	auto pos = m_ownTitleMap.find(titleId);
	if(pos == m_ownTitleMap.end())
		return;

	if(!pos->second.used)
		return;

	std::vector<TitleInfo> modifyVec;
	
	pos->second.used = false;
	modifyVec.push_back(pos->second);

	calcAttribute();
	updateTitleToDB(modifyVec);
	m_owner.syncScreenDataTo9();
	m_owner.sendMainToMe();
	m_owner.sendSysChat("取下成功");
	return;
}

bool Title::isTitleIdOverdue(uint32_t titleId)
{
	auto pos = m_ownTitleMap.find(titleId);
	if(pos == m_ownTitleMap.end())
		return true;

	if(0 == pos->second.disableTime)
		return false;
	
	uint32_t now = toUnixTime(Clock::now());
	if(now >= pos->second.disableTime)
		return true;

	return false;
}

void Title::autoUseTitle(TitleType titleType)
{
	uint32_t titleId = 0;
	if(titleType == TitleType::special)
		titleId = getSpecialTitleIdOfMaxPriority();
	else if(titleType == TitleType::guanzhi)
		titleId = getTitleIdOfGuanzhiLevel();
	if(0 == titleId)
		return;

	std::vector<TitleInfo> modifyVec;
	for(auto pos = m_ownTitleMap.begin(); pos != m_ownTitleMap.end(); ++pos)
	{
		if(pos->second.titleType != titleType)
			continue;

		if(isTitleIdOverdue(pos->second.titleId))
			continue;

		if(pos->second.titleId != titleId)
		{
			pos->second.used = false;
			modifyVec.push_back(pos->second);
		}
		else
		{
			pos->second.used = true;
			modifyVec.push_back(pos->second);
		}
	}

	if(modifyVec.empty())
		return;

	calcAttribute();
	updateTitleToDB(modifyVec);
	m_owner.syncScreenDataTo9();
	m_owner.sendMainToMe();
	return;
}


void Title::calcAttribute()
{
	Attribute::reset();

	for(auto pos = m_ownTitleMap.begin(); pos != m_ownTitleMap.end(); ++pos)
	{
		if(!pos->second.used)
			continue;

		if(isTitleIdOverdue(pos->second.titleId))
			continue;

		const auto& cfg = TitleConfig::me().titleCfg;
		auto rewardPropVec = cfg.getRewardPropVec(pos->second.titleId);

		if(rewardPropVec.empty())
			continue;

		Attribute::addAttribute(rewardPropVec);
	}

	return;
}

//获取最高优先佩戴权限的特殊称号ID
uint32_t Title::getSpecialTitleIdOfMaxPriority()
{
	uint32_t maxPriority = 0;
	uint32_t titleId = 0;
	for(auto pos = m_ownTitleMap.begin(); pos != m_ownTitleMap.end(); ++pos)
	{
		if(pos->second.titleType != TitleType::special)
			continue;

		if(isTitleIdOverdue(pos->second.titleId))
			continue;

		const auto& cfg = TitleConfig::me().titleCfg;
		uint32_t priority = cfg.getPriority(pos->second.titleId);

		if(priority >= maxPriority)
		{
			maxPriority = priority;
			titleId = pos->second.titleId;
		}
	}

	return titleId;
}

uint32_t Title::getTitleIdOfGuanzhiLevel()
{
	const auto& cfg = GuanzhiConfig::me().guanzhiCfg;
	auto pos = cfg.m_guanzhiMap.find(m_owner.m_guanzhi.level());
	if(pos == cfg.m_guanzhiMap.end())
		return 0;

	return pos->second.titleId;
}

uint32_t Title::getUsedTitleIdByType(TitleType type) const
{
	for(auto pos = m_ownTitleMap.begin(); pos != m_ownTitleMap.end(); ++pos)
	{
		if(pos->second.titleType != type)
			continue;

		if(!pos->second.used)
			continue;

		return pos->second.titleId;
	}

	return 0;
}

void Title::checkTitleOverdue(const TimePoint& now)
{
	bool flag = false;
	std::vector<TitleInfo> modifyVec;

	for(auto pos = m_ownTitleMap.begin(); pos != m_ownTitleMap.end(); ++pos)
	{
		if(!pos->second.used)
			continue;

		if(!isTitleIdOverdue(pos->second.titleId))
			continue;
				
		pos->second.used = false;
		modifyVec.push_back(pos->second);
		flag = true;
	
		if(pos->second.titleType == TitleType::special)
		{
			autoUseTitle(TitleType::special);
		}
	}

	if(!flag)
		return;

	calcAttribute();
	updateTitleToDB(modifyVec);
	m_owner.sendMainToMe();
	return;
}

void Title::updateTitleToDB(std::vector<TitleInfo> titleVec)
{
	if(titleVec.empty())
		return;

	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PrivateRaw::UpdateOrInsertTitle));

	auto* msg = reinterpret_cast<PrivateRaw::UpdateOrInsertTitle*>(buf.data());
	msg->roleId = m_owner.id();
	msg->size = 0;

	for(auto iter = titleVec.begin(); iter != titleVec.end(); ++iter)
	{
		buf.resize(buf.size() + sizeof(PrivateRaw::UpdateOrInsertTitle::data[0]));
		auto* msg  = reinterpret_cast<PrivateRaw::UpdateOrInsertTitle*>(buf.data());

		msg->data[msg->size].titleId = iter->titleId;
		msg->data[msg->size].titleType = iter->titleType;
		msg->data[msg->size].createTime = iter->createTime;
		msg->data[msg->size].disableTime = iter->disableTime;
		msg->data[msg->size].used = iter->used;

		msg->size++;
	}
	
	ProcessIdentity dbcachedId("dbcached", 1);
	World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateOrInsertTitle), buf.data(), buf.size());
	
	return;
}



}
