/*
 * Author: zhupengfei
 *
 * Created: 2015-07-24 15:00 +0800
 *
 * Modified: 2015-07-24 15:00 +0800
 *
 * Description: 称号
 */

#ifndef PROCESS_WORLD_TITLE_HPP
#define PROCESS_WORLD_TITLE_HPP

#include "attribute.h"

#include "water/common/roledef.h"
#include "water/componet/datetime.h"
#include <map>

namespace world{

using namespace water;
using water::componet::TimePoint;

class Role;

/*********************************************/
class Title : public Attribute 
{
public:
	Title(Role& owner);
	~Title() = default;

public:
	void loadFromDB(const std::vector<TitleInfo>& titleVec);	
	void addTitle(uint32_t titleId);

	void sendTitleList();
	void requestUseNormalTitle(uint32_t titleId);
	void requestTakeOffNormalTitle(uint32_t titleId);

private:
	void sendGotNormalTitle(uint32_t titleId);
	bool isTitleIdOverdue(uint32_t titleId);

	void autoUseTitle(TitleType titleType);
	void calcAttribute();

	//获取最高优先佩戴权限的特殊称号ID
	uint32_t getSpecialTitleIdOfMaxPriority();
	uint32_t getTitleIdOfGuanzhiLevel();

	void updateTitleToDB(std::vector<TitleInfo> titleVec);

public:
	uint32_t getUsedTitleIdByType(TitleType type) const;
	void checkTitleOverdue(const TimePoint& now);

private:
	Role& m_owner;
	
private:
	std::map<uint32_t, TitleInfo> m_ownTitleMap;

};


}

#endif
