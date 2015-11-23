#include "attribute.h"

namespace world{

Attribute::Attribute()
{
}

void Attribute::reset()
{
    maxhp = 0;
    maxmp = 0;
    addhpRatio = 0;
    addmpRatio = 0;
    hpLv = 0;
    mpLv = 0;
    speed = 0;

    p_attackMin = 0;
    p_attackMax = 0;
    m_attackMin = 0;
    m_attackMax = 0;               
    witchMin = 0;                  
    witchMax = 0;                  
    p_defenceMin = 0;              
    p_defenceMax = 0;              
    m_defenceMin = 0;              
    m_defenceMax = 0;              
    lucky = 0;                     
    evil = 0;                      

    shot = 0;                      
    shotRatio = 0;                 
    p_escape = 0;                  
    m_escape = 0;                  
    escapeRatio = 0;               
    crit = 0;                      
    critRatio = 0;                 
    antiCrit = 0;                  
    critDamage = 0;                
    
    damageAdd = 0;                 
    damageReduce = 0;              
    damageAddLv = 0;               
    damageReduceLv = 0;            

    antiDropEquip = 0;             
}

void Attribute::setAttribute(const std::vector<std::pair<PropertyType, uint32_t> >& propertys)
{
    for(const auto& iter : propertys)
    {
        switch(iter.first)
        {
        case PropertyType::maxhp:
            SET_ATTRIBUTE(maxhp, iter.second);
            break;
        case PropertyType::maxmp:
            SET_ATTRIBUTE(maxmp, iter.second);
            break;
        case PropertyType::addhpRatio:
            SET_ATTRIBUTE(addhpRatio, iter.second);
            break;
        case PropertyType::addmpRatio:
            SET_ATTRIBUTE(addmpRatio, iter.second);
            break;
        case PropertyType::hpLv:
            SET_ATTRIBUTE(hpLv, iter.second);
            break;
        case PropertyType::mpLv:
            SET_ATTRIBUTE(mpLv, iter.second);
            break;
        case PropertyType::speed:
            SET_ATTRIBUTE(speed, iter.second);
            break;
        case PropertyType::p_attackMin:
            SET_ATTRIBUTE(p_attackMin, iter.second);
            break;
        case PropertyType::p_attackMax:
            SET_ATTRIBUTE(p_attackMax, iter.second);
            break;
        case PropertyType::m_attackMin:
            SET_ATTRIBUTE(m_attackMin, iter.second);
            break;
        case PropertyType::m_attackMax:
            SET_ATTRIBUTE(m_attackMax, iter.second);
            break;
        case PropertyType::witchMin:
            SET_ATTRIBUTE(witchMin, iter.second);
            break;
        case PropertyType::witchMax:
            SET_ATTRIBUTE(witchMax, iter.second);
            break;
        case PropertyType::p_defenceMin:
            SET_ATTRIBUTE(p_defenceMin, iter.second);
            break;
        case PropertyType::p_defenceMax:
            SET_ATTRIBUTE(p_defenceMax, iter.second);
            break;
        case PropertyType::m_defenceMin:
            SET_ATTRIBUTE(m_defenceMin, iter.second);
            break;
        case PropertyType::m_defenceMax:
            SET_ATTRIBUTE(m_defenceMax, iter.second);
            break;
        case PropertyType::lucky:
            SET_ATTRIBUTE(lucky, iter.second);
            break;
        case PropertyType::evil:
            SET_ATTRIBUTE(evil, iter.second);
            break;
        case PropertyType::shot:
            SET_ATTRIBUTE(shot, iter.second);
            break;
        case PropertyType::shotRatio:
            SET_ATTRIBUTE(shotRatio, iter.second);
            break;
        case PropertyType::p_escape:
            SET_ATTRIBUTE(p_escape, iter.second);
            break;
        case PropertyType::m_escape:
            SET_ATTRIBUTE(m_escape, iter.second);
            break;
        case PropertyType::escapeRatio:
            SET_ATTRIBUTE(escapeRatio, iter.second);
            break;
        case PropertyType::crit:
            SET_ATTRIBUTE(crit, iter.second);
            break;
        case PropertyType::critRatio:
            SET_ATTRIBUTE(critRatio, iter.second);
            break;
        case PropertyType::antiCrit:
            SET_ATTRIBUTE(antiCrit, iter.second);
            break;
        case PropertyType::critDamage:
            SET_ATTRIBUTE(critDamage, iter.second);
            break;
        case PropertyType::damageAdd:
            SET_ATTRIBUTE(damageAdd, iter.second);
            break;
        case PropertyType::damageReduce:
            SET_ATTRIBUTE(damageReduce, iter.second);
            break;
        case PropertyType::damageAddLv:
            SET_ATTRIBUTE(damageAddLv, iter.second);
            break;
        case PropertyType::damageReduceLv:
            SET_ATTRIBUTE(damageReduceLv, iter.second);
            break;
        case PropertyType::antiDropEquip:
            SET_ATTRIBUTE(antiDropEquip, iter.second);
        default:
            break;
        }
    }
}

void Attribute::setAttribute(const std::vector<std::pair<PropertyType, int32_t> >& propertys)
{
    for(const auto& iter : propertys)
    {
        switch(iter.first)
        {
        case PropertyType::maxhp:
            SET_ATTRIBUTE(maxhp, iter.second);
            break;
        case PropertyType::maxmp:
            SET_ATTRIBUTE(maxmp, iter.second);
            break;
        case PropertyType::addhpRatio:
            SET_ATTRIBUTE(addhpRatio, iter.second);
            break;
        case PropertyType::addmpRatio:
            SET_ATTRIBUTE(addmpRatio, iter.second);
            break;
        case PropertyType::hpLv:
            SET_ATTRIBUTE(hpLv, iter.second);
            break;
        case PropertyType::mpLv:
            SET_ATTRIBUTE(mpLv, iter.second);
            break;
        case PropertyType::speed:
            SET_ATTRIBUTE(speed, iter.second);
            break;
        case PropertyType::p_attackMin:
            SET_ATTRIBUTE(p_attackMin, iter.second);
            break;
        case PropertyType::p_attackMax:
            SET_ATTRIBUTE(p_attackMax, iter.second);
            break;
        case PropertyType::m_attackMin:
            SET_ATTRIBUTE(m_attackMin, iter.second);
            break;
        case PropertyType::m_attackMax:
            SET_ATTRIBUTE(m_attackMax, iter.second);
            break;
        case PropertyType::witchMin:
            SET_ATTRIBUTE(witchMin, iter.second);
            break;
        case PropertyType::witchMax:
            SET_ATTRIBUTE(witchMax, iter.second);
            break;
        case PropertyType::p_defenceMin:
            SET_ATTRIBUTE(p_defenceMin, iter.second);
            break;
        case PropertyType::p_defenceMax:
            SET_ATTRIBUTE(p_defenceMax, iter.second);
            break;
        case PropertyType::m_defenceMin:
            SET_ATTRIBUTE(m_defenceMin, iter.second);
            break;
        case PropertyType::m_defenceMax:
            SET_ATTRIBUTE(m_defenceMax, iter.second);
            break;
        case PropertyType::lucky:
            SET_ATTRIBUTE(lucky, iter.second);
            break;
        case PropertyType::evil:
            SET_ATTRIBUTE(evil, iter.second);
            break;
        case PropertyType::shot:
            SET_ATTRIBUTE(shot, iter.second);
            break;
        case PropertyType::shotRatio:
            SET_ATTRIBUTE(shotRatio, iter.second);
            break;
        case PropertyType::p_escape:
            SET_ATTRIBUTE(p_escape, iter.second);
            break;
        case PropertyType::m_escape:
            SET_ATTRIBUTE(m_escape, iter.second);
            break;
        case PropertyType::escapeRatio:
            SET_ATTRIBUTE(escapeRatio, iter.second);
            break;
        case PropertyType::crit:
            SET_ATTRIBUTE(crit, iter.second);
            break;
        case PropertyType::critRatio:
            SET_ATTRIBUTE(critRatio, iter.second);
            break;
        case PropertyType::antiCrit:
            SET_ATTRIBUTE(antiCrit, iter.second);
            break;
        case PropertyType::critDamage:
            SET_ATTRIBUTE(critDamage, iter.second);
            break;
        case PropertyType::damageAdd:
            SET_ATTRIBUTE(damageAdd, iter.second);
            break;
        case PropertyType::damageReduce:
            SET_ATTRIBUTE(damageReduce, iter.second);
            break;
        case PropertyType::damageAddLv:
            SET_ATTRIBUTE(damageAddLv, iter.second);
            break;
        case PropertyType::damageReduceLv:
            SET_ATTRIBUTE(damageReduceLv, iter.second);
            break;
        case PropertyType::antiDropEquip:
            SET_ATTRIBUTE(antiDropEquip, iter.second);
        default:
            break;
        }
    }
}

void Attribute::addAttribute(const std::vector<std::pair<PropertyType, uint32_t> >& propertys)
{
    for(const auto& iter : propertys)
    {
        switch(iter.first)
        {
        case PropertyType::maxhp:
            ADD_ATTRIBUTE(maxhp, iter.second);
            break;
        case PropertyType::maxmp:
            ADD_ATTRIBUTE(maxmp, iter.second);
            break;
        case PropertyType::addhpRatio:
            ADD_ATTRIBUTE(addhpRatio, iter.second);
            break;
        case PropertyType::addmpRatio:
            ADD_ATTRIBUTE(addmpRatio, iter.second);
            break;
        case PropertyType::hpLv:
            ADD_ATTRIBUTE(hpLv, iter.second);
            break;
        case PropertyType::mpLv:
            ADD_ATTRIBUTE(mpLv, iter.second);
            break;
        case PropertyType::speed:
            ADD_ATTRIBUTE(speed, iter.second);
            break;
        case PropertyType::p_attackMin:
            ADD_ATTRIBUTE(p_attackMin, iter.second);
            break;
        case PropertyType::p_attackMax:
            ADD_ATTRIBUTE(p_attackMax, iter.second);
            break;
        case PropertyType::m_attackMin:
            ADD_ATTRIBUTE(m_attackMin, iter.second);
            break;
        case PropertyType::m_attackMax:
            ADD_ATTRIBUTE(m_attackMax, iter.second);
            break;
        case PropertyType::witchMin:
            ADD_ATTRIBUTE(witchMin, iter.second);
            break;
        case PropertyType::witchMax:
            ADD_ATTRIBUTE(witchMax, iter.second);
            break;
        case PropertyType::p_defenceMin:
            ADD_ATTRIBUTE(p_defenceMin, iter.second);
            break;
        case PropertyType::p_defenceMax:
            ADD_ATTRIBUTE(p_defenceMax, iter.second);
            break;
        case PropertyType::m_defenceMin:
            ADD_ATTRIBUTE(m_defenceMin, iter.second);
            break;
        case PropertyType::m_defenceMax:
            ADD_ATTRIBUTE(m_defenceMax, iter.second);
            break;
        case PropertyType::lucky:
            ADD_ATTRIBUTE(lucky, iter.second);
            break;
        case PropertyType::evil:
            ADD_ATTRIBUTE(evil, iter.second);
            break;
        case PropertyType::shot:
            ADD_ATTRIBUTE(shot, iter.second);
            break;
        case PropertyType::shotRatio:
            ADD_ATTRIBUTE(shotRatio, iter.second);
            break;
        case PropertyType::p_escape:
            ADD_ATTRIBUTE(p_escape, iter.second);
            break;
        case PropertyType::m_escape:
            ADD_ATTRIBUTE(m_escape, iter.second);
            break;
        case PropertyType::escapeRatio:
            ADD_ATTRIBUTE(escapeRatio, iter.second);
            break;
        case PropertyType::crit:
            ADD_ATTRIBUTE(crit, iter.second);
            break;
        case PropertyType::critRatio:
            ADD_ATTRIBUTE(critRatio, iter.second);
            break;
        case PropertyType::antiCrit:
            ADD_ATTRIBUTE(antiCrit, iter.second);
            break;
        case PropertyType::critDamage:
            ADD_ATTRIBUTE(critDamage, iter.second);
            break;
        case PropertyType::damageAdd:
            ADD_ATTRIBUTE(damageAdd, iter.second);
            break;
        case PropertyType::damageReduce:
            ADD_ATTRIBUTE(damageReduce, iter.second);
            break;
        case PropertyType::damageAddLv:
            ADD_ATTRIBUTE(damageAddLv, iter.second);
            break;
        case PropertyType::damageReduceLv:
            ADD_ATTRIBUTE(damageReduceLv, iter.second);
            break;
        case PropertyType::antiDropEquip:
            ADD_ATTRIBUTE(antiDropEquip, iter.second);
        default:
            break;
        }
    }
}

void Attribute::addAttribute(const std::vector<std::pair<PropertyType, int32_t> >& propertys)
{
    for(const auto& iter : propertys)
    {
        switch(iter.first)
        {
        case PropertyType::maxhp:
            ADD_ATTRIBUTE(maxhp, iter.second);
            break;
        case PropertyType::maxmp:
            ADD_ATTRIBUTE(maxmp, iter.second);
            break;
        case PropertyType::addhpRatio:
            ADD_ATTRIBUTE(addhpRatio, iter.second);
            break;
        case PropertyType::addmpRatio:
            ADD_ATTRIBUTE(addmpRatio, iter.second);
            break;
        case PropertyType::hpLv:
            ADD_ATTRIBUTE(hpLv, iter.second);
            break;
        case PropertyType::mpLv:
            ADD_ATTRIBUTE(mpLv, iter.second);
            break;
        case PropertyType::speed:
            ADD_ATTRIBUTE(speed, iter.second);
            break;
        case PropertyType::p_attackMin:
            ADD_ATTRIBUTE(p_attackMin, iter.second);
            break;
        case PropertyType::p_attackMax:
            ADD_ATTRIBUTE(p_attackMax, iter.second);
            break;
        case PropertyType::m_attackMin:
            ADD_ATTRIBUTE(m_attackMin, iter.second);
            break;
        case PropertyType::m_attackMax:
            ADD_ATTRIBUTE(m_attackMax, iter.second);
            break;
        case PropertyType::witchMin:
            ADD_ATTRIBUTE(witchMin, iter.second);
            break;
        case PropertyType::witchMax:
            ADD_ATTRIBUTE(witchMax, iter.second);
            break;
        case PropertyType::p_defenceMin:
            ADD_ATTRIBUTE(p_defenceMin, iter.second);
            break;
        case PropertyType::p_defenceMax:
            ADD_ATTRIBUTE(p_defenceMax, iter.second);
            break;
        case PropertyType::m_defenceMin:
            ADD_ATTRIBUTE(m_defenceMin, iter.second);
            break;
        case PropertyType::m_defenceMax:
            ADD_ATTRIBUTE(m_defenceMax, iter.second);
            break;
        case PropertyType::lucky:
            ADD_ATTRIBUTE(lucky, iter.second);
            break;
        case PropertyType::evil:
            ADD_ATTRIBUTE(evil, iter.second);
            break;
        case PropertyType::shot:
            ADD_ATTRIBUTE(shot, iter.second);
            break;
        case PropertyType::shotRatio:
            ADD_ATTRIBUTE(shotRatio, iter.second);
            break;
        case PropertyType::p_escape:
            ADD_ATTRIBUTE(p_escape, iter.second);
            break;
        case PropertyType::m_escape:
            ADD_ATTRIBUTE(m_escape, iter.second);
            break;
        case PropertyType::escapeRatio:
            ADD_ATTRIBUTE(escapeRatio, iter.second);
            break;
        case PropertyType::crit:
            ADD_ATTRIBUTE(crit, iter.second);
            break;
        case PropertyType::critRatio:
            ADD_ATTRIBUTE(critRatio, iter.second);
            break;
        case PropertyType::antiCrit:
            ADD_ATTRIBUTE(antiCrit, iter.second);
            break;
        case PropertyType::critDamage:
            ADD_ATTRIBUTE(critDamage, iter.second);
            break;
        case PropertyType::damageAdd:
            ADD_ATTRIBUTE(damageAdd, iter.second);
            break;
        case PropertyType::damageReduce:
            ADD_ATTRIBUTE(damageReduce, iter.second);
            break;
        case PropertyType::damageAddLv:
            ADD_ATTRIBUTE(damageAddLv, iter.second);
            break;
        case PropertyType::damageReduceLv:
            ADD_ATTRIBUTE(damageReduceLv, iter.second);
            break;
        case PropertyType::antiDropEquip:
            ADD_ATTRIBUTE(antiDropEquip, iter.second);
        default:
            break;
        }
    }
}

}
