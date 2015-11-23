#include "dragon_heart_cfg.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

DragonHeartBase::DragonHeartBase()
{
}

DragonHeartBase& DragonHeartBase::me()
{
    static DragonHeartBase me;
    return me;
}

void DragonHeartBase::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/dragonHeart.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        DragonHeartCfg::Ptr dragonCfg = DragonHeartCfg::create();
        dragonCfg->dragonSkillId = itemNode.getChildNodeText<uint32_t>("id");
        dragonCfg->dragonSkillLevel = itemNode.getChildNodeText<uint32_t>("level");
        dragonCfg->costDragonSoul = itemNode.getChildNodeText<uint32_t>("dragonSoul");
        dragonCfg->roleLevel = itemNode.getChildNodeText<uint32_t>("roleLevel");

        std::string str;
        std::vector<std::string> subvs;
        str = itemNode.getChildNodeText<std::string>("props");
        std::vector<std::string> vs = componet::splitString(str, ",");
        for(const auto& iter : vs)
        {
            subvs.clear();
            subvs = componet::splitString(iter, "-");
            if(subvs.size() < 2)
                continue;

            std::pair<DragonSkillProp, uint16_t> prop;
            prop.first = static_cast<DragonSkillProp>(atoi(subvs[0].c_str()));
            prop.second = atoi(subvs[1].c_str());

            dragonCfg->dragonSkillProps.push_back(prop);
        }

        uint32_t id = 1000 * dragonCfg->dragonSkillId + dragonCfg->dragonSkillLevel;
        m_dragonHeartCfg.insert({id, dragonCfg});
    }
}

DragonHeartCfg::Ptr DragonHeartBase::getCfg(uint32_t dragonSkillId, uint32_t dragonSkillLevel) const
{
    uint32_t id = 1000 * dragonSkillId + dragonSkillLevel;
    auto it = m_dragonHeartCfg.find(id);
    if(it == m_dragonHeartCfg.end())
        return nullptr;
    return it->second;
}

}
