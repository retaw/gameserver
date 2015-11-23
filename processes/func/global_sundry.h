/*
 * Description: 全局杂项数据保存
 */
#ifndef PROCESS_FUNC_GLOBAL_SUNDRY_H
#define PROCESS_FUNC_GLOBAL_SUNDRY_H

#include <string>
#include <unordered_map>

namespace func{

#pragma pack(1)

//商城限量道具出售记录结构
struct StoreRecordObj
{
    uint32_t    objId;
    uint32_t    sellNum; //已出售的数量
    uint8_t     type;    //1:永久保存 0:只保存当天的
    uint32_t    datetime;//最后一次出售的时间点
};

#pragma pack()

class GlobalSundry
{
private:
    GlobalSundry();

public:
    ~GlobalSundry() = default;

    static GlobalSundry& me();
    void regTimer();

    void loadDB();
    void saveDB();

private:
    void timerLoop();

    void serializeData(std::string& saveStr);
    void deserializeData(const std::string& loadStr);

public:
    //定义自己需要保存的数据
    uint16_t m_worldBossLv = 0;     //下次世界boss活动开启世界boss等级
    std::unordered_map<uint32_t, StoreRecordObj> m_recordStoreObjForever; //<itemId, > 永久保存系列
    std::unordered_map<uint32_t, StoreRecordObj> m_recordStoreObjDay;     //<itemId, > 按日清除系列
};


}

#endif

