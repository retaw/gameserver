#include "factionactive_manager.h"
#include "world.h"

#include "protocol/rawmsg/public/faction_active.h"
#include "protocol/rawmsg/public/faction_active.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

#include  "protocol/rawmsg/private/relay.h"
#include  "protocol/rawmsg/private/relay.codedef.private.h"

namespace world{

FactionActiveManager& FactionActiveManager::me()
{
    static FactionActiveManager me;
    return me;
}

void FactionActiveManager::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/faction_task.xml";
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");
    m_taskStartLevel = root.getChildNodeText<uint32_t>("factionTaskStartLevel");
    m_taskNum = root.getChildNodeText<uint16_t>("factionTaskNum");
    m_rewardId = root.getChildNodeText<uint32_t>("rewardId");

    XmlParseNode priceNode = root.getChild("price");
    m_vipBuyPriceNum = priceNode.getAttr<uint64_t>("num");
    m_vipBuyMoneytype = (MoneyType)priceNode.getAttr<uint8_t>("money_type");

    std::vector<FactionTaskRewardConf> confs;
    XmlParseNode taskNumNode = root.getChild("taskNum");
    for(XmlParseNode numNode = taskNumNode.getChild("num"); numNode; ++numNode)
    {
        confs.clear();
        for(XmlParseNode roleLevelNode = numNode.getChild("roleLevel"); roleLevelNode; ++roleLevelNode)
        {
            FactionTaskRewardConf conf;
            conf.minLevel = roleLevelNode.getAttr<uint32_t>("miniRoleLevel");
            conf.roleExp = roleLevelNode.getAttr<uint32_t>("roleExp");
            conf.heroExp = roleLevelNode.getAttr<uint32_t>("heroExp");
            conf.factionExp = roleLevelNode.getAttr<uint32_t>("factionExp");
            conf.factionResource = roleLevelNode.getAttr<uint32_t>("factionResource");
            conf.banggong = roleLevelNode.getAttr<uint32_t>("banggong");
            conf.objTplId = roleLevelNode.getAttr<uint32_t>("objTmlId");
            conf.objNum = roleLevelNode.getAttr<uint32_t>("objNum");
            conf.bind = (Bind)roleLevelNode.getAttr<uint8_t>("bind");

            confs.emplace_back(conf);
        }
        m_rewardConfs.emplace_back(confs);
    }
    
}

void FactionActiveManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(FactionActive, std::bind(&FactionActiveManager::clientmsg_FactionActive,this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(FactionTask, std::bind(&FactionActiveManager::clientmsg_FactionTask,this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(AcceptFactionTask, std::bind(&FactionActiveManager::clientmsg_AcceptFactionTask,this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(FinishFactionTask, std::bind(&FactionActiveManager::clientmsg_FinishFactionTask,this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(FactionTaskBuyVipGift, std::bind(&FactionActiveManager::clientmsg_FactionTaskBuyVipGift,this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(QuitFactionTask, std::bind(&FactionActiveManager::clientmsg_QuitFactionTask,this, _1, _2, _3));
}
void FactionActiveManager::clientmsg_FactionActive(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(0 != msgSize)
        return;
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    PublicRaw::RetFactionActive send;
    send.taskNum = m_taskNum;
    send.finishedTaskNum = role->m_roleFactionTask.m_taskInfo.taskRecord;
//...
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionActive), &send, sizeof(send));
}


void FactionActiveManager::clientmsg_FactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(!factionTaskStart(role))
    {
        role->sendSysChat("你的帮派任务功能未开启,需要帮派等级达到{}级", m_taskStartLevel);
    }

    role->m_roleFactionTask.sendRetFactionTask();
}

void FactionActiveManager::clientmsg_AcceptFactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return; 
    role->m_roleFactionTask.acceptTask();
}

void FactionActiveManager::clientmsg_FinishFactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    role->m_roleFactionTask.finishTask();
}

void FactionActiveManager::clientmsg_QuitFactionTask(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    role->m_roleFactionTask.quitTask();
}

void FactionActiveManager::clientmsg_FactionTaskBuyVipGift(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    role->m_roleFactionTask.buyVipGift(m_rewardId);
}

/*
void FactionActiveManager::sendRetFactionTask(Role& role)
{
    //检查，如果当前保存的任务不是今天de，刷新...
    if(role.m_roleFactionTask.taskOutTime())
        role.m_roleFactionTask.refreshTask();

    if(role.m_roleFactionTask.m_taskInfo.taskId == 0) //一定是未开启帮派任务或者当天任务完成
    {
        return;
    }
    else
    {
        fillFactionTaskReward(role);
        PublicRaw::RetFactionTask send;
        send.taskId = role.m_roleFactionTask.m_taskInfo.taskId;
        send.state = role.m_roleFactionTask.m_taskInfo.state;
        send.reward = role.m_roleFactionTask.m_taskInfo.reward;
        role.sendToMe(RAWMSG_CODE_PUBLIC(RetFactionTask), &send, sizeof(send));
    }
}
*/
void FactionActiveManager::fillFactionTaskReward(Role& role)
{
    auto& reward = role.m_roleFactionTask.m_taskInfo.reward;
    uint32_t num = role.m_roleFactionTask.m_taskInfo.taskRecord;
    if(m_rewardConfs.size() <= num)
        return;
    for(auto& it : m_rewardConfs[num])
    {
        if(it.minLevel <= role.level())
        {
            reward.roleExp = it.roleExp;
            reward.heroExp = it.heroExp;
            reward.banggong = it.banggong;
            reward.factionExp = it.factionExp;
            reward.factionResource = it.factionResource;
            reward.objTplId = it.objTplId;
            reward.objNum = it.objNum;
            role.m_roleFactionTask.m_taskInfo.bind = it.bind;
            break;
        }
    }
}

bool FactionActiveManager::factionTaskStart(Role::Ptr role)
{
    return role->level() >= m_taskStartLevel;
}

}
