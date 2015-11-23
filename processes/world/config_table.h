/*
 *
 * 配置表数据结构定义在此
 *
 */

#ifndef PROCESS_WORLD_CONFIG_TABLE_HPP
#define PROCESS_WORLD_CONFIG_TABLE_HPP

#include "pkdef.h"

#include "water/common/roledef.h"
#include "water/componet/xmlparse.h"
#include "water/componet/class_helper.h" 

#include <string.h>
#include <vector>
#include <stdint.h>
#include <set>

namespace world{

using namespace water;
using componet::XmlParseNode;

#define skill_hash(id, level) (id*100 + level)
/*
 * 技能表配置
 */
struct SkillBase
{
    SkillBase()
    {
        memset(this, 0, sizeof(*this));
    }
    TYPEDEF_PTR(SkillBase)

    TplId       id;
    char        name[NAME_BUFF_SZIE];
    uint32_t    level;          //技能等级
    std::pair<Job, Job> jobs; //职业
    SceneItemType     sceneItem;        //1:主角 2:英雄
    uint8_t     injury;         //是否伤害
    uint8_t     hit;            //1:必中 0:不必中
    skill_kind  kind;           //1:平砍 2:正常技能
    SkillType   type;           //技能类型(主动 被动)
    uint32_t    trigger_per;    //被动技能触发概率(权重)
    uint32_t    cdtime;         //冷却时间(秒)
    uint16_t    min_distance;   //施放技能最小距离
    uint16_t    max_distance;   //施放技能最远距离
    uint16_t    hero_min_distance; //释放合击技能英雄距离目标最小范围
    uint16_t    hero_max_distance; //释放合击技能英雄距离目标最大范围
    CenterType  center_type;    //技能释放中心点(1:自己为中心 2:目标 3:坐标点)
    uint32_t    costmp;         //法力消耗
    uint32_t    add_skillexp;   //每次施放技能时增加的技能经验
    std::vector<std::pair<uint32_t, uint32_t> > effects;    //效果列表

    //升级相关
    uint32_t    role_level;     //升到该级需要的角色等级
    uint32_t    needexp;        //升到该等级需要的技能经验
    std::vector<std::pair<uint32_t, uint32_t> > consumes;

    //返回主键id
    uint32_t parse(const XmlParseNode& node);
};



#define skill_strengthen_hash(id, level) (id*100 + level)
/*
 * 技能强化表
 */
struct SkillStrengthenBase
{
    SkillStrengthenBase()
    {
        memset(this, 0, sizeof(*this));
    }
    TYPEDEF_PTR(SkillStrengthenBase)

    TplId       skill_id;
    uint32_t    strengthen_level;   //强化等级
    uint32_t    role_level;         //主角等级限制
    uint32_t    skill_level;        //技能等级限制
    std::vector<std::pair<uint32_t, uint32_t> > effects;
    std::vector<std::pair<uint32_t, uint32_t> > consumes; //消耗资源

    uint32_t parse(const XmlParseNode& node);
};


#define skill_effect_hash(id, level) (id*100 + level)
/*
 * 技能效果表
 */
struct SkillEffectBase
{
    SkillEffectBase()
    {
        memset(this, 0, sizeof(*this));
    }
    TYPEDEF_PTR(SkillEffectBase)

    uint32_t    effectid;
    uint32_t    level;
    uint32_t    trigger_per;    //触发概率
    TargetType  target_type;    //生效目标类型(1:所有目标 2:友方 3:小怪 4:BOSS 5:自己 6:召唤物)
    uint32_t    target_num;     //范围内目标数量上限
    Range     range_type;     //范围类型

    union
    {
        uint16_t range_param1;  //范围参数1(1:距离 2:半径 3:第几格)
        uint16_t distance;
        uint16_t radius;
        uint16_t order;
    };

    union
    {
        uint16_t range_param2;  //范围参数1(1:弧度)
        uint16_t radian;
        uint16_t width;
    };

    uint32_t    logic_id;       //逻辑库id
    uint32_t    param1;
    uint32_t    param2;
    uint32_t    param3;
    uint32_t    param4;


    uint32_t parse(const XmlParseNode& node);
};


/*
 * buff
 */
struct BuffBase
{
    BuffBase()
    {
        memset(this, 0, sizeof(*this));
    }
    TYPEDEF_PTR(BuffBase)

    TplId       buffid;
    uint16_t    group;              //buff组
    uint32_t    status_id;          //状态效果id
    buff_action action_type;        //生效方式(同组buff生效0:不覆盖,1:覆盖)
    uint8_t     priority;           //同组buff优先级
    uint8_t     time_merge;         //时间叠加(只对同一buffid有效0:不叠加,1:叠加)
    uint8_t     die_exist;          //死后是否保留(0:不保留 1:保留)
    uint8_t     offline_exist;      //下线是否保留(0:不保留 1:保留)
    uint32_t    dur;                //耐久度(剩余作用次数)
    uint8_t     dur_type;           //耐久度激活方式
    uint32_t    interval;           //作用间隔时间
    uint32_t    sec;                //作用时间(秒)
    uint32_t    endtime;            //到期时间
    uint32_t    recovery_hp;        //生命固定恢复
    uint16_t    recovery_hp_percent;//生命上限百分比恢复
    uint32_t    recovery_mp;        //法力固定恢复
    uint16_t    recovery_mp_percent;//法力上限百分比恢复
    uint32_t    damage_hp;          //生命固定伤害
    uint16_t    damage_hp_percent;  //生命上限百分比伤害
    uint32_t    damage_mp;          //法力固定伤害
    uint16_t    damage_mp_percent;  //法力上限百分比伤害
    uint16_t    role_exp_percent;   //角色打怪经验加成
    uint16_t    hero_exp_percent;   //英雄打怪经验加成
    std::vector<std::pair<PropertyType, int32_t> > props; //固定数值属性
    std::vector<std::pair<PropertyType, int32_t> > percent_props; //固定百分比属性


    uint32_t parse(const XmlParseNode& node);
};


/*
 * 主角英雄等级属性表
 */
#define rlp_hash(job, type, level) (job*1000000+type*10000+level)
struct LevelPropsBase
{
    LevelPropsBase()
    {
        memset(this, 0, sizeof(*this));
    }
    TYPEDEF_PTR(LevelPropsBase)

    uint32_t    level;      //等级
    uint32_t    job;        //职业
    uint32_t    type;       //1主角 2英雄
    std::vector<std::pair<PropertyType, uint32_t> > level_props;

    uint32_t parse(const XmlParseNode& node);
};


/*
 * 等级K值表
 */
struct KValueBase
{
    KValueBase()
    {
        memset(this, 0, sizeof(*this));
    }
    TYPEDEF_PTR(KValueBase)

    uint32_t level;
    uint32_t shotk;     //命中k
    uint32_t critk;     //暴击k

    uint32_t parse(const XmlParseNode& node);
};


}

#endif

