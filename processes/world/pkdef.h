#ifndef PROCESS_WORLD_PKDEF_HPP
#define PROCESS_WORLD_PKDEF_HPP

#include <stdint.h>

enum class SceneItemType : uint8_t
{
    none    = 0,    //非法值
    role    = 1,    //主角
    hero    = 2,    //英雄
    npc     = 3,    //npc
    pet     = 4,    //宠物
    fire    = 5,    //火墙
    trigger = 6,    //机关
    object	= 7,    //物品	
};


enum class attack_mode : uint8_t
{
    none        = 0,
    peace       = 1,    //和平模式
    all         = 2,    //杀戮模式
    team        = 3,    //组队模式
    goodevil    = 4,    //善恶模式
    unions      = 5,    //帮派模式
    consort     = 6,    //夫妻模式
};


enum class PropertyType : uint16_t
{
    none                = 0,
    maxhp               = 1,    //生命
    maxmp               = 2,    //魔法
    addhpRatio          = 3,    //生命加成
    addmpRatio          = 4,    //魔法加成
    hpLv                = 5,    //生命等级
    mpLv                = 6,    //魔法等级
    speed               = 7,    //速度


    p_attackMin         = 8,    //物攻min
    p_attackMax         = 9,    //物攻max
    m_attackMin         = 10,   //魔法min
    m_attackMax         = 11,   //魔法max
    witchMin            = 12,   //道术min
    witchMax            = 13,   //道术max
    p_defenceMin        = 14,   //物防min
    p_defenceMax        = 15,   //物防max
    m_defenceMin        = 16,   //魔防min
    m_defenceMax        = 17,   //魔防max


    lucky               = 18,   //幸运
    evil                = 19,   //诅咒
    shot                = 20,   //命中
    shotRatio           = 21,   //命中率
    p_escape            = 22,   //物闪
    m_escape            = 23,   //魔闪
    escapeRatio         = 24,   //闪避率
    crit                = 25,   //暴击
    critRatio           = 26,   //暴击率
    antiCrit            = 27,   //防暴
    critDamage          = 28,   //暴伤
    

    damageAdd           = 29,   //增伤
    damageReduce        = 30,   //减伤
    damageAddLv         = 31,   //增伤等级
    damageReduceLv      = 32,   //减伤等级

    antiDropEquip       = 33,   //防爆装备属性
};


//合击炼心技能属性
enum class DragonSkillProp : uint8_t
{
    roleSkillPower      = 1,    //技能威力(角色技能生效)
    roleSkillDamage     = 2,    //技能伤害
    rolePveConstDamage  = 3,    //pve固定伤害
    rolepveDamageAdd    = 4,    //pve伤害加成
    rolePvpConstDamage  = 5,    //pvp固定伤害
    rolePvpDamageAdd    = 6,    //pvp伤害加成

    heroSkillPower      = 7,    //技能威力(英雄技能生效)
    heroSkillDamage     = 8,    //技能伤害
    heroPveConstDamage  = 9,    //pve固定伤害
    heropveDamageAdd    = 10,   //pve伤害加成
    heroPvpConstDamage  = 11,   //pvp固定伤害
    heroPvpDamageAdd    = 12,   //pvp伤害加成

    reduceEnergeCost        = 13,     //释放合击技能龙心能量减少数值           
    addEnergeLimit          = 14,     //提升龙心能量上限                        
    extraAnger              = 15,     //怒气增长速度提高值                      
    extendReadyTime       = 16,    //增加合击技能准备时间(delay合击技能释放)
};


enum class CenterType : uint8_t
{
    self        = 1,    //自身
    target      = 2,    //目标
    remote      = 3,    //远方坐标
};


enum class SkillType : uint8_t
{
    active      = 1,    //主动
    passive     = 2,    //被动
};


enum class TargetType : uint8_t
{
    self        = 1,    //自己
    all         = 2,    //所有目标
    friendly    = 3,    //友方
    mob         = 4,    //普通小怪
    Boss        = 5,    //BOSS
    allnpc      = 6,    //所有怪物
    role        = 7,    //玩家、英雄、召唤兽
};


enum class Range : uint8_t
{
    none        = 0,
    one         = 1,    //单体
    circle      = 2,    //圆形
    line        = 3,    //直线
    rect        = 4,    //自身面向矩形框
    order       = 5,    //攻击方向上的第几格(战士刺杀专用)
};


enum class logic_retcode : uint8_t
{
    none    = 0,
    timer   = 1,   //持续技能效果
};


enum class skill_step : uint8_t
{
    start   = 1,
    timer   = 2,
    stop    = 3,
};


enum class visual_status : uint32_t
{
    none        = 0,
    paralysis   = 1,        //麻痹
    fixed       = 2,        //定身
    stun        = 4,        //昏迷
    frozen      = 8,        //冰冻
    subspeed    = 16,       //减速
    poisoned    = 32,       //中毒
    recovery    = 64,       //回血恢复
    shield      = 128,      //魔法盾
    hide        = 256,      //隐身
    treatment   = 512,      //治疗术
    xuanjia     = 1024,     //玄甲术
    wuji        = 2048,     //无极真气
    holdbox     = 4096,     //持有世界boss终极宝箱状态
};


enum class skill_kind : uint8_t
{
    normal      = 1,    //平砍
    regular     = 2,    //正常技能
    assassinate = 3,    //战士刺杀
    chongfeng   = 4,    //战士冲锋
    flame       = 5,    //战士逐日剑法
    shield      = 6,    //法师的魔法盾
    poison      = 7,    //道士的施毒术
    thunder     = 8,    //法师雷电术
    firewall    = 9,    //法师火墙
    ice         = 10,   //法师冰咆哮
    resistfire  = 11,   //法师抗拒火环
    summon      = 12,   //道士召唤术

    joint       = 20,   //合击技能

    //以下为被动技能种类
    passiveBuff     = 100,  //buff类被动技能
    passiveDebuff   = 101,  //debuff类
    passiveRelive   = 102,  //死亡复活类
    passiveDrop     = 103,  //爆装备类
    passiveStatus   = 104,  //pk状态类
    passiveDamage   = 105,  //额外伤害类(有生效类型对象)
};


enum class AttackRetcode : uint8_t
{
    success         = 0,    //成功
    mp              = 1,    //mp不足
    cdtime          = 2,    //cd中
    pubCDTime       = 3,    //公共cd中
    range           = 4,    //超出射程
    invalidTarget   = 5,    //无效目标
    unknown         = 6,    //未知错误
};


enum class HPChangeType : uint16_t
{
    none            = 0,
    normal          = 1,    //普通
    crit            = 2,    //暴击
    reflect         = 3,    //反弹
    vampire         = 4,    //吸血
    breakdef        = 5,    //破甲
    escape          = 6,    //闪避
    recovery        = 7,    //回血
};


struct DamageUnit
{
    HPChangeType type = HPChangeType::none;
    int32_t hp  = 0;//正:加血 负:扣血
};


enum class buff_kind : uint8_t
{
    none        = 0,
    normal      = 1,    //一般buff
    recoveryHP  = 2,    //回血
    recoveryMP  = 3,    //回蓝
};


enum class buff_action : uint8_t
{
    not_cover   = 0,    //不覆盖
    cover       = 1,    //覆盖
};


enum class upgrade_skill : uint8_t
{
    success     = 0,    //成功
    exp         = 1,    //经验值不够(升级专用)
    rolelevel   = 2,    //主角等级不够
    obj         = 3,    //材料不够
    max         = 4,    //满级
    job         = 5,    //职业不对(升级专用)
    skilllevel  = 6,    //技能等级不够(强化专用)
};


enum class name_color : uint8_t
{
    white       = 0,    //名称默认颜色
    yellow      = 1,    //黄色
    red         = 2,    //红色
    grey        = 4,    //灰色
};


//打断操作
enum class interrupt_operate : uint8_t
{
    move        = 1,    //移动
    attack      = 2,    //主动攻击
    beAttack    = 3,    //被攻击
    dead        = 4,    //死亡
    leaveScene  = 5,    //离开场景
};


#endif

