#include "horse_base.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{

HorseBase& HorseBase::me()
{
    static HorseBase me;
    return me;
}

void HorseBase::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    {//坐骑配置配置加载
        const std::string cfgFile = cfgdir + "/horseTrain.xml";
        LOG_TRACE("读取配置文件 {}", cfgFile);

        XmlParseDoc doc(cfgFile);
        XmlParseNode root = doc.getRoot();
        if(!root)
            EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

        for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
        {
            HorseTrainTpl::Ptr horseTrainTpl = HorseTrainTpl::create();
            horseTrainTpl->star = itemNode.getChildNodeText<uint8_t>("star");
            horseTrainTpl->nextexp = itemNode.getChildNodeText<uint32_t>("nextexp");
            horseTrainTpl->skin = itemNode.getChildNodeText<uint16_t>("skin");
            horseTrainTpl->moneyType = static_cast<MoneyType>(itemNode.getChildNodeText<uint16_t>("moneytype"));
            horseTrainTpl->money = itemNode.getChildNodeText<uint64_t>("money");

            std::vector<std::string> subvs;
            std::vector<std::string> vs;
            {//道具消耗
                std::string str = itemNode.getChildNodeText<std::string>("costobj");
                vs = componet::splitString(str, ",");
                for(const auto& iter : vs)
                {
                    TplId objId = atoi(iter.c_str());
                    horseTrainTpl->costObjs.insert(objId);
                }
            }

            {//角色增加属性
                vs.clear();
                std::string str = itemNode.getChildNodeText<std::string>("roleprop");
                vs = componet::splitString(str, ",");
                for(const auto& iter : vs)
                {
                    subvs.clear();
                    subvs = componet::splitString(iter, "-");
                    if(subvs.size() < 2)
                        continue;

                    PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
                    uint32_t prop_val = atoi(subvs[1].c_str());
                    horseTrainTpl->roleProps.push_back(std::make_pair(prop_type, prop_val));
                }
            }

            {//英雄增加属性
                subvs.clear();
                vs.clear();
                std::string str = itemNode.getChildNodeText<std::string>("heroprop");
                std::vector<std::string> vs = componet::splitString(str, ",");
                for(const auto& iter : vs)
                {
                    subvs.clear();
                    subvs = componet::splitString(iter, "-");
                    if(subvs.size() < 2)
                        continue;

                    PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
                    uint32_t prop_val = atoi(subvs[1].c_str());
                    horseTrainTpl->heroProps.push_back(std::make_pair(prop_type, prop_val));
                }
            }

            m_horseTrainTpls.insert({horseTrainTpl->star, horseTrainTpl});
        }
    }

    {//坐骑皮肤配置
        const std::string cfgFile = cfgdir + "/horseSkin.xml";
        LOG_TRACE("读取配置文件 {}", cfgFile);

        XmlParseDoc doc(cfgFile);
        XmlParseNode root = doc.getRoot();
        if(!root)
            EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

        for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
        {
            HorseSkinTpl::Ptr horseSkinTpl = HorseSkinTpl::create();
            horseSkinTpl->skin = itemNode.getChildNodeText<uint16_t>("skin");
            horseSkinTpl->type = itemNode.getChildNodeText<uint8_t>("type");
            horseSkinTpl->notice = itemNode.getChildNodeText<std::string>("notice");

            std::vector<std::string> subvs;
            std::vector<std::string> vs;
            {
                std::string str = itemNode.getChildNodeText<std::string>("skinprop");
                vs = componet::splitString(str, ",");
                for(const auto& iter : vs)
                {
                    subvs.clear();
                    subvs = componet::splitString(iter, "-");
                    if(subvs.size() < 2)
                        continue;

                    PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
                    uint32_t prop_val = atoi(subvs[1].c_str());
                    horseSkinTpl->skinProps.push_back(std::make_pair(prop_type, prop_val));
                }
            }

            m_horseSkinTpls.insert({horseSkinTpl->skin, horseSkinTpl});
        }
    }

    {//坐骑培养暴击配置
        const std::string cfgFile = cfgdir + "/horseTrainMultiplier.xml";
        LOG_TRACE("读取配置文件 {}", cfgFile);

        XmlParseDoc doc(cfgFile);
        XmlParseNode root = doc.getRoot();
        if(!root)
            EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

        for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
        {
            HorseRateTpl::Ptr horseRateTpl = HorseRateTpl::create();
            horseRateTpl->rate = itemNode.getChildNodeText<uint8_t>("rate");
            horseRateTpl->needRaiseCount = itemNode.getChildNodeText<uint16_t>("needcount");
            horseRateTpl->effectNum = itemNode.getChildNodeText<uint8_t>("effectnum");
            horseRateTpl->weight = itemNode.getChildNodeText<uint16_t>("weight");

            m_horseRateTpls.insert({horseRateTpl->rate, horseRateTpl});
        }
    }
}


void HorseBase::execSkin(std::function<void (HorseSkinTpl::Ptr)> exec)
{
    for(const auto& iter : m_horseSkinTpls)
        exec(iter.second);
}

void HorseBase::execRate(std::function<void (HorseRateTpl::Ptr)> exec)
{
    for(const auto& iter : m_horseRateTpls)
        exec(iter.second);
}

HorseTrainTpl::Ptr HorseBase::getTrainTpl(uint8_t star)
{
    if(m_horseTrainTpls.find(star) == m_horseTrainTpls.end())
        return nullptr;
    return m_horseTrainTpls[star];
}

HorseSkinTpl::Ptr HorseBase::getSkinTpl(uint16_t skin)
{
    if(m_horseSkinTpls.find(skin) == m_horseSkinTpls.end())
        return nullptr;
    return m_horseSkinTpls[skin];
}

HorseRateTpl::Ptr HorseBase::getRateTpl(uint8_t rate)
{
    if(m_horseRateTpls.find(rate) == m_horseRateTpls.end())
        return nullptr;
    return m_horseRateTpls[rate];
}

}
