/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-18 16:51 +0800
 *
 * Modified: 2015-05-18 16:51 +0800
 *
 * Description: ai管理器
 */


#ifndef PROCESS_WORLD_AI_MANAGER_H
#define PROCESS_WORLD_AI_MANAGER_H

#include "ai.h"

#include <map>
#include <list>


namespace world{
namespace ai{


class AIManager
{
public:
	~AIManager();

	bool loadConfig(const std::string& cfgDir);
    AI::Ptr create(uint32_t tplId);
//	uint64_t activeDelay(int32_t id);

private:
	AIManager();
	void clear();

private:
	std::map<uint32_t, AI::Ptr> m_ais;


public:
	static AIManager& me();
};


}}

#endif
