#include "trigger_cfg.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"

namespace world{

TriggerCfg::TriggerCfg()
{
}

TriggerCfg& TriggerCfg::me()
{
    static TriggerCfg me;
    return me;
}

void TriggerCfg::loadConfig(const std::string& cfgDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgDir + "/trigger.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        return;

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        TriggerTpl::Ptr triggerTpl = TriggerTpl::create();
        triggerTpl->id = itemNode.getChildNodeText<uint32_t>("id");
        triggerTpl->type = static_cast<TriggerType>(itemNode.getChildNodeText<uint8_t>("type"));
        triggerTpl->lifetime = itemNode.getChildNodeText<uint16_t>("lifetime");
        triggerTpl->param1 = itemNode.getChildNodeText<uint16_t>("param1");
        triggerTpl->param2 = itemNode.getChildNodeText<uint16_t>("param2");
        triggerTpl->param3 = itemNode.getChildNodeText<uint16_t>("param3");

        m_triggerTpls.insert({triggerTpl->id, triggerTpl});
    }
}

TriggerTpl::Ptr TriggerCfg::getById(uint32_t id) const
{
    auto iter = m_triggerTpls.find(id);
    if(iter == m_triggerTpls.end())
        return nullptr;

    return iter->second;
}

}
