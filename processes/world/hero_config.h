/*
 * Author: zhupengfei
 *
 * Created: 2015-06-16 13:34 +0800
 *
 * Modified: 2015-06-16 13:34 +0800
 *
 * Description: 加载英雄配置文件
 */

#ifndef PROCESS_WORLD_HERO_CONFIG_HPP
#define PROCESS_WORLD_HERO_CONFIG_HPP

#include "water/componet/xmlparse.h"
#include "water/common/roledef.h"

#include <vector>
#include <map>

class XmlParseNode;

namespace world{

using namespace water;
using water::componet::XmlParseNode;

class HeroConfig
{
public:
	~HeroConfig() = default;
	static HeroConfig& me();
private:
	static HeroConfig m_me;

public:
	void loadConfig(const std::string& configDir);

public:
	struct Hero 
	{
		void load(XmlParseNode root);
		void clear();

		uint32_t m_needSpanSec;
		uint32_t m_needLevel;

		struct HeroItem
		{
			uint8_t job;
			std::string name;
		};
	
		struct ConsumeItem
		{
			uint8_t createNum;
			uint32_t needLevel;
			uint32_t needTplId;
			uint16_t needNum;
		};

		std::map<uint8_t, HeroItem> m_heroMap;			//<job, HeroItem>
		std::map<uint8_t, ConsumeItem> m_consumeMap;	//<createNum, ConsumeItem>

        struct AISetting
        {
            Job job;
            std::vector<TplId> skillPriority;
            TplId defaultSkill;
            TplId singleTargetAttSkill;
            TplId multiTargetAttSkill;
            std::map<Job, TplId> jointSkills;   //<ownerJob, tplId>
        };
        std::map<Job, AISetting> m_ai;
	} heroCfg;

public:
    const Hero::AISetting* getAICfg(Job heroJob) const;
    TplId getJointSkillId(Job roleJob, Job heroJob) const;
};


}

#endif
