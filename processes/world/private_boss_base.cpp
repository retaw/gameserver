#include "private_boss_base.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace world{


PrivateBossBase& PrivateBossBase::me()
{
    static PrivateBossBase me;
    return me;
}

void PrivateBossBase::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/personal_boss.xml";

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    std::vector<std::string> vec1;
    std::vector<std::string> vec2;
    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        PrivateBossTpl::Ptr bossPtr = PrivateBossTpl::create();
        bossPtr->bossId = itemNode.getChildNodeText<uint32_t>("bossId");
        bossPtr->npcId = itemNode.getChildNodeText<uint32_t>("npcId");
        bossPtr->npcMapId = itemNode.getChildNodeText<uint32_t>("mapId");
        std::string str = itemNode.getChildNodeText<std::string>("transfer");
        vec1.clear();
        vec1 =  water::componet::splitString(str, "_");
        if(vec1.size() < 3)
        {
            LOG_ERROR("{}配置错误, transfer出错, bossId={}", cfgdir, bossPtr->bossId);
            continue;
        }
        bossPtr->transferMapId = water::componet::fromString<uint32_t>(vec1[0]);
        bossPtr->posx = water::componet::fromString<uint16_t>(vec1[1]);
        bossPtr->posy = water::componet::fromString<uint16_t>(vec1[2]);

        std::string str1 = itemNode.getChildNodeText<std::string>("vipDayNum");
        vec1.clear();
        vec1 = water::componet::splitString(str1, ",");
        for(auto& it : vec1)
        {
            vec2.clear();
            vec2 = water::componet::splitString(it, "_");
            if(vec2.size() < 2)
            {
                LOG_ERROR("{}配置错误, vipDayNum出错, bossId={}", cfgdir, bossPtr->bossId);
                continue;
            }
            bossPtr->enterTimes.push_back(water::componet::fromString<uint16_t>(vec2[1]));
        }

        std::string str2 = itemNode.getChildNodeText<std::string>("reward");
        vec1.clear();
        vec1 = water::componet::splitString(str2, ",");
        for(auto& it : vec1)
        {
            vec2.clear();
            vec2 = water::componet::splitString(it, "-");
            if(vec2.size() < 3)
            {
                LOG_ERROR("{}配置错误, reward出错, bossId={}", cfgdir, bossPtr->bossId);
                continue;
            }
            PrivateBossTpl::Reward reward;
            reward.objTplId = water::componet::fromString<uint32_t>(vec2[0]);
            reward.objnum = water::componet::fromString<uint16_t>(vec2[1]);
            reward.bind = Bind(water::componet::fromString<uint8_t>(vec2[2]));
            bossPtr->reward.push_back(reward);
        }
        bossPtr->duration = itemNode.getChildNodeText<uint32_t>("duration");
        m_privateBoss.emplace(bossPtr->bossId, bossPtr);
        m_npcTplIdMapId2BossId.emplace(std::make_pair(bossPtr->npcId, bossPtr->npcMapId), bossPtr->bossId);
    }
}

uint32_t PrivateBossBase::getKeepDuration(uint32_t bossId)
{
    auto pair = m_privateBoss.find(bossId);
    if(pair == m_privateBoss.end())
    {
        LOG_ERROR("个人boss获取活动duration失败, 不存在的bossId={}", bossId);
        return 0;
    }
    return pair->second->duration;

}


}
