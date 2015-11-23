#include "pet_base.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

PetBase& PetBase::me()
{
    static PetBase me;
    return me;
}

void PetBase::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    {
        const std::string cfgFile = cfgdir + "/pet.xml";
        LOG_TRACE("读取配置文件 {}", cfgFile);

        XmlParseDoc doc(cfgFile);
        XmlParseNode root = doc.getRoot();
        if(!root)
            EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

        for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
        {
            PetTpl::Ptr petTpl = PetTpl::create();
            petTpl->id = itemNode.getChildNodeText<TplId>("id");
            petTpl->name = itemNode.getChildNodeText<std::string>("name");
            petTpl->job = static_cast<Job>(itemNode.getChildNodeText<uint8_t>("job"));

            std::vector<std::string> subvs;
            std::string str = itemNode.getChildNodeText<std::string>("props");
            std::vector<std::string> vs = componet::splitString(str, ",");
            for(const auto& iter : vs)
            {
                subvs.clear();
                subvs = componet::splitString(iter, "-");
                if(subvs.size() < 2)
                    continue;

                PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
                uint32_t prop_val = atoi(subvs[1].c_str());
                petTpl->props.push_back(std::make_pair(prop_type, prop_val));
            }

            vs.clear();
            str = itemNode.getChildNodeText<std::string>("skillId");
            vs = componet::splitString(str, ",");
            for(const auto& iter : vs)
            {
                subvs.clear();
                subvs = componet::splitString(iter, "-");
                if(subvs.size() < 2)
                    continue;
                uint32_t skillId = atoi(subvs[0].c_str());
                uint32_t skillLv = atoi(subvs[1].c_str());
                petTpl->skillIDs.push_back(std::make_pair(skillId, skillLv));
                //LOG_DEBUG("宠物, 技能加载, id:{}, skillId:{}, skillLv:{}", petTpl->id, skillId, skillLv);
            }

            LOG_DEBUG("宠物配置加载, id={}", petTpl->id);
            m_petTpls.insert({petTpl->id, petTpl});
        }
    }

    {
        const std::string cfgFile = cfgdir + "/pet_level.xml";
        LOG_TRACE("读取配置文件 {}", cfgFile);

        XmlParseDoc doc(cfgFile);
        XmlParseNode root = doc.getRoot();
        if(!root)
            EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

        for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
        {
            PetLevelTpl::Ptr petLevelTpl = PetLevelTpl::create();
            std::string idStr = itemNode.getChildNodeText<std::string>("id_level");
            std::vector<uint32_t> id_level = componet::fromString<std::vector<uint32_t>>(idStr, "-");
            if(id_level.size() < 2)
                continue;
            petLevelTpl->skillId = id_level[0];
            petLevelTpl->skillLv = id_level[1];

            std::vector<std::string> subvs;
            std::string str = itemNode.getChildNodeText<std::string>("props");
            std::vector<std::string> vs = componet::splitString(str, ",");
            for(const auto& iter : vs)
            {
                subvs.clear();
                subvs = componet::splitString(iter, "-");
                if(subvs.size() < 2)
                    continue;

                PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
                uint32_t prop_val = atoi(subvs[1].c_str());
                petLevelTpl->props.push_back(std::make_pair(prop_type, prop_val));
            }

            uint32_t key = pet_level_hash(petLevelTpl->skillId, petLevelTpl->skillLv);
            m_petLevelTpls.insert({key, petLevelTpl});
        }
    }

}

PetTpl::Ptr PetBase::getPetTpl(TplId petTplId)
{
    if(m_petTpls.find(petTplId) == m_petTpls.end())
        return nullptr;
    return m_petTpls[petTplId];
}

PetLevelTpl::Ptr PetBase::getPetLevelTpl(uint32_t skillId, uint32_t skillLv)
{
    uint32_t key = pet_level_hash(skillId, skillLv);
    if(m_petLevelTpls.find(key) == m_petLevelTpls.end())
        return nullptr;
    return m_petLevelTpls[key];
}

}
