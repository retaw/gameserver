#include "expbead_manager.h"
#include "mail_manager.h"

#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/random.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/expbead.h"
#include "protocol/rawmsg/public/expbead.codedef.public.h"

namespace world{

ExpBeadManager& ExpBeadManager::me()
{
    static ExpBeadManager me;
    return me;
}

void ExpBeadManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(ExpBeadShow, std::bind(&ExpBeadManager::clientmsg_ExpBeadShow, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ExpBeadMainWindow, std::bind(&ExpBeadManager::clientmsg_ExpBeadMainWindow, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ExpBeadGet, std::bind(&ExpBeadManager::clientmsg_ExpBeadGet, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ExpBeadRefresh, std::bind(&ExpBeadManager::clientmsg_ExpBeadRefresh, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(BuyGetTimes, std::bind(&ExpBeadManager::clientmsg_BuyGetTimes, this, _1, _2, _3));
}

void ExpBeadManager::loadConfig(const std::string& cfgDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgDir + "/expbead.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);
    
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(ExpBeadException, cfgFile + "parse root node failed");
    
    m_cfg.openLevel = root.getChildNodeText<uint32_t>("openLevel");
    m_cfg.refreshCostMoney = root.getChildNodeText<uint64_t>("refreshCostMoney");
    m_cfg.refreshUpTimes = root.getChildNodeText<uint16_t>("refreshUpTimes");
    m_cfg.maxGetTimes = root.getChildNodeText<uint16_t>("maxGetTimes");
    m_cfg.buyRefreshCostVcoin = root.getChildNodeText<uint64_t>("buyRefreshCostVcoin");
    m_cfg.buyGetTimesCost = root.getChildNodeText<uint64_t>("buyGetTimesCost");

    //vips
    XmlParseNode vipsNode = root.getChild("vips");
    uint32_t vipsMax = vipsNode.getAttr<uint32_t>("max");
    for(XmlParseNode vipNode = vipsNode.getChild("vip"); vipNode; ++vipNode)
    {
        Vip vip;
        vip.level = vipNode.getAttr<uint32_t>("level");
        vip.num = vipNode.getAttr<uint16_t>("num");
        m_cfg.vipVec.emplace_back(std::move(vip));
    }
    if(vipsMax != m_cfg.vipVec.size() - 1)
        EXCEPTION(ExpBeadException, cfgFile + "vips' max does not match");
    //starLevels
    XmlParseNode starLevelsNode = root.getChild("starLevels");
    uint32_t starLevelsNum = starLevelsNode.getAttr<uint32_t>("num");
    for(XmlParseNode starLevelNode = starLevelsNode.getChild("starLevel"); starLevelNode; ++starLevelNode)
    {
        StarLevel starLevel;
        starLevel.level = starLevelNode.getAttr<uint16_t>("level");
        starLevel.exp = starLevelNode.getAttr<uint64_t>("exp");
        starLevel.addLevelRandom = starLevelNode.getAttr<uint16_t>("addLevelRandom");
        m_cfg.starLevelVec.emplace_back(std::move(starLevel));
    }
    if(starLevelsNum != m_cfg.starLevelVec.size())
        EXCEPTION(ExpBeadException, cfgFile + "starLevel' num does not match");
    //types
    XmlParseNode typesNode = root.getChild("types");
    uint32_t typesNum = typesNode.getAttr<uint32_t>("num");
    for(XmlParseNode typeNode = typesNode.getChild("type"); typeNode; ++typeNode)
    {
        Type type;
        type.ratio = typeNode.getAttr<uint16_t>("ratio");
        type.costVcoin = typeNode.getAttr<uint32_t>("costVcoin");
        type.objectId = typeNode.getAttr<uint64_t>("objectTplId");
        type.objectNum = typeNode.getAttr<uint32_t>("objectNum");
        type.random = typeNode.getAttr<uint16_t>("random");
        type.objBind = Bind(typeNode.getAttr<uint8_t>("objBind"));
        type.vip = typeNode.getAttr<uint32_t>("vip");
        m_cfg.typeMap.emplace(type.ratio, std::move(type));
    }
    if(typesNum != m_cfg.typeMap.size())
        EXCEPTION(ExpBeadException, cfgFile + "types' num does not match");
}

void ExpBeadManager::clear()
{
    m_expBeadInfo.clear();
}

void ExpBeadManager::clientmsg_ExpBeadShow(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    LOG_DEBUG("经验珠是否可用");
    if(msgSize != 0)
        return;
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->level() < m_cfg.openLevel)
        return;
    if(role->level() < m_cfg.openLevel)
    {
        role->sendSysChat("{}级时此功能开启", m_cfg.openLevel);
        return;
    }
    PublicRaw::RetExpBeadShow send;
    send.show = 1;
    auto it = m_expBeadInfo.find(roleId);
    if(it != m_expBeadInfo.end())
    {
        //是否有领取
        if(it->second.getTimes < 1)
            return;
        if(it->second.refreshTimes < 1)
            return;
    }
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetExpBeadShow), (uint8_t*)&send, sizeof(send));
    LOG_DEBUG("经验珠可用");
}

void ExpBeadManager::clientmsg_ExpBeadMainWindow(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("经验珠窗口, 消息错误, msgsize={}", msgSize);
        return;
    }
    LOG_DEBUG("经验珠主窗口请求");
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->level() < m_cfg.openLevel)
    {
        role->sendSysChat("{}级时此功能开启", m_cfg.openLevel);
        return;
    }

    sendMainWindowToClient(role);
}

void ExpBeadManager::clientmsg_ExpBeadGet(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    LOG_DEBUG("经验珠领取");
    if(msgSize != sizeof(PublicRaw::ExpBeadGet))
    {
        LOG_DEBUG("经验珠领取, 消息错误, msgsize={}", msgSize);
        return;
    }

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->level() < m_cfg.openLevel)
    {
        role->sendSysChat("{}级时此功能开启", m_cfg.openLevel);
        return;
    }

    auto rev = reinterpret_cast<const PublicRaw::ExpBeadGet *>(msgData);
    if(!getExpBead(role, rev->ratio))
        return;

    sendMainWindowToClient(role); 
}

void ExpBeadManager::clientmsg_ExpBeadRefresh(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    LOG_DEBUG("经验珠刷新");
    if(msgSize != 0)
    {
        LOG_DEBUG("经验珠领取, 消息错误, msgsize={}", msgSize);
        return;
    }

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->level() < m_cfg.openLevel)
    {
        role->sendSysChat("{}级时此功能开启", m_cfg.openLevel);
        return;
    }

    if(!expBeadRefresh(role))
        return;

    sendMainWindowToClient(role);
}

void ExpBeadManager::clientmsg_BuyGetTimes(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
   LOG_DEBUG("经验珠购买领取次数");
   if(msgSize != 0)
   {
        LOG_DEBUG("经验珠购买领取次数, msgsize={}", msgSize);
        return;
   }

   auto role = RoleManager::me().getById(roleId);
   if(role == nullptr)
       return;
    if(role->level() < m_cfg.openLevel)
    {
        role->sendSysChat("{}级时此功能开启", m_cfg.openLevel);
        return;
    }

   if(!buyGetTimes(role))
       return;

   PublicRaw::RetBuyGetTimes send;
   send.getTimes = m_expBeadInfo[roleId].getTimes;
   send.buyGetTimes = m_expBeadInfo[roleId].buyGetTimes;
   role->sendToMe(RAWMSG_CODE_PUBLIC(RetBuyGetTimes), (uint8_t*)&send, sizeof(send));
   LOG_DEBUG("经验珠购买领取次数回复, getTimes={}, buyGetTimes={}",
             send.getTimes, send.buyGetTimes);
}

void ExpBeadManager::sendMainWindowToClient(Role::Ptr role)
{
    PublicRaw::RetExpBeadMaiWindow send;
    if(!fillMainWindow(role, send))
        return;

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetExpBeadMaiWindow), (uint8_t*)&send, sizeof(send));
    LOG_DEBUG("经验珠主窗口回复, starLevel={}, starLevelExp={}, starUpLevelExp={}, getTimes={}, refreshTimes={}, refreshMaxTimes={}",
              send.starLevel, send.starLevelExp, send.starUpLevelExp, send.getTimes, send.refreshTimes, send.refreshMaxTimes);
    
}

bool ExpBeadManager::fillMainWindow(Role::Ptr role, PublicRaw::RetExpBeadMaiWindow& send)
{
    if(role == nullptr)
        return false;
    send.starLevel = 1;
    send.refreshTimes = m_cfg.refreshUpTimes;//先设置为最大（非vip）
    send.getTimes = m_cfg.maxGetTimes;
    send.buyGetTimes = 0;

    auto it = m_expBeadInfo.find(role->id());
    if(it == m_expBeadInfo.end())
    {
        if(!initBeanInfo(role))
            return false;
    }
    send.starLevel = m_expBeadInfo[role->id()].starLevel;
    send.refreshTimes = m_expBeadInfo[role->id()].refreshTimes;
    send.getTimes = m_expBeadInfo[role->id()].getTimes;
    send.buyGetTimes = m_expBeadInfo[role->id()].buyGetTimes;

    if(send.starLevel > m_cfg.starLevelVec.size())
    {
        LOG_ERROR("经验珠等级在配置中不存在, startLevel={}, starLevelSize={}", 
                  send.starLevel, m_cfg.starLevelVec.size());
        return false;
    }
    send.starLevelExp = m_cfg.starLevelVec[send.starLevel -1 ].exp;
    send.starUpLevelExp = m_cfg.starLevelVec.back().exp;
    send.maxGetTimes = m_cfg.maxGetTimes;
    send.refreshMaxTimes = m_cfg.refreshUpTimes;

    return true;
}

bool ExpBeadManager::getExpBead(Role::Ptr role, const uint16_t ratio)
{
    //先做所有的条件判断，成立后再操作
    if(role == nullptr)
        return false;
    //做领取次数判断
    auto infoiter = m_expBeadInfo.find(role->id());
    if(infoiter != m_expBeadInfo.end())
    {
        if(infoiter->second.getTimes < 1)
            return false;
    }
    //判断该倍数是否在配置中
    auto it = m_cfg.typeMap.find(ratio);
    if(it == m_cfg.typeMap.end())
    {
        LOG_ERROR("经验珠领取, 领取倍数在配置中不存在, ratio{}", ratio);
        return false;
    }
    //判断元宝是否够消耗
    uint64_t money_4Num = role->getMoney(MoneyType::money_4);//非绑定元宝
    if(it->second.costVcoin > money_4Num)
        return false;

    //执行所有操作

    //1.首先花钱,花钱失败后面的操作都失败
    if(it->second.costVcoin != 0)
    {
        if(!role->reduceMoney(MoneyType::money_4, it->second.costVcoin, "经验珠领取功能"))
            return false;
    }

    //2.概率获得装备
    if(it->second.objectId != 0 && it->second.objectNum != 0 && it->second.objBind != Bind::none)
    {
        water::componet::Random<uint16_t> rand(1, 10000);
        if(rand.get() <= it->second.random)
        {
            //检查背包是否满
            if(role->checkPutObj(it->second.objectId, it->second.objectNum, it->second.objBind))
                role->putObj(it->second.objectId, it->second.objectNum, it->second.objBind, PackageType::role);
            else
            {
                std::vector<ObjItem> objVec;
                ObjItem temp;
                temp.tplId = it->second.objectId;
                temp.num = it->second.objectNum;
                temp.bind = it->second.objBind;
                objVec.push_back(std::move(temp));
                std::string text = "您获得了灵珠奖励";
                MailManager::me().send(role->id(), "灵珠奖励", text, objVec);
                role->sendSysChat("背包空间不足, 奖励通过邮件发放");

            }
        }
    }

    //3.获取经验并设置expbeaninfo
    addExpAndSetInfo(role, ratio);

    return true;
}

bool ExpBeadManager::addExpAndSetInfo(Role::Ptr role, const uint16_t ratio)
{
    //英雄领取经验
    if(m_cfg.starLevelVec.size() < 1)
        return false;
    uint64_t exp = m_cfg.starLevelVec[0].exp;//默认领取1级经验
    auto beadInfoiter = m_expBeadInfo.find(role->id());
    if(beadInfoiter != m_expBeadInfo.end())
    {
        if(m_cfg.starLevelVec.size() < beadInfoiter->second.starLevel)
            return false;
        exp = m_cfg.starLevelVec[beadInfoiter->second.starLevel - 1].exp * ratio;
    }
    auto hero = role->m_heroManager.getDefaultHero();
    if(hero != nullptr)//不应该为空
        hero->addExp(exp);

    //初始经验珠数据等级修改剩余领取次数
    if(beadInfoiter == m_expBeadInfo.end())
    {
        if(!initBeanInfo(role))
            return false;
    }
    --m_expBeadInfo[role->id()].getTimes;
    m_expBeadInfo[role->id()].starLevel = 1;
    //if vip
    //info.refreshTimes = m_cfg.refreshUpTimes;

    return true;
}

bool ExpBeadManager::expBeadRefresh(Role::Ptr role)
{
    //所有的判断
    //金钱是否够用
    uint64_t money_1Num = role->getMoney(MoneyType::money_1);//绑定元宝
    uint64_t money_2Num = role->getMoney(MoneyType::money_2);//非绑定元宝
    if(m_cfg.refreshCostMoney > money_1Num + money_2Num)
        return false;
    //是否还有刷新次数
    auto info = m_expBeadInfo.find(role->id());
    if(info != m_expBeadInfo.end())
    {
        if(info->second.starLevel == 6)//满级不再刷新
            return false;
        if(info->second.refreshTimes < 1)
        {
            //元宝是否可购买刷新次数,并混合消费元宝
            if(!costVcoin(role, m_cfg.buyRefreshCostVcoin, "经验珠刷新功能, 购买刷新次数"))
                return false;
            ++m_expBeadInfo[role->id()].refreshTimes;
        }
    }
    
    //执行所有操作
    //1.首先花钱,花钱失败后面的操作都失败
    if(m_cfg.refreshCostMoney > money_1Num + money_2Num)
        return false;
    if(m_cfg.refreshCostMoney < money_1Num)//消耗绑定金币
    {
        if(!role->reduceMoney(MoneyType::money_1, m_cfg.refreshCostMoney, "经验珠刷新功能"))
            return false;
    }
    else//混合消耗金币
    {
        if(!role->reduceMoney(MoneyType::money_1, money_1Num, "经验珠刷新功能"))
            return false;
        if(!role->reduceMoney(MoneyType::money_2, m_cfg.refreshCostMoney - money_1Num, "经验珠刷新功能"))
            return false;
    }

    //刷新后配置新的等级
    if(info == m_expBeadInfo.end())
    {
        if(!initBeanInfo(role))
            return false;
    }
    --m_expBeadInfo[role->id()].refreshTimes;

    water::componet::Random<uint16_t> rand(1, 10000);
    if(m_expBeadInfo[role->id()].starLevel > m_cfg.starLevelVec.size())
        return false;
    if(rand.get() <= m_cfg.starLevelVec[m_expBeadInfo[role->id()].starLevel - 1].addLevelRandom)
    {
        //add
        if(m_expBeadInfo[role->id()].starLevel < 6)
            m_expBeadInfo[role->id()].starLevel++;
    }
    else
    {
        //sub
        if(m_expBeadInfo[role->id()].starLevel > 1)
            m_expBeadInfo[role->id()].starLevel--;
    }

    return true;
}

bool ExpBeadManager::buyGetTimes(Role::Ptr role)
{
    //判断是否可购买
    auto it = m_expBeadInfo.find(role->id());
    if(it == m_expBeadInfo.end())
    {
        if(!initBeanInfo(role))
            return false;
    }
    //判断是否还有购买次数
    //vipLevel = role->vip()...
    uint32_t vipLevel = 0;
    if(m_cfg.vipVec.size() < vipLevel)
        return false;
    if(m_expBeadInfo[role->id()].buyGetTimes >= m_cfg.vipVec[vipLevel].num)//已购买次数>=可购买
        return false;
    
    //1.花钱（元宝）
    if(!costVcoin(role, m_cfg.buyGetTimesCost, "经验珠领取次数购买"))
        return false;
    //2.增加已购买领取次数
    ++m_expBeadInfo[role->id()].buyGetTimes;
    //3.增加剩余领取次数
    ++m_expBeadInfo[role->id()].getTimes;

    return true;
}

bool ExpBeadManager::initBeanInfo(Role::Ptr role)
{
    m_expBeadInfo[role->id()].starLevel = 1;
    m_expBeadInfo[role->id()].refreshTimes = m_cfg.refreshUpTimes;
    m_expBeadInfo[role->id()].getTimes = m_cfg.maxGetTimes; 
    //vipLevel = role->vip()...
    uint32_t vipLevel = 0;
    if(m_cfg.vipVec.size() < vipLevel)
        return false;
    m_expBeadInfo[role->id()].buyGetTimes = 0;

    return true;
}

bool ExpBeadManager::costVcoin(Role::Ptr role, const uint64_t num, const std::string& reference)
{
    if(num == 0)
        return false;
    uint64_t money_4Num = role->getMoney(MoneyType::money_4);//非绑定元宝
    if(num > money_4Num)
        return false;
    else
    {
        if(!role->reduceMoney(MoneyType::money_4, num, reference))
            return false;
    }
    return true;
}

}
