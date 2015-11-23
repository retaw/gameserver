#include "massive_config.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

Massive::Massive()
{
}

Massive& Massive::me()
{
    static Massive me;
    return me;
}

void Massive::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/massive_config.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    if(XmlParseNode evilNode = root.getChild("Evil"))
    {
        m_evilCfg.killAddEvil = evilNode.getAttr<uint16_t>("kill_add_evil");
        m_evilCfg.yellowEvil = evilNode.getAttr<uint16_t>("yellow");
        m_evilCfg.redEvil = evilNode.getAttr<uint16_t>("red");
        m_evilCfg.subEvil = evilNode.getAttr<uint16_t>("sub_evil");
        m_evilCfg.greynameTime = evilNode.getAttr<uint16_t>("greyname_time");
    }

    if(XmlParseNode dropNode = root.getChild("DropEquip"))
    {
        m_dropCfg.normalPackage = dropNode.getAttr<uint16_t>("normal_package");
        m_dropCfg.equipPackage = dropNode.getAttr<uint16_t>("equip_package");
        m_dropCfg.evilParam1 = dropNode.getAttr<uint16_t>("evil_param1");
        m_dropCfg.evilParam2 = dropNode.getAttr<uint16_t>("evil_param2");
        m_dropCfg.constant = dropNode.getAttr<uint16_t>("constant");
        m_dropCfg.ownerTime = dropNode.getAttr<uint16_t>("owner_time");

        LOG_DEBUG("DropEquip配置, normalPackage={},equipPackage={},evilParam1={},evilParam2={},constant={},ownerTime={}",
                  m_dropCfg.normalPackage, m_dropCfg.equipPackage, m_dropCfg.evilParam1, m_dropCfg.evilParam2, m_dropCfg.constant, m_dropCfg.ownerTime);
    }

    XmlParseNode skillNode = root.getChild("Skill");
    if(skillNode)
        m_publicSkillCD = skillNode.getAttr<uint16_t>("public_cd");

    XmlParseNode fireNode = root.getChild("Fire");
    if(fireNode)
    {
        m_fireCfg.lifeTime = fireNode.getAttr<uint16_t>("lifetime");
        m_fireCfg.interval = fireNode.getAttr<uint16_t>("interval");
    }

    XmlParseNode jointSkillNode = root.getChild("JointSkill");
    if(jointSkillNode)
    {
        m_jointSkillCfg.baseAnger = jointSkillNode.getAttr<uint16_t>("baseAnger");
        m_jointSkillCfg.costAnger = jointSkillNode.getAttr<uint16_t>("costAnger");
        m_jointSkillCfg.angerLimit = jointSkillNode.getAttr<uint16_t>("angerLimit");
        m_jointSkillCfg.energeLimit = jointSkillNode.getAttr<uint16_t>("energeLimit");
        m_jointSkillCfg.readyTime = jointSkillNode.getAttr<uint16_t>("prepareTime");
        m_jointSkillCfg.dragonSoul = jointSkillNode.getAttr<uint16_t>("dragonSoul");
        m_jointSkillCfg.costObj = jointSkillNode.getAttr<uint32_t>("costObj");
    }

    XmlParseNode stallNode = root.getChild("Stall");
    if(stallNode)
    {
        m_stallCfg.mapId = stallNode.getAttr<uint32_t>("map");
        m_stallCfg.stallLevel = stallNode.getAttr<uint32_t>("level");
    }
}

}
