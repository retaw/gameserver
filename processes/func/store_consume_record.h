/*
 * Author: LiZhaojia
 *
 * Created: 2015-06-17 16:22 +0800
 *
 * Modified: 2015-06-17 16:22 +0800
 *
 * Description: 商城限量道具出售记录
 */

#ifndef PROCESS_FUNC_STORE_CONSUMES_H
#define PROCESS_FUNC_STORE_CONSUMES_H


#include "water/common/roledef.h"
#include <unordered_map>
#include <vector>

namespace func{

class StoreConsumeRecord
{
private:
    StoreConsumeRecord() = default;

    //=================配置开始=======================
    struct ItemConf
    {
        uint32_t itemId;
        uint32_t serverLimit = 0;
        uint32_t dayLimit = 0;   
    };

    struct TabConf
    {
        uint32_t tabId;
        std::vector<ItemConf> limitObj; //<itemId, >
    };

    struct StoreConf
    {
        uint16_t shopId;
        std::unordered_map<uint32_t, TabConf> tabConfs; //<tabId, >
    };

    //只加载有限量道具属性的商店配置
    std::unordered_map<uint32_t, StoreConf> storeConfs; //<shopId, >
    std::unordered_map<uint32_t, ItemConf> dayObjConfs;    //<itemId, >
    std::unordered_map<uint32_t, ItemConf> foreverObjConfs;    //<itemId, >
    //==================配置结束=======================
    
    enum class RecordType : uint8_t
    {
        day     = 0,
        forever = 1,
    };


private:
    //消息处理
    void clientmsg_RoleRequestLimitObjTabNum(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    void servermsg_RequestCheckObjRecord(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    void servermsg_AddObjSellNum(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);


    uint32_t getRecordStoreObj(uint32_t itemId, RecordType type) const;
    void addRecordStoreObj(uint32_t itemId, uint16_t num, RecordType type);


public:
    ~StoreConsumeRecord() = default;

    static StoreConsumeRecord& me();
    void regMsgHandler();
    void loadConfig(const std::string& cfgDir);

};

}

#endif
