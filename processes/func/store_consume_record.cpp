#include "store_consume_record.h"
#include "role_manager.h"
#include "global_sundry.h"
#include "func.h"

#include "water/componet/datetime.h"
#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/store.h"
#include "protocol/rawmsg/public/store.codedef.public.h"
#include "protocol/rawmsg/private/store.h"
#include "protocol/rawmsg/private/store.codedef.private.h"


namespace func{

using namespace water;
using namespace componet;
using componet::XmlParseDoc;
using componet::XmlParseNode;

StoreConsumeRecord& StoreConsumeRecord::me()
{
    static StoreConsumeRecord me;
    return me;
}

void StoreConsumeRecord::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RoleRequestLimitObjTabNum, std::bind(&StoreConsumeRecord::clientmsg_RoleRequestLimitObjTabNum, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RequestCheckObjRecord, std::bind(&StoreConsumeRecord::servermsg_RequestCheckObjRecord, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(AddObjSellNum, std::bind(&StoreConsumeRecord::servermsg_AddObjSellNum, this, _1, _2, _3));
}


void StoreConsumeRecord::loadConfig(const std::string& cfgDir)
{
    const std::string file = cfgDir + "/shop.xml";                                                 
    XmlParseDoc doc(file);                                                                         
    XmlParseNode root = doc.getRoot();                                                             
    if(!root)                                                                                      
    {                                                                                              
        EXCEPTION(componet::ExceptionBase, file + " parse root node failed");                      
        return;                                                                                    
    }                                                                                              

    for(XmlParseNode shopNode = root.getChild("shop"); shopNode; ++shopNode)                       
    {                                                                                              
        StoreConf storecfg;
        storecfg.shopId = shopNode.getAttr<uint16_t>("id");                                      
        for(XmlParseNode tabNode = shopNode.getChild("tab"); tabNode; ++tabNode)
        {
            TabConf tabcfg;
            tabcfg.tabId = tabNode.getAttr<uint8_t>("id");
            //parse itemNode
            for(XmlParseNode itemNode = tabNode.getChild("item"); itemNode; ++itemNode)
            {
                ItemConf itemcfg;
                itemcfg.itemId = itemNode.getAttr<uint32_t>("id");
                itemcfg.serverLimit = itemNode.getAttr<uint32_t>("server_limit");
                itemcfg.dayLimit = itemNode.getAttr<uint32_t>("day_limit");
                if(0 == itemcfg.serverLimit && 0 == itemcfg.dayLimit)
                    continue;

                if((itemcfg.serverLimit > 0 && foreverObjConfs.find(itemcfg.itemId) != foreverObjConfs.end())
                   || (itemcfg.dayLimit > 0 && dayObjConfs.find(itemcfg.itemId) != dayObjConfs.end()))
                {
                    EXCEPTION(componet::ExceptionBase, "商店限量道具重复, shopId={}, tabId={}, itemId={}", 
                              storecfg.shopId, tabcfg.tabId, itemcfg.itemId);                      
                    return;                                                                                    
                }
                tabcfg.limitObj.push_back(itemcfg);
                if(itemcfg.serverLimit > 0)
                    foreverObjConfs.insert(std::make_pair(itemcfg.itemId, itemcfg));
                else
                    dayObjConfs.insert(std::make_pair(itemcfg.itemId, itemcfg));
            }

            if(tabcfg.limitObj.size() > 0)
                storecfg.tabConfs.insert(std::make_pair(tabcfg.tabId, tabcfg));
        }
        if(storecfg.tabConfs.size() > 0)
            storeConfs.insert(std::make_pair(storecfg.shopId, storecfg));
    }                                                                                              
}


void StoreConsumeRecord::clientmsg_RoleRequestLimitObjTabNum(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto* rev = reinterpret_cast<const PublicRaw::RoleRequestLimitObjTabNum*>(msgData);
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    if(storeConfs.find(rev->shopId) == storeConfs.end())
    {
        LOG_ERROR("商城, 请求限量道具数量, 没有该配置, shopId={}", rev->shopId);
        return;
    }
    if(storeConfs[rev->shopId].tabConfs.find(rev->tabId) == storeConfs[rev->shopId].tabConfs.end())
    {
        LOG_ERROR("商城, 请求限量道具数量, 没有该配置, shopId={},tabId={}", rev->shopId, rev->tabId);
        return;
    }

    std::vector<uint8_t> buf;
    buf.reserve(128);
    buf.resize(sizeof(PublicRaw::RetLimitObjTabNum));
    auto msg = reinterpret_cast<PublicRaw::RetLimitObjTabNum*>(buf.data());
    msg->shopId = rev->shopId;
    msg->tabId = rev->tabId;
    for(const auto& iter : storeConfs[rev->shopId].tabConfs[rev->tabId].limitObj)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetLimitObjTabNum::SellObjInfo));
        msg = reinterpret_cast<PublicRaw::RetLimitObjTabNum*>(buf.data());
        msg->info[msg->size].goodsId = iter.itemId;
        if(iter.serverLimit > 0)
            msg->info[msg->size].goodsNum = SAFE_SUB(iter.serverLimit, getRecordStoreObj(iter.itemId, RecordType::forever));
        else
            msg->info[msg->size].goodsNum = SAFE_SUB(iter.dayLimit, getRecordStoreObj(iter.itemId, RecordType::day));
        ++msg->size;
    }
    if(msg->size > 0)
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetLimitObjTabNum), buf.data(), buf.size());
}


void StoreConsumeRecord::servermsg_RequestCheckObjRecord(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    ProcessIdentity pid(remoteProcessId);
    auto rev = reinterpret_cast<const PrivateRaw::RequestCheckObjRecord*>(msgData);
    if((rev->type == 1 && foreverObjConfs.find(rev->objId) == foreverObjConfs.end())
       || (rev->type == 0 && dayObjConfs.find(rev->objId) == dayObjConfs.end()))
    {
        LOG_ERROR("商城, 购买限量道具 RequestCheckObjRecord, 配置中不存在, itemId={}, type={}", rev->objId, rev->type);
        return;
    }

    uint8_t retcode = 0;
    ItemConf& itemcfg = rev->type == 1 ? foreverObjConfs[rev->objId] : dayObjConfs[rev->objId];
    if(itemcfg.serverLimit > 0 
       && rev->num > SAFE_SUB(itemcfg.serverLimit, getRecordStoreObj(itemcfg.itemId, RecordType::forever)))
        retcode = 1;
    else if(itemcfg.dayLimit > 0 
            && rev->num > SAFE_SUB(itemcfg.dayLimit, getRecordStoreObj(itemcfg.itemId, RecordType::day)))
        retcode = 1;

    PrivateRaw::RetCheckObjRecord ret;
    ret.retcode = retcode;
    ret.roleId = rev->roleId;
    ret.shopId = rev->shopId;
    ret.tabId = rev->tabId;
    ret.objId = rev->objId;
    ret.num = rev->num;
    Func::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetCheckObjRecord), &ret, sizeof(ret));
}

void StoreConsumeRecord::servermsg_AddObjSellNum(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::AddObjSellNum*>(msgData);
    PublicRaw::RefreshObjLimitNum refresh;

    ItemConf& itemcfg = rev->type == 1 ? foreverObjConfs[rev->objId] : dayObjConfs[rev->objId];
    if(itemcfg.serverLimit > 0)
    {
        addRecordStoreObj(rev->objId, rev->num, RecordType::forever);
        refresh.leftnum = SAFE_SUB(itemcfg.serverLimit, getRecordStoreObj(itemcfg.itemId, RecordType::forever));
    }
    else
    {
        addRecordStoreObj(rev->objId, rev->num, RecordType::day);
        refresh.leftnum = SAFE_SUB(itemcfg.dayLimit, getRecordStoreObj(itemcfg.itemId, RecordType::day));
    }

    auto role = RoleManager::me().getById(rev->roleId);
    if(nullptr == role)
        return;

    refresh.shopId = rev->shopId;
    refresh.tabId = rev->tabId;
    refresh.goodsId = rev->objId;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshObjLimitNum), &refresh, sizeof(refresh));
}

/*
 * type: 0,day  1,forever
 */
uint32_t StoreConsumeRecord::getRecordStoreObj(uint32_t itemId, RecordType type) const
{
    auto foreverIt = GlobalSundry::me().m_recordStoreObjForever.find(itemId);
    if(type == RecordType::forever 
       && foreverIt != GlobalSundry::me().m_recordStoreObjForever.end())
    {
        return foreverIt->second.sellNum;
    }

    auto dayIt = GlobalSundry::me().m_recordStoreObjDay.find(itemId);
    if(type == RecordType::day 
       && dayIt != GlobalSundry::me().m_recordStoreObjDay.end())
    {
        StoreRecordObj& obj = dayIt->second;;
        if(!inSameDay(Clock::from_time_t(obj.datetime), Clock::now()))
        {
            obj.sellNum = 0;
            obj.datetime = toUnixTime(Clock::now());
        }
        return obj.sellNum;
    }

    return 0;
}

void StoreConsumeRecord::addRecordStoreObj(uint32_t itemId, uint16_t num, RecordType type)
{
    if(type == RecordType::forever)
    {
        auto foreverIt = GlobalSundry::me().m_recordStoreObjForever.find(itemId);
        if(foreverIt == GlobalSundry::me().m_recordStoreObjForever.end())
        {
            StoreRecordObj obj;
            obj.objId = itemId;
            obj.sellNum = num;
            obj.type = static_cast<uint8_t>(type);
            GlobalSundry::me().m_recordStoreObjForever.insert(std::make_pair(itemId, obj));
        }
        else
        {
            foreverIt->second.sellNum += num;
        }
        return;
    }

    auto dayIt = GlobalSundry::me().m_recordStoreObjDay.find(itemId);
    if(dayIt == GlobalSundry::me().m_recordStoreObjDay.end())
    {
        StoreRecordObj obj;
        obj.objId = itemId;
        obj.sellNum = num;
        obj.type = static_cast<uint8_t>(type);
        obj.datetime = toUnixTime(Clock::now());
        GlobalSundry::me().m_recordStoreObjDay.insert(std::make_pair(itemId, obj));
    }
    else
    {
        dayIt->second.sellNum += num;
    }
}


}

