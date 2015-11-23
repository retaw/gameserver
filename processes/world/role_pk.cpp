#include "role_pk.h"

#include "role_manager.h"

#include "protocol/rawmsg/public/role_pk.h"
#include "protocol/rawmsg/public/role_pk.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{


using namespace std::placeholders;

RolePk& RolePk::me()
{
    static RolePk me;
    return me;
}



void RolePk::regMsgHandler()
{
    REG_RAWMSG_PUBLIC(RoleRequestUpgradeSkill, std::bind(&RolePk::clientmsg_RoleRequestUpgradeSkill, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(HeroRequestUpgradeSkill, std::bind(&RolePk::clientmsg_HeroRequestUpgradeSkill, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RoleRequestStrengthenSkill, std::bind(&RolePk::clientmsg_RoleRequestStrengthenSkill, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(HeroRequestStrengthenSkill, std::bind(&RolePk::clientmsg_HeroRequestStrengthenSkill, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RoleRequestAttack, std::bind(&RolePk::clientmsg_RequestAttack, this, _1, _2, _3, _4));
	REG_RAWMSG_PUBLIC(RequestReliveRole, std::bind(&RolePk::clientmsg_RequestReliveRole, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestChangeAttackMode, std::bind(&RolePk::clientmsg_RequestChangeAttackMode, this, _1, _2, _3));
}

void RolePk::clientmsg_RoleRequestUpgradeSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto rev = reinterpret_cast<const PublicRaw::RoleRequestUpgradeSkill*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;

    role->upgradeSkill(rev->skillId);
}

void RolePk::clientmsg_HeroRequestUpgradeSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;
 
	auto rev = reinterpret_cast<const PublicRaw::HeroRequestUpgradeSkill*>(msgData);
    role->m_heroManager.upgradeSkill(rev->skillId);
}

void RolePk::clientmsg_RoleRequestStrengthenSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto rev = reinterpret_cast<const PublicRaw::RoleRequestStrengthenSkill*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;
    
    role->strengthenSkill(rev->skillId);
}

void RolePk::clientmsg_HeroRequestStrengthenSkill(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto rev = reinterpret_cast<const PublicRaw::HeroRequestStrengthenSkill*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;
    
    role->m_heroManager.strengthenSkill(rev->skillId);
}

void RolePk::clientmsg_RequestAttack(const uint8_t* msgData, uint32_t msgSize, RoleId rid, const TimePoint& now)
{
    auto rev = reinterpret_cast<const PublicRaw::RoleRequestAttack*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;

    if(rev->atkModel == static_cast<uint8_t>(SceneItemType::role))
    {
        role->launchAttack(rev);
        auto t = role->target();
        if(t != nullptr && t->pos().distance(Coord2D(rev->posX, rev->posY)) <= 1)
        {
            Hero::Ptr hero = role->m_heroManager.getSummonHero();
            if(nullptr != hero)
                hero->onOwnerAttackSth(t, now);
        }
    }
    else if(rev->atkModel == static_cast<uint8_t>(SceneItemType::hero))
    {
        Hero::Ptr hero = role->m_heroManager.getSummonHero();
        if(nullptr != hero)
            hero->launchAttack(rev);
    }
    else if(rev->atkModel == static_cast<uint8_t>(SceneItemType::pet))
    {
        //宠物
        PK::Ptr pet = PK::getPkptr(rev->atkId, SceneItemType::pet);
        Role::Ptr owner = getRole(pet);
        if(nullptr == owner)
            return;
        if(owner != role)
            return;
        pet->launchAttack(rev);
    }
}

//请求角色复活
void RolePk::clientmsg_RequestReliveRole(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	auto role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	role->requestRelive(msgData, msgSize);
}


void RolePk::clientmsg_RequestChangeAttackMode(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	auto role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

    auto rev = reinterpret_cast<const PublicRaw::RequestChangeAttackMode*>(msgData);
	role->m_attackMode.setMode(static_cast<attack_mode>(rev->mode));
}











}
