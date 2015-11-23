#include "store.h"
#include "world.h"
#include "object_config.h"
#include <ctime>
#include "water/componet/random.h"

namespace world{

using namespace water::componet;

Store::Store(uint16_t id)
: m_id(id)
{
}


void Store::parse(const XmlParseNode& node, const XmlParseDoc& xml)
{
    for(XmlParseNode tabNode = node.getChild("tab"); tabNode; ++tabNode)
    {
        TabConf tabcfg;
        tabcfg.tabId = tabNode.getAttr<uint8_t>("id");
        /*
        if(0 == tabcfg.tabId)
            continue;
        */
        if(tabConfs.find(tabcfg.tabId) != tabConfs.end())
        {
            EXCEPTION(componet::ExceptionBase, "shopid={} tab={} 重复 error", m_id, tabcfg.tabId);
            return;
        }

        tabcfg.moneytype = static_cast<MoneyType>(tabNode.getAttr<uint8_t>("money_type"));
        /*
        if(tabcfg.moneytype == MoneyType::none)
        {
            EXCEPTION(componet::ExceptionBase, "shopid={} tab={} 没有指定货币类型,money_type=0,error", m_id, tabcfg.tabId, tabcfg.moneytype);
            return;
        }
        */

        tabcfg.openServerTime = tabNode.getAttr<uint32_t>("open_server_time");
        tabcfg.duration = tabNode.getAttr<uint32_t>("duration");
        tabcfg.sellnum = tabNode.getAttr<uint32_t>("sell_num");
        //parse时间
        {
            std::string time = tabNode.getAttr<std::string>("start_time");
            if(time != "")
                tabcfg.startTime = stringToTimePoint(time);
            time = tabNode.getAttr<std::string>("end_time");
            if(time != "")
                tabcfg.endTime = stringToTimePoint(time);
        }

        //parse itemNode
        for(XmlParseNode itemNode = tabNode.getChild("item"); itemNode; ++itemNode)
        {
            ItemsConf itemcfg;
            itemcfg.itemId = itemNode.getAttr<uint32_t>("id");
            if(0 == itemcfg.itemId)
                continue;
            if(tabcfg.itemConfs.find(itemcfg.itemId) != tabcfg.itemConfs.end())
            {
                EXCEPTION(componet::ExceptionBase, "shopid={} tab={} itemid={} 重复 error", m_id, tabcfg.tabId, itemcfg.itemId);
                return;
            }
            itemcfg.price = itemNode.getAttr<uint64_t>("present_price");
            itemcfg.bind = static_cast<Bind>(itemNode.getAttr<uint8_t>("bind"));
            if(itemcfg.bind == Bind::none)
                itemcfg.bind = Bind::no;
            itemcfg.maxnum = itemNode.getAttr<uint16_t>("maxnum");
            itemcfg.weight = itemNode.getAttr<uint16_t>("weight");
            tabcfg.addUpWeight += itemcfg.weight;
            itemcfg.serverLimit = itemNode.getAttr<uint32_t>("server_limit");
            itemcfg.dayLimit = itemNode.getAttr<uint32_t>("day_limit");
            itemcfg.everyLimit = itemNode.getAttr<uint32_t>("every_limit");
            itemcfg.moneytype = (MoneyType)itemNode.getAttr<uint32_t>("money_type");
            if(tabcfg.moneytype != MoneyType::none)
                itemcfg.moneytype = tabcfg.moneytype;
            itemcfg.discount = itemNode.getAttr<uint16_t>("discount");
            itemcfg.objnum = itemNode.getAttr<uint16_t>("objnum");

            tabcfg.itemConfs.insert(std::make_pair(itemcfg.itemId, itemcfg));
        }

        tabConfs.insert(std::make_pair(tabcfg.tabId, tabcfg));
    }
}

uint64_t Store::price(TplId objId)
{
    if(tabConfs.find(ybTabId) == tabConfs.end())
        return (uint64_t)-1;

    TabConf& tabcfg = tabConfs[ybTabId];
    if(tabcfg.itemConfs.find(objId) == tabcfg.itemConfs.end())
        return (uint64_t)-1;

    ItemsConf& itemcfg = tabcfg.itemConfs[objId];
    return itemcfg.price;
}

void Store::retSellGoods(Role::Ptr role, MoneyType moneytype, BuyRetcode retcode)
{
    std::string text;
    switch(retcode)
    {
    case BuyRetcode::success:
        text = "购买成功";
        break;
    case BuyRetcode::full:
        text = "包裹已满";
        break;
    case BuyRetcode::time:
        text = "不在活动时间";
        break;
    case BuyRetcode::serverLimit:
        text = "该道具已售完";
        break;
    case BuyRetcode::dayLimit:
        text = "今日该道具已售完";
        break;
    case BuyRetcode::everyLimit:
        text = "您今日已达该道具购买上限";
        break;
    default:
        return;
    }
    role->sendSysChat(text);;
}


BuyRetcode Store::checkSellGoods(Role::Ptr role, const ItemsConf& itemcfg, uint32_t goodsNum)
{
    if(itemcfg.everyLimit > 0 
       && role->getBuyDayLimit(itemcfg.itemId) >= itemcfg.everyLimit)
    {
        return BuyRetcode::dayLimit;
    }

    uint16_t sellnum = goodsNum;
    if(itemcfg.maxnum != 0)
        sellnum = goodsNum > itemcfg.maxnum ? itemcfg.maxnum : goodsNum;

    //这里因为帮贡与其他货币处理机制不同，所以不是同一货币接口，需额外处理帮贡
    const uint64_t needMoney = (uint64_t)sellnum * itemcfg.price;
    if(itemcfg.moneytype == MoneyType::money_11)//消耗帮贡
    {
       if(!role->checkBanggong(needMoney))
           return BuyRetcode::banggong;
    }
    else//消耗金币
    {
        if(!role->checkMoney(itemcfg.moneytype, needMoney))
            return BuyRetcode::money;
    }

    if(!role->checkPutObj(itemcfg.itemId, sellnum, itemcfg.bind, PackageType::role))
    {
        return BuyRetcode::full;
    }

    return BuyRetcode::success;
}

void Store::successSellGoods(Role::Ptr role, uint8_t tabId, const ItemsConf& itemcfg, uint32_t goodsNum)
{
    uint16_t sellnum = goodsNum;
    if(itemcfg.maxnum != 0)
        sellnum = goodsNum > itemcfg.maxnum ? itemcfg.maxnum : goodsNum;
    const uint64_t needMoney = (uint64_t)sellnum * itemcfg.price;

    const ObjBasicData& base = ObjectConfig::me().objectCfg.m_objBasicDataMap[itemcfg.itemId];
    role->reduceMoney(itemcfg.moneytype, needMoney, "商城购买物品, objname={}, num={}", base.name, sellnum);
    role->putObj(itemcfg.itemId, sellnum, itemcfg.bind, PackageType::role);
    if(itemcfg.serverLimit > 0 || itemcfg.dayLimit > 0)
    {
        //更新func数据
        updateFuncObjLimit(role->id(), tabId, itemcfg, sellnum);
    }
    else
    {
        if(itemcfg.everyLimit > 0)
            role->addBuyDayLimit(itemcfg.itemId, sellnum);
    }
    LOG_TRACE("商城, 成功出售道具,objId={},objname={},objnum={}, 购买者 roleId={},rolename={},moneytype={},money={}", 
              itemcfg.itemId, base.name, sellnum, role->id(), role->name(), itemcfg.moneytype, needMoney);
}

void Store::sellGoods(Role::Ptr role, const PublicRaw::RoleRequestBuyGoods* rev)
{
    if(0 == rev->goodsNum)
        return;
    if(ObjectConfig::me().objectCfg.m_objBasicDataMap.find(rev->goodsId) == ObjectConfig::me().objectCfg.m_objBasicDataMap.end())
    {
        LOG_ERROR("商城, 道具配置表中没有该道具, objId={}", rev->goodsId);
        return;
    }
    if(tabConfs.find(rev->tabId) == tabConfs.end())
    {
        LOG_ERROR("商城, 购买道具找不到shopId={}, tabId={} 的配置", rev->shopId, rev->tabId);
        return;
    }

    TabConf& tabcfg = tabConfs[rev->tabId];
    if(tabcfg.itemConfs.find(rev->goodsId) == tabcfg.itemConfs.end())
    {
        LOG_ERROR("商城, shopId={}, tabId={} 没有itemId={} 的道具出售", rev->shopId, rev->tabId, rev->goodsId);
        return;
    }

    if(tabcfg.startTime != EPOCH 
       && tabcfg.endTime != EPOCH
       && (Clock::now() < tabcfg.startTime || Clock::now() > tabcfg.endTime))
    {
        retSellGoods(role, tabcfg.moneytype, BuyRetcode::time);
        return;
    }

    const ItemsConf& itemcfg = tabcfg.itemConfs[rev->goodsId];
    if(itemcfg.serverLimit > 0 || itemcfg.dayLimit > 0)
    {
        //向func验证
        requestFuncCheckObjLimit(role->id(), rev->tabId, itemcfg, rev->goodsNum);
    }
    else
    {
        BuyRetcode ret = checkSellGoods(role, itemcfg, rev->goodsNum);
        retSellGoods(role, itemcfg.moneytype, ret);
        if(ret == BuyRetcode::success)
        {
            successSellGoods(role, rev->tabId, itemcfg, rev->goodsNum);
        }
    }
}

bool Store::sellGoods(Role::Ptr role, const uint16_t tabId, const uint32_t goodsId, const uint32_t goodsNum)
{
    if(0 == goodsNum)
        return true;
    if(ObjectConfig::me().objectCfg.m_objBasicDataMap.find(goodsId) == ObjectConfig::me().objectCfg.m_objBasicDataMap.end())
    {
        LOG_ERROR("商城, 道具配置表中没有该道具, objId={}", goodsId);
        return false;
    }
    if(tabConfs.find(tabId) == tabConfs.end())
    {
        LOG_ERROR("商城, 购买道具找不到shopId={}, tabId={} 的配置", m_id, tabId);
        return false;
    }

    TabConf& tabcfg = tabConfs[tabId];
    if(tabcfg.itemConfs.find(goodsId) == tabcfg.itemConfs.end())
    {
        LOG_ERROR("商城, shopId={}, tabId={} 没有itemId={} 的道具出售", m_id, tabId, goodsId);
        return false;
    }

    if(tabcfg.startTime != EPOCH 
       && tabcfg.endTime != EPOCH
       && (Clock::now() < tabcfg.startTime || Clock::now() > tabcfg.endTime))
    {
        retSellGoods(role, tabcfg.moneytype, BuyRetcode::time);
        return false;
    }

    const ItemsConf& itemcfg = tabcfg.itemConfs[goodsId];
    if(itemcfg.serverLimit > 0 || itemcfg.dayLimit > 0)
    {
        //向func验证
        requestFuncCheckObjLimit(role->id(), tabId, itemcfg, goodsNum);
    }
    else
    {
        BuyRetcode ret = checkSellGoods(role, itemcfg, goodsNum);
        retSellGoods(role, itemcfg.moneytype, ret);
        if(ret == BuyRetcode::success)
        {
            successSellGoods(role, tabId, itemcfg, goodsNum);
            return true;
        }
    }
    return false;
}

void Store::sellGoods(Role::Ptr role, const PrivateRaw::RetCheckObjRecord* rev)
{
    TabConf& tabcfg = tabConfs[rev->tabId];
    const ItemsConf& itemcfg = tabcfg.itemConfs[rev->objId];
    if(rev->retcode > 0)
    {
        if(itemcfg.serverLimit > 0)
            retSellGoods(role, itemcfg.moneytype, BuyRetcode::serverLimit);
        else
            retSellGoods(role, itemcfg.moneytype, BuyRetcode::dayLimit);
        return;
    }

    BuyRetcode ret = checkSellGoods(role, itemcfg, rev->num);
    retSellGoods(role, itemcfg.moneytype, ret);
    if(ret == BuyRetcode::success)
    {
        successSellGoods(role, rev->tabId, itemcfg, rev->num);
    }
}

std::vector<uint32_t> Store::getgoodsIdByweight(const uint16_t tabId, const uint16_t num)
{

    std::vector<uint32_t> ret;
    TabConf& tabcfg = tabConfs[tabId];
    if(tabcfg.itemConfs.empty())
    {
        LOG_ERROR("帮派商店, 权重配置出错");
        return ret;
    }

    water::componet::Random<uint16_t> rand((uint16_t)1, tabcfg.addUpWeight);
    uint16_t i = 0;
    while(ret.size() < num)
    {
        uint16_t randNum = rand.get();//得到一个随机数
        uint16_t addWeight = 0;
        for(auto& item : tabcfg.itemConfs)
        {
            addWeight += item.second.weight;
            if(randNum <= addWeight)
            {
                if(std::find(ret.begin(), ret.end(), item.second.itemId) != ret.end())//已经存在,此次随机无效
                {
                    if(ret.size() >= tabcfg.itemConfs.size())    //商店物品不足num个
                        return ret;
                    break;
                }
                ret.push_back(item.second.itemId);  //一次随机得到数据后应结束本次随机
                break;
            }
        }
        i++;
    }
    return ret;
}


//=====================to func====================
void Store::requestFuncCheckObjLimit(RoleId roleId, uint8_t tabId, const ItemsConf& itemcfg, uint16_t goodsNum)
{
    if(goodsNum == 0)
        return;
    PrivateRaw::RequestCheckObjRecord send;
    send.roleId = roleId;
    send.shopId = m_id;
    send.tabId = tabId;
    send.objId = itemcfg.itemId;
    send.type = itemcfg.serverLimit > 0 ? 1 : 0;
    send.num = goodsNum;
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(RequestCheckObjRecord), &send, sizeof(send));
}

void Store::updateFuncObjLimit(RoleId roleId, uint8_t tabId, const ItemsConf& itemcfg, uint16_t goodsNum)
{
    PrivateRaw::AddObjSellNum send;
    send.roleId = roleId;
    send.shopId = m_id;
    send.tabId = tabId;
    send.objId = itemcfg.itemId;
    send.type = itemcfg.serverLimit > 0 ? 1 : 0;
    send.num = goodsNum;
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(AddObjSellNum), &send, sizeof(send));
}


}

