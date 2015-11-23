#include "object_config.h"

#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/componet/string_kit.h"

namespace world{

ObjectConfig ObjectConfig::m_me;

ObjectConfig& ObjectConfig::me()
{
	return m_me;
}

void ObjectConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/object.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	objectCfg.load(root);
}

void ObjectConfig::Object::load(componet::XmlParseNode root)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	if(!root)
		return;

	auto strToProbList = [](std::vector<StrongPropItem>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() < 3)
				continue;
			
			StrongPropItem temp;
			temp.level = propItems[0];
			temp.propType = static_cast<PropertyType>(propItems[1]);
			temp.prop = propItems[2];

			ret->push_back(temp);
		}
	};

	auto parseFenjieList = [](std::vector<FenjieRewardItem>* ret, const std::string& str)
	{
		ret->clear();
		std::vector<std::string> strItems = componet::splitString(str, ",");
		
		for(const std::string& item : strItems)
		{
			std::vector<uint32_t> propItems;
			componet::fromString(&propItems, item, "-");
			if(propItems.size() < 4)
				continue;
			
			FenjieRewardItem temp;
			temp.tplId = propItems[0];
			temp.num = propItems[1];
			temp.bind = static_cast<Bind>(propItems[2]);
			temp.prob = propItems[3];

			ret->push_back(temp);
		}
	};

	for(XmlParseNode node = root.getChild("item"); node; ++node)
	{
		ObjBasicData temp;
		temp.tplId = node.getChildNodeText<uint32_t>("tplId");
		temp.name = node.getChildNodeText<std::string>("name");
		temp.childType = node.getChildNodeText<uint16_t>("childType");
		temp.quality = node.getChildNodeText<uint8_t>("quality");
		temp.turnLife = node.getChildNodeText<uint8_t>("turnLife");
		temp.level = node.getChildNodeText<uint8_t>("level");
		temp.job = node.getChildNodeText<uint8_t>("job");
		temp.sex = node.getChildNodeText<uint8_t>("sex");
		temp.needMoneyType = node.getChildNodeText<uint8_t>("needMoneyType");
		temp.needMoneyNum = node.getChildNodeText<uint32_t>("needMoneyNum");
		temp.maxStackNum = node.getChildNodeText<uint16_t>("maxStackNum");
		temp.moneyType = node.getChildNodeText<uint64_t>("moneyType");
		temp.price = node.getChildNodeText<uint32_t>("price");
		temp.lifeType = node.getChildNodeText<uint8_t>("lifeType");
		temp.lifeSpan = node.getChildNodeText<uint32_t>("lifeSpan");
		temp.bBatUse = node.getChildNodeText<bool>("bBatUse");
		temp.bDiscard = node.getChildNodeText<bool>("bDiscard");
		temp.prob = node.getChildNodeText<uint32_t>("prob");
		temp.objLevel = node.getChildNodeText<uint32_t>("objLevel");

		temp.p_attackMin = node.getChildNodeText<uint32_t>("p_attackMin");
		temp.p_attackMax = node.getChildNodeText<uint32_t>("p_attackMax");
		temp.m_attackMin = node.getChildNodeText<uint32_t>("m_attackMin");
		temp.m_attackMax = node.getChildNodeText<uint32_t>("m_attackMax");
		temp.witchMin = node.getChildNodeText<uint32_t>("witchMin");
		temp.witchMax = node.getChildNodeText<uint32_t>("witchMax");
		temp.p_defenceMin = node.getChildNodeText<uint32_t>("p_defenceMin");
		temp.p_defenceMax = node.getChildNodeText<uint32_t>("p_defenceMax");
		temp.m_defenceMin = node.getChildNodeText<uint32_t>("m_defenceMin");
		temp.m_defenceMax = node.getChildNodeText<uint32_t>("m_defenceMax");

		temp.hp = node.getChildNodeText<uint32_t>("hp");
		temp.mp = node.getChildNodeText<uint32_t>("mp");
		temp.shot = node.getChildNodeText<uint32_t>("shot");
		temp.p_escape = node.getChildNodeText<uint32_t>("p_escape");
		temp.m_escape = node.getChildNodeText<uint32_t>("m_escape");
		temp.crit = node.getChildNodeText<uint32_t>("crit");
		temp.antiCrit = node.getChildNodeText<uint32_t>("antiCrit");
		temp.lucky = node.getChildNodeText<uint32_t>("lucky");
		temp.evil = node.getChildNodeText<uint32_t>("evil");
		temp.critDamage = node.getChildNodeText<uint32_t>("critDamage");
		temp.shotRatio = node.getChildNodeText<uint32_t>("shotRatio");
		temp.escapeRatio = node.getChildNodeText<uint32_t>("escapeRatio");
		temp.critRatio = node.getChildNodeText<uint32_t>("critRatio");

		temp.hpLv = node.getChildNodeText<uint32_t>("hpLv");
		temp.mpLv = node.getChildNodeText<uint32_t>("mpLv");
		temp.damageAdd = node.getChildNodeText<uint32_t>("damageAdd");
		temp.damageReduce = node.getChildNodeText<uint32_t>("damageReduce");
		temp.damageAddLv = node.getChildNodeText<uint32_t>("damageAddLv");
		temp.damageReduceLv = node.getChildNodeText<uint32_t>("damageReduceLv");
		temp.antiDropEquip = node.getChildNodeText<uint32_t>("antiDropEquip");
		temp.suitId = node.getChildNodeText<uint32_t>("suitId");
		temp.nonsuchId = node.getChildNodeText<uint32_t>("nonsuchId");
		strToProbList(&temp.strongPropVec, node.getChildNodeText<std::string>("strongProp"));
		parseFenjieList(&temp.fenjieRewardVec, node.getChildNodeText<std::string>("fenjie_reward_list"));
        temp.broadCast= (BroadCast)node.getChildNodeText<uint8_t>("broadcast");
		temp.sep1 = node.getChildNodeText<uint32_t>("spe1");
		temp.sep2 = node.getChildNodeText<uint32_t>("spe2");
		temp.sep3 = node.getChildNodeText<uint32_t>("spe3");
		
		m_objBasicDataMap.insert(std::make_pair(temp.tplId, temp));
	}
	
	return;
}

std::string ObjectConfig::getName(uint32_t tplId) const
{
	auto pos = objectCfg.m_objBasicDataMap.find(tplId);
	if(pos == objectCfg.m_objBasicDataMap.end())
		return "";

	return pos->second.name;
}

bool ObjectConfig::getObjBasicData(uint32_t tplId, ObjBasicData* data) const
{
    auto pos = objectCfg.m_objBasicDataMap.find(tplId);
    if(pos == objectCfg.m_objBasicDataMap.end())
        return false;

    *data = pos->second;
    return true;
}

uint16_t ObjectConfig::getMaxStackNum(uint32_t tplId) const
{
    auto pos = objectCfg.m_objBasicDataMap.find(tplId);
    if(pos == objectCfg.m_objBasicDataMap.end())
        return 0;

    return pos->second.maxStackNum;
}

}
