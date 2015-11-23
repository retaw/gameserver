#include "package_config.h"

#include "water/componet/logger.h"

namespace world{

PackageConfig PackageConfig::m_me;

PackageConfig& PackageConfig::me()
{
	return m_me;
}

void PackageConfig::loadConfig(const std::string& configDir)
{
	using namespace water;
	using componet::XmlParseDoc;
	using componet::XmlParseNode;

	const std::string cfgFile = configDir + "/package.xml";
	LOG_TRACE("读取配置文件 {}", cfgFile);

	XmlParseDoc doc(cfgFile);
	XmlParseNode root = doc.getRoot();
	if(!root)
	{
		EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed"); 
	}
	
	packageCfg.load(root);
}

void PackageConfig::Package::load(componet::XmlParseNode root)
{
	if(!root)
		return;

	clear();

	XmlParseNode nodeRole = root.getChild("role");
	sec = nodeRole.getAttr<uint32_t>("sec");
	yuanbao = nodeRole.getAttr<uint32_t>("yuanbao");

	for(XmlParseNode node = nodeRole.getChild("item"); node; ++node)
	{
		Role temp;
		temp.cell = node.getAttr<uint16_t>("cell");
		temp.needOnlineSec = node.getAttr<uint32_t>("needOnlineSec");
		temp.addExp = node.getAttr<uint32_t>("addExp");
		
		m_roleMap.insert(std::make_pair(temp.cell, temp));
	}

	XmlParseNode nodeStorage = root.getChild("storage");
	needTplId = nodeStorage.getAttr<uint32_t>("needTplId");
	for(XmlParseNode node = nodeStorage.getChild("item"); node; ++node)
	{
		Storage temp;
		temp.cell = node.getAttr<uint16_t>("cell");
		temp.needNum = node.getAttr<uint16_t>("needNum");
		temp.addExp = node.getAttr<uint32_t>("addExp");

		m_storageMap.insert(std::make_pair(temp.cell, temp));
	}

	return;
}

void PackageConfig::Package::clear()
{
	m_roleMap.clear();
	m_storageMap.clear();

	return;
}

}
