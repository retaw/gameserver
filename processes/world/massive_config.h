#ifndef PROCESS_WORLD_MASSIVE_H
#define PROCESS_WORLD_MASSIVE_H

#include <utility>
#include <stdint.h>
#include <string>

namespace world{

class Massive final
{
public:
    Massive();
    ~Massive()=default;

    static Massive& me();
    void loadConfig(const std::string& cfgdir);

    struct EvilCfg
    {
        uint16_t killAddEvil;   //击杀主角/英雄增加的罪恶值(非正常防伪或者善恶模式)
        uint16_t yellowEvil; //黄名罪恶值区间
        uint16_t redEvil; //红名罪恶值区间
        uint16_t subEvil;   //每分钟系统自动扣除的罪恶值
        uint16_t greynameTime; //灰名持续时间
    };
    EvilCfg m_evilCfg;

    struct DropCfg
    {
        uint16_t normalPackage; //普通背包掉落系数(万分比)
        uint16_t equipPackage;  //装备背包掉落系数
        uint16_t evilParam1; //罪恶值系数1
        uint16_t evilParam2; //罪恶值系数2
        uint16_t constant; //常数
        uint16_t ownerTime;//归属时间
    };
    DropCfg m_dropCfg;

    uint16_t m_publicSkillCD;

    struct FireCfg
    {
        uint16_t lifeTime; //火墙持续时间(秒)
        uint16_t interval; //作用间隔时间(秒)
    };
    FireCfg m_fireCfg;

    struct JointSkillCfg
    {
        uint16_t baseAnger;     //每次增加的基础怒气值
        uint16_t costAnger;     //释放合击技能消耗的怒气
        uint16_t angerLimit;    //怒气上限
        uint16_t energeLimit;   //龙心能量上限
        uint16_t readyTime;     //合击准备时长
        uint16_t dragonSoul;    //每次释放合击技能时获得的龙魂
        uint32_t costObj;       //填充龙心能量消耗的道具
    };
    JointSkillCfg m_jointSkillCfg;

    struct StallCfg
    {
        uint32_t mapId;         //摆摊地图id
        uint32_t stallLevel;    //摆摊所需等级
    };
    StallCfg m_stallCfg;
};

}

#endif

