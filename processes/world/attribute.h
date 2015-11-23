 /*
 * Description: 属性数据接口
 */

#ifndef PROCESS_WORLD_ATTRIBUTE_H
#define PROCESS_WORLD_ATTRIBUTE_H

#include "pkdef.h"

#include <stdint.h>
#include <string.h>
#include <vector>

namespace world{


#define DEFINE_SET_ATTRIBUTE(var, value) \
    void set_##var(int32_t value)\
    {\
        var = value;\
    }

#define DEFINE_GET_ATTRIBUTE(var) \
    int32_t get_##var() const\
    {\
       return var;\
    } 

#define DEFINE_ADD_ATTRIBUTE(var, value) \
    void add_##var(int32_t value) \
    {\
        var += value;\
    }


#define SET_ATTRIBUTE(var, value) set_##var(value)
#define GET_ATTRIBUTE(var) get_##var()
#define ADD_ATTRIBUTE(var, value) add_##var(value)

class Attribute
{
public:
    Attribute();
    virtual ~Attribute() = default;


public:
    DEFINE_SET_ATTRIBUTE(maxhp, val);
    DEFINE_SET_ATTRIBUTE(maxmp, val);
    DEFINE_SET_ATTRIBUTE(addhpRatio, val);
    DEFINE_SET_ATTRIBUTE(addmpRatio, val);
    DEFINE_SET_ATTRIBUTE(hpLv, val);
    DEFINE_SET_ATTRIBUTE(mpLv, val);
    DEFINE_SET_ATTRIBUTE(speed, val);

    DEFINE_SET_ATTRIBUTE(p_attackMin, val);
    DEFINE_SET_ATTRIBUTE(p_attackMax, val);
    DEFINE_SET_ATTRIBUTE(m_attackMin, val);
    DEFINE_SET_ATTRIBUTE(m_attackMax, val);
    DEFINE_SET_ATTRIBUTE(witchMin, val);
    DEFINE_SET_ATTRIBUTE(witchMax, val);
    DEFINE_SET_ATTRIBUTE(p_defenceMin, val);
    DEFINE_SET_ATTRIBUTE(p_defenceMax, val);
    DEFINE_SET_ATTRIBUTE(m_defenceMin, val);
    DEFINE_SET_ATTRIBUTE(m_defenceMax, val);
    DEFINE_SET_ATTRIBUTE(lucky, val);
    DEFINE_SET_ATTRIBUTE(evil, val);

    DEFINE_SET_ATTRIBUTE(shot, val);
    DEFINE_SET_ATTRIBUTE(shotRatio, val);
    DEFINE_SET_ATTRIBUTE(p_escape, val);
    DEFINE_SET_ATTRIBUTE(m_escape, val);
    DEFINE_SET_ATTRIBUTE(escapeRatio, val);
    DEFINE_SET_ATTRIBUTE(crit, val);
    DEFINE_SET_ATTRIBUTE(critRatio, val);
    DEFINE_SET_ATTRIBUTE(antiCrit, val);
    DEFINE_SET_ATTRIBUTE(critDamage, val);

    DEFINE_SET_ATTRIBUTE(damageAdd, val);
    DEFINE_SET_ATTRIBUTE(damageReduce, val);
    DEFINE_SET_ATTRIBUTE(damageAddLv, val);
    DEFINE_SET_ATTRIBUTE(damageReduceLv, val);
    DEFINE_SET_ATTRIBUTE(antiDropEquip, val);


public:
    DEFINE_ADD_ATTRIBUTE(maxhp, val);
    DEFINE_ADD_ATTRIBUTE(maxmp, val);
    DEFINE_ADD_ATTRIBUTE(addhpRatio, val);
    DEFINE_ADD_ATTRIBUTE(addmpRatio, val);
    DEFINE_ADD_ATTRIBUTE(hpLv, val);
    DEFINE_ADD_ATTRIBUTE(mpLv, val);
    DEFINE_ADD_ATTRIBUTE(speed, val);

    DEFINE_ADD_ATTRIBUTE(p_attackMin, val);
    DEFINE_ADD_ATTRIBUTE(p_attackMax, val);
    DEFINE_ADD_ATTRIBUTE(m_attackMin, val);
    DEFINE_ADD_ATTRIBUTE(m_attackMax, val);
    DEFINE_ADD_ATTRIBUTE(witchMin, val);
    DEFINE_ADD_ATTRIBUTE(witchMax, val);
    DEFINE_ADD_ATTRIBUTE(p_defenceMin, val);
    DEFINE_ADD_ATTRIBUTE(p_defenceMax, val);
    DEFINE_ADD_ATTRIBUTE(m_defenceMin, val);
    DEFINE_ADD_ATTRIBUTE(m_defenceMax, val);
    DEFINE_ADD_ATTRIBUTE(lucky, val);
    DEFINE_ADD_ATTRIBUTE(evil, val);

    DEFINE_ADD_ATTRIBUTE(shot, val);
    DEFINE_ADD_ATTRIBUTE(shotRatio, val);
    DEFINE_ADD_ATTRIBUTE(p_escape, val);
    DEFINE_ADD_ATTRIBUTE(m_escape, val);
    DEFINE_ADD_ATTRIBUTE(escapeRatio, val);
    DEFINE_ADD_ATTRIBUTE(crit, val);
    DEFINE_ADD_ATTRIBUTE(critRatio, val);
    DEFINE_ADD_ATTRIBUTE(antiCrit, val);
    DEFINE_ADD_ATTRIBUTE(critDamage, val);

    DEFINE_ADD_ATTRIBUTE(damageAdd, val);
    DEFINE_ADD_ATTRIBUTE(damageReduce, val);
    DEFINE_ADD_ATTRIBUTE(damageAddLv, val);
    DEFINE_ADD_ATTRIBUTE(damageReduceLv, val);
    DEFINE_ADD_ATTRIBUTE(antiDropEquip, val);


public:
    DEFINE_GET_ATTRIBUTE(maxhp);
    DEFINE_GET_ATTRIBUTE(maxmp);
    DEFINE_GET_ATTRIBUTE(addhpRatio);
    DEFINE_GET_ATTRIBUTE(addmpRatio);
    DEFINE_GET_ATTRIBUTE(hpLv);
    DEFINE_GET_ATTRIBUTE(mpLv);
    DEFINE_GET_ATTRIBUTE(speed);

    DEFINE_GET_ATTRIBUTE(p_attackMin);
    DEFINE_GET_ATTRIBUTE(p_attackMax);
    DEFINE_GET_ATTRIBUTE(m_attackMin);
    DEFINE_GET_ATTRIBUTE(m_attackMax);
    DEFINE_GET_ATTRIBUTE(witchMin);
    DEFINE_GET_ATTRIBUTE(witchMax);
    DEFINE_GET_ATTRIBUTE(p_defenceMin);
    DEFINE_GET_ATTRIBUTE(p_defenceMax);
    DEFINE_GET_ATTRIBUTE(m_defenceMin);
    DEFINE_GET_ATTRIBUTE(m_defenceMax);
    DEFINE_GET_ATTRIBUTE(lucky);
    DEFINE_GET_ATTRIBUTE(evil);

    DEFINE_GET_ATTRIBUTE(shot);
    DEFINE_GET_ATTRIBUTE(shotRatio);
    DEFINE_GET_ATTRIBUTE(p_escape);
    DEFINE_GET_ATTRIBUTE(m_escape);
    DEFINE_GET_ATTRIBUTE(escapeRatio);
    DEFINE_GET_ATTRIBUTE(crit);
    DEFINE_GET_ATTRIBUTE(critRatio);
    DEFINE_GET_ATTRIBUTE(antiCrit);
    DEFINE_GET_ATTRIBUTE(critDamage);

    DEFINE_GET_ATTRIBUTE(damageAdd);
    DEFINE_GET_ATTRIBUTE(damageReduce);
    DEFINE_GET_ATTRIBUTE(damageAddLv);
    DEFINE_GET_ATTRIBUTE(damageReduceLv);
    DEFINE_GET_ATTRIBUTE(antiDropEquip);


public:
    void reset();
    virtual void setAttribute(const std::vector<std::pair<PropertyType, uint32_t> >& propertys);
    virtual void setAttribute(const std::vector<std::pair<PropertyType, int32_t> >& propertys);
    virtual void addAttribute(const std::vector<std::pair<PropertyType, uint32_t> >& propertys);
    virtual void addAttribute(const std::vector<std::pair<PropertyType, int32_t> >& propertys);



private:
    int32_t maxhp = 0;                      //生命
    int32_t maxmp = 0;                      //魔法
    int32_t addhpRatio = 0;                 //生命加成
    int32_t addmpRatio = 0;                 //魔法加成
    int32_t hpLv = 0;                       //生命等级
    int32_t mpLv = 0;                       //魔法等级
    int32_t speed = 0;                      //速度


    int32_t p_attackMin = 0;                //物攻min
    int32_t p_attackMax = 0;                //物攻max
    int32_t m_attackMin = 0;                //魔法min
    int32_t m_attackMax = 0;                //魔法max
    int32_t witchMin = 0;                   //道术min
    int32_t witchMax = 0;                   //道术max
    int32_t p_defenceMin = 0;               //物防min
    int32_t p_defenceMax = 0;               //物防max
    int32_t m_defenceMin = 0;               //魔防min
    int32_t m_defenceMax = 0;               //魔防max
    int32_t lucky = 0;                      //幸运
    int32_t evil = 0;                       //诅咒


    int32_t shot = 0;                       //命中
    int32_t shotRatio = 0;                  //命中率
    int32_t p_escape = 0;                   //物闪
    int32_t m_escape = 0;                   //魔闪
    int32_t escapeRatio = 0;                //闪避率
    int32_t crit = 0;                       //暴击
    int32_t critRatio = 0;                  //暴击率
    int32_t antiCrit = 0;                   //防暴
    int32_t critDamage = 0;                 //暴伤
    

    int32_t damageAdd = 0;                  //增伤
    int32_t damageReduce = 0;               //减伤
    int32_t damageAddLv = 0;                //增伤等级
    int32_t damageReduceLv = 0;             //减伤等级

    int32_t antiDropEquip = 0;              //防爆装备属性
};



}

#endif

