#include "attack_mode.h"
#include "scene.h"
#include "massive_config.h"

#include "protocol/rawmsg/public/role_pk.h"
#include "protocol/rawmsg/public/role_pk.codedef.public.h"

namespace world{

AttackMode::AttackMode(Role& owner, uint8_t mode, uint16_t evil)
: m_mode(static_cast<attack_mode>(mode))
, m_mapAttackMode(attack_mode::none)
, m_owner(owner)
, m_evilVal(evil)
{
}

/*
 * force: true:服务端强制设置攻击模式
 */
void AttackMode::setMode(attack_mode mode, bool force/*=false*/)
{
    if(!force)
    {
        auto s = m_owner.scene();
        if(nullptr == s)
            return;
        if(mode == m_mode)
            return;
        if(attack_mode::none != s->mapAttackMode())
        {
            m_owner.sendSysChat("该地图不能切换攻击模式");
            return;
        }
        if(mode > attack_mode::consort)
            return;
        m_mode = mode;
    }
    m_mapAttackMode = mode;
    notifyAttackMode(mode);
}

attack_mode AttackMode::mode() const
{
    return m_mapAttackMode;
}

attack_mode AttackMode::saveMode() const
{
    return m_mode;
}

bool AttackMode::isEnemy(PK::Ptr atk) const
{
    if(nullptr == atk)
        return false;
    SceneItemType sceneItem = atk->sceneItemType();
    switch(sceneItem)
    {
    case SceneItemType::role:
    case SceneItemType::hero:
    case SceneItemType::pet:
        {
            Role::Ptr role = getRole(atk);
            return isEnemy(role);
        }
    case SceneItemType::npc:
        {
            Npc::Ptr npc = std::static_pointer_cast<Npc>(atk);
            return isEnemy(npc);
        }
    default:
        break;
    }
    return false;
}

bool AttackMode::isEnemy(Role::Ptr atkRole) const
{
    if(nullptr == atkRole)
        return false;
    if(atkRole->id() == m_owner.id())
        return false;
    attack_mode attackerMode = atkRole->m_attackMode.mode();
    switch(attackerMode)
    {
    case attack_mode::peace:
        return false;
    case attack_mode::all:
        return true;
    case attack_mode::team:
        {
            //LOG_DEBUG("组队模式, defRole({}) teamId={}, atkRole({}) teamId={}", m_owner.name(), m_owner.teamId(), atkRole->name(), atkRole->teamId());
            //组队模式, 如果攻击者没有队伍, 默认和全体模式一样
            if(0 == atkRole->teamId())
                return true;
            return m_owner.teamId() != atkRole->teamId();
        }
    case attack_mode::goodevil:
        return m_owner.issetNameColor(name_color::red);
    case attack_mode::unions:
        {
            if(0 == atkRole->factionId())
                return true;
            return m_owner.factionId() != atkRole->factionId();
        }
    case attack_mode::consort:
        return true;
    default:
        break;
    }
    return false;
}

bool AttackMode::isEnemy(Npc::Ptr atkNpc) const
{
    return true;
}

void AttackMode::login()
{
    uint32_t mins = SAFE_SUB(toUnixTime(Clock::now()), m_owner.offlineTime()) / 60;
    m_evilVal = SAFE_SUB(m_evilVal, mins * Massive::me().m_evilCfg.subEvil);
}

void AttackMode::initNameColor()
{
    name_color nameColor = name_color::white;
    if(m_owner.greynameTime() != water::componet::EPOCH
       && m_owner.greynameTime() + std::chrono::seconds {Massive::me().m_evilCfg.greynameTime} > water::componet::Clock::now())
    {
        nameColor = name_color::grey;
    }

    if(m_evilVal >= Massive::me().m_evilCfg.yellowEvil
       && m_evilVal < Massive::me().m_evilCfg.redEvil)
    {
        if(nameColor != name_color::grey)
            nameColor = name_color::yellow;
    }
    else if(m_evilVal >= Massive::me().m_evilCfg.redEvil)
    {
        nameColor = name_color::red;
    }
    
    m_owner.setNameColor(nameColor, false);
}

uint16_t AttackMode::evilVal() const
{
    return m_evilVal;
}

void AttackMode::addEvil()
{
    m_evilVal += Massive::me().m_evilCfg.killAddEvil;
    refreshPKValue();
    judgeAndSetNameColor();
}

void AttackMode::subEvil(uint16_t evil)
{
    if(0 == m_evilVal)
        return;

    m_evilVal = SAFE_SUB(m_evilVal, evil);
    refreshPKValue();
    judgeAndSetNameColor();
}

void AttackMode::judgeAndSetNameColor(bool clearGreyname)
{
    if(m_evilVal >= Massive::me().m_evilCfg.yellowEvil
       && m_evilVal < Massive::me().m_evilCfg.redEvil)
    {
        m_owner.clearNameColor(name_color::red);
        if(!m_owner.issetNameColor(name_color::grey))
            m_owner.setNameColor(name_color::yellow);
    }
    else if(m_evilVal >= Massive::me().m_evilCfg.redEvil)
    {
        //知道这样写很呕心,暂时没想到其他办法
        if(!clearGreyname)
            m_owner.clearNameColor(name_color::grey);
        m_owner.clearNameColor(name_color::yellow);
        m_owner.setNameColor(name_color::red);
    }
    else
    {
        m_owner.clearNameColor(name_color::red);
        m_owner.clearNameColor(name_color::yellow);
    }
}

void AttackMode::notifyAttackMode(attack_mode mode) const
{
    PublicRaw::NotifyAttackMode send;
    send.mode = static_cast<uint8_t>(mode);
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(NotifyAttackMode), &send, sizeof(send));
}

void AttackMode::refreshPKValue() const
{
    PublicRaw::RefreshPKValue send;
    send.val = m_evilVal;
    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshPKValue), &send, sizeof(send));
}

}

