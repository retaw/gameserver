#ifndef PROCESS_WORLD_HORSE_BASE_H
#define PROCESS_WORLD_HORSE_BASE_H

#include "pkdef.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <set>

#include "water/common/roledef.h"
#include "water/componet/class_helper.h"

namespace world{

//坐骑培养配置表
struct HorseTrainTpl
{
    TYPEDEF_PTR(HorseTrainTpl)
    CREATE_FUN_NEW(HorseTrainTpl)

    uint8_t star;
    uint16_t skin;    //当前品阶的皮肤
    uint32_t nextexp; //下一阶需要的经验值
    MoneyType moneyType;
    uint64_t money;
    std::set<TplId> costObjs;
    std::vector<std::pair<PropertyType, uint32_t>> roleProps;
    std::vector<std::pair<PropertyType, uint32_t>> heroProps;
};

//坐骑皮肤配置表
struct HorseSkinTpl
{
    TYPEDEF_PTR(HorseSkinTpl)
    CREATE_FUN_NEW(HorseSkinTpl)

    uint16_t skin;
    uint8_t type;
    std::vector<std::pair<PropertyType, uint32_t>> skinProps;
    std::string notice;
};


//坐骑培养暴击配置
struct HorseRateTpl
{
    TYPEDEF_PTR(HorseRateTpl)
    CREATE_FUN_NEW(HorseRateTpl)

    uint8_t rate; //暴击倍率
    uint16_t needRaiseCount; //激活需要培养的次数
    uint8_t effectNum; //激活后可作用的次数
    uint16_t weight; //权重
};


class HorseBase
{
public:
    ~HorseBase() = default;

private:
    HorseBase() = default;

public:
    static HorseBase& me();

    void loadConfig(const std::string& cfgDir);
    HorseTrainTpl::Ptr getTrainTpl(uint8_t star);
    HorseSkinTpl::Ptr getSkinTpl(uint16_t skin);
    HorseRateTpl::Ptr getRateTpl(uint8_t rate);

    void execSkin(std::function<void (HorseSkinTpl::Ptr)> exec);
    void execRate(std::function<void (HorseRateTpl::Ptr)> exec);

private:
    std::unordered_map<uint8_t, HorseTrainTpl::Ptr> m_horseTrainTpls;
    std::unordered_map<uint16_t, HorseSkinTpl::Ptr> m_horseSkinTpls;
    std::unordered_map<uint8_t, HorseRateTpl::Ptr> m_horseRateTpls;
};

}

#endif
