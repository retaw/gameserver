#include "skill_effect_element.h"
#include "pk.h"
#include "world.h"
#include "scene.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/skill.h"
#include "protocol/rawmsg/private/skill.codedef.private.h"



namespace world{

using namespace water;
using namespace water::componet;

PKValue::PKValue()
: m_skillPower(0)
  , m_skillDamage(0)
  , m_damageAddPer(0)
  , m_constDamage(0)
  , m_vampirePdamagePer(0)
  , m_vampireMdamagePer(0)
  , m_ignorePdefencePer(0)
  , m_ignoreMdefencePer(0)
  , m_bossConstDamage(0)
  , m_bossDamageAdd(0)
  , m_mobConstDamage(0)
  , m_mobDamageAdd(0)
  , m_roleConstDamage(0)
  , m_roleDamageAdd(0)
{
}


void PKValue::clear()
{
    m_skillPower = 0;
    m_skillDamage = 0;
    m_damageAddPer = 0;
    m_constDamage = 0;
    m_vampirePdamagePer = 0;
    m_vampireMdamagePer = 0;
    m_ignorePdefencePer = 0;
    m_ignoreMdefencePer = 0;

    m_bossConstDamage = 0;
    m_bossDamageAdd = 0;
    m_mobConstDamage = 0;
    m_mobDamageAdd = 0;
    m_roleConstDamage = 0;
    m_roleDamageAdd = 0;
}

}




