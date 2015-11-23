/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-14 11:00 +0800
 *
 * Modified: 2015-05-14 11:00 +0800
 *
 * Description: npc基本定义
 */

#ifndef WORLD_NPC_CFG_H

#include "pkdef.h"

#include "water/common/commdef.h"
#include "water/common/roledef.h"
#include "water/componet/class_helper.h"
#include "water/componet/datetime.h"

#include <string>
#include <vector>

namespace world{

enum class NpcType : uint8_t
{
    npc     = 1, //功能NPC
    mob     = 2, //普通小怪
    elite   = 3, //精英怪
    boss    = 4, //boss
    fight   = 5, //战斗npc
    collect = 6, //采集物
    bigboss = 7, //大boss(召唤出来有一定得占地面积)
    belong  = 8, //有归属的npc
};

typedef TplId NpcTplId;

struct NpcTpl
{
    TYPEDEF_PTR(NpcTpl)

    struct ObjDropInfo
    {
        uint32_t objId;
        uint16_t num;
        Bind isOwnByOne;
        uint32_t probability;
    };
    bool expired = false;

    NpcTplId id;
    std::string name;
    NpcType type;
    Job job;
    uint32_t level;
    uint64_t roleExp;
    uint64_t heroExp;
    uint32_t aiTplId;
    uint16_t keepCorpseSec;
    uint16_t reliveSec;
    uint16_t timeOfBelongTo;
    std::vector<ObjDropInfo> objDropInfos;
    std::vector<std::pair<PropertyType, uint32_t> > props; //基础属性值
    std::chrono::milliseconds stepCost;
    std::chrono::milliseconds stopCost;
    uint32_t skillTplId;
    uint16_t extend; //扩展字段
    uint32_t rewardRandomId;
};





}


#endif 
