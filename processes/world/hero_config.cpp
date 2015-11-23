#include "hero_config.h"

#include "water/componet/logger.h"
#include "water/componet/exception.h"
#include "water/componet/string_kit.h"

namespace world{

HeroConfig HeroConfig::m_me;

HeroConfig& HeroConfig::me()
{
	return m_me;
}

void HeroConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using water::componet::XmlParseDoc;

	const std::string cfgFile = configDir + "/hero.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	heroCfg.load(root);
}

void HeroConfig::Hero::load(XmlParseNode root)
{
	if(!root)
		return;

	clear();

	XmlParseNode nodeHero = root.getChild("hero");
	m_needLevel = nodeHero.getAttr<uint32_t>("needLevel");
	m_needSpanSec = nodeHero.getAttr<uint32_t>("needSpanSec");

	for(XmlParseNode node = nodeHero.getChild("item"); node; ++node)
	{
		HeroItem temp;
		temp.job = node.getAttr<uint8_t>("job");
		temp.name = node.getAttr<std::string>("name");

		m_heroMap.insert(std::make_pair(temp.job, temp));
	}

	for(XmlParseNode node = root.getChild("consume").getChild("item"); node; ++node)
	{
		ConsumeItem temp;
		temp.createNum = node.getAttr<uint8_t>("createNum");
		temp.needLevel = node.getAttr<uint32_t>("needLevel");
		temp.needTplId = node.getAttr<uint32_t>("needTplId");
		temp.needNum = node.getAttr<uint16_t>("needNum");

		m_consumeMap.insert(std::make_pair(temp.createNum, temp));
	}

    auto aiNode = root.getChild("ai");
    for(XmlParseNode node = root.getChild("ai").getChild("item"); node; ++node)
    {
        auto job = static_cast<Job>(node.getAttr<uint8_t>("job"));
        auto& item = m_ai[job];
        item.job = job;
        componet::fromString(&item.skillPriority, node.getAttr<std::string>("skillPriority"), ",");
        item.defaultSkill         = node.getAttr<TplId>("defaultSkill");
        item.singleTargetAttSkill = node.getAttr<TplId>("singleTargetAttSkill");
        item.multiTargetAttSkill  = node.getAttr<TplId>("multiTargetAttSkill");

        for(XmlParseNode n = node.getChild("jointSkill"); n; ++n)
        {
            auto roleJob = static_cast<Job>(n.getAttr<uint8_t>("job"));
            item.jointSkills[roleJob] = n.getAttr<TplId>("skillId");
        }
    }

	return;
}

void HeroConfig::Hero::clear()
{
	m_heroMap.clear();
	m_consumeMap.clear();
}

const HeroConfig::Hero::AISetting* HeroConfig::getAICfg(Job heroJob) const
{
    auto it = heroCfg.m_ai.find(heroJob);
    if(it == heroCfg.m_ai.end())
        return nullptr;
    return &(it->second);
}

TplId HeroConfig::getJointSkillId(Job roleJob, Job heroJob) const
{
    auto cfg = getAICfg(heroJob);
    if(cfg == nullptr)
        return 0;
    auto it = cfg->jointSkills.find(roleJob);
    if(it == cfg->jointSkills.end())
        return 0;

    return it->second;
}


}
