#include "npc_manager.h"
#include "world.h"
#include "scene.h"

#include "water/componet/string_kit.h"

namespace world{

NpcManager& NpcManager::me()
{
    static NpcManager me;
    return me;
}

NpcManager::NpcManager()
{
}

NpcManager::~NpcManager()
{
}

void NpcManager::loadConfig(const std::string& configDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = configDir + "/npc.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        NpcTpl::Ptr tpl(new NpcTpl);
        tpl->id      = itemNode.getChildNodeText<NpcTplId>("id");
        tpl->name    = itemNode.getChildNodeText<std::string>("name");
        tpl->type    = static_cast<NpcType>(itemNode.getChildNodeText<uint16_t>("type"));
        tpl->job     = static_cast<Job>(itemNode.getChildNodeText<uint8_t>("job"));
        tpl->level   = itemNode.getChildNodeText<uint32_t>("level");
        tpl->roleExp = itemNode.getChildNodeText<uint64_t>("roleExp");
        tpl->heroExp = itemNode.getChildNodeText<uint64_t>("heroExp");
        tpl->aiTplId = itemNode.getChildNodeText<uint32_t>("ai");
        tpl->keepCorpseSec = itemNode.getChildNodeText<uint16_t>("keepCorpseSec");
        tpl->reliveSec = itemNode.getChildNodeText<uint16_t>("reliveSec");
        tpl->timeOfBelongTo = itemNode.getChildNodeText<uint16_t>("timeOfBelongTo");

        std::string objDropStr = itemNode.getChildNodeText<std::string>("objDrop");
        tpl->rewardRandomId = itemNode.getChildNodeText<uint32_t>("rewardRandomId");
        std::vector<std::string> objDropInfoEleVec;
        std::vector<std::string> objDropInfoStrVec = water::componet::splitString(objDropStr,";");
        LOG_DEBUG("怪物:{},共有{}种物品掉落", tpl->id, objDropInfoStrVec.size());
        for(const auto& iters : objDropInfoStrVec)
        {
            objDropInfoEleVec.clear();
            objDropInfoEleVec = water::componet::splitString(iters,"-");
            if(objDropInfoEleVec.size() != 4 && objDropInfoEleVec.size() !=0)
            {
                LOG_ERROR("怪物死亡物品掉落,npc.xml的objectDrop列配置有问题,npcId={}",tpl->id);
                continue;
            }
            NpcTpl::ObjDropInfo objDropInfo;
            objDropInfo.objId = water::componet::fromString<uint32_t>(objDropInfoEleVec[0]);
            objDropInfo.num =  water::componet::fromString<uint16_t>(objDropInfoEleVec[1]);
            objDropInfo.isOwnByOne = Bind(water::componet::fromString<uint16_t>(objDropInfoEleVec[2]));
            if((objDropInfo.isOwnByOne != Bind::yes) && (objDropInfo.isOwnByOne != Bind::no))
            {
                LOG_ERROR("怪物死亡物品掉落,npc.xml的objectDrop列配置有问题,绑定只能为1或者2,npcId={}",tpl->id);
                continue;
            }
            objDropInfo.probability = water::componet::fromString<uint32_t>(objDropInfoEleVec[3]);
            tpl->objDropInfos.push_back(objDropInfo);
        }
        tpl->objDropInfos.shrink_to_fit();

        std::vector<std::string> subvs;
        std::string str = itemNode.getChildNodeText<std::string>("props");
        std::vector<std::string> vs = water::componet::splitString(str, ",");
        for(const auto& iter : vs)
        {
            subvs.clear();
            subvs = water::componet::splitString(iter, "-");
            if(subvs.size() < 2)
                continue;

            PropertyType prop_type = static_cast<PropertyType>(atoi(subvs[0].c_str()));
            uint32_t prop_val = atoi(subvs[1].c_str());
            tpl->props.push_back(std::make_pair(prop_type, prop_val));
        }

        tpl->stepCost = std::chrono::milliseconds(itemNode.getChildNodeText<int32_t>("stepCost"));
        tpl->stopCost = std::chrono::milliseconds(itemNode.getChildNodeText<int32_t>("stopCost"));

        tpl->skillTplId = itemNode.getChildNodeText<uint32_t>("skill");
        tpl->extend = itemNode.getChildNodeText<uint16_t>("extend");

        m_tpls[tpl->id] = tpl;
    }
}

Npc::Ptr NpcManager::getById(NpcId id)
{
    auto it = find(id);
    if(it == end())
        return nullptr;
    return it->second;
}

Npc::Ptr NpcManager::createNpc(NpcTplId tplId)
{
    auto it = m_tpls.find(tplId);
    if(it == m_tpls.end())
    {
        LOG_TRACE("Npc创建失败, tplId={}不存在", tplId);
        return nullptr;
    }

    NpcTpl::Ptr tpl = it->second;
    Npc::Ptr npc = Npc::create(++m_lastNpcId, tpl);
    npc->initTplData(); //这里依赖shared_from_this, 不能放入构造函数, 只能两步初始化, 破坏掉raii了......

    if(!insert(std::make_pair(npc->id(), npc)))
        return nullptr;

    return npc;
}

void NpcManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::milliseconds(100),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::msec_100, _1));
    World::me().regTimer(std::chrono::milliseconds(300),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::msec_300, _1));
    World::me().regTimer(std::chrono::milliseconds(500),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::msec_500, _1));

    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&NpcManager::timerLoop, this, StdInterval::sec_1, _1));
    World::me().regTimer(std::chrono::seconds(3), 
                         std::bind(&NpcManager::timerLoop, this, StdInterval::sec_3, _1));
    World::me().regTimer(std::chrono::seconds(5),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::sec_5, _1));
    World::me().regTimer(std::chrono::seconds(15),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::sec_15, _1));
    World::me().regTimer(std::chrono::seconds(30),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::sec_30, _1));

    World::me().regTimer(std::chrono::minutes(1),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::min_1, _1));
    World::me().regTimer(std::chrono::minutes(5),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::min_5, _1));
    World::me().regTimer(std::chrono::minutes(10),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::min_10, _1));
    World::me().regTimer(std::chrono::minutes(15),
                         std::bind(&NpcManager::timerLoop, this, StdInterval::min_15, _1));
}

void NpcManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
    for(auto it = begin(); it != end(); /**/)
    {
        if(it->second->needErase())
        {
            auto s = it->second->scene();
            if(nullptr != s)
                s->eraseNpc(it->second);
            it = erase(it);
        } 
        else
        {
            it->second->timerLoop(interval, now);
            ++it;
        }
    }
}


}


