#include "screen.h"

namespace world{

bool Screen::addRole(Role::Ptr role)
{
	if(role == nullptr)
		return false;

    m_roles.erase(role->id());
    return m_roles.insert(std::make_pair(role->id(), role));
}

void Screen::eraseRole(Role::Ptr role)
{
	if(role == nullptr)
		return;

    m_roles.erase(role->id());
}

bool Screen::addNpc(Npc::Ptr npc)
{
	if(npc == nullptr)
		return false;

    m_npcs.erase(npc->id());
    return m_npcs.insert(std::make_pair(npc->id(), npc));
}

void Screen::eraseNpc(Npc::Ptr npc)
{
	if(npc == nullptr)
		return;

    m_npcs.erase(npc->id());
}

bool Screen::addObj(SceneObject::Ptr obj)
{
	if(obj == nullptr)
		return false;

    m_sceneObjs.erase(obj->objId());
    return m_sceneObjs.insert(std::make_pair(obj->objId(), obj));
}

void Screen::eraseObj(SceneObject::Ptr obj)
{
	if(obj == nullptr)
		return;

    m_sceneObjs.erase(obj->objId());
}

bool Screen::addHero(Hero::Ptr hero)
{
	if(hero == nullptr)
		return false;

    m_roles.erase(hero->id());
    return m_heros.insert(std::make_pair(hero->id(), hero));
}

void Screen::eraseHero(Hero::Ptr hero)
{
	if(hero == nullptr)
		return;

    m_heros.erase(hero->id());
}

bool Screen::addPet(Pet::Ptr pet)
{
    if(nullptr == pet)
        return false;

    m_pets.erase(pet->id());
    return m_pets.insert(std::make_pair(pet->id(), pet));
}

void Screen::erasePet(Pet::Ptr pet)
{
    if(nullptr == pet)
        return;

    m_pets.erase(pet->id());
}

bool Screen::addFire(Fire::Ptr fire)
{
    if(nullptr == fire)
        return false;

    m_fires.erase(fire->id());
    return m_fires.insert(std::make_pair(fire->id(), fire));
}

void Screen::eraseFire(Fire::Ptr fire)
{
    if(nullptr == fire)
        return;
    m_fires.erase(fire->id());
}

bool Screen::addTrigger(Trigger::Ptr trigger)
{
    if(nullptr == trigger)
        return false;

    m_triggers.erase(trigger->id());
    return m_triggers.insert(std::make_pair(trigger->id(), trigger));
}

void Screen::eraseTrigger(Trigger::Ptr trigger)
{
    if(nullptr == trigger)
        return;
    m_triggers.erase(trigger->id());
}

RoleMap& Screen::roles()
{
    return m_roles;
}

const RoleMap& Screen::roles() const
{
    return m_roles;
}

NpcMap& Screen::npcs()
{
    return m_npcs;
}

const NpcMap& Screen::npcs() const
{
    return m_npcs;
}

SceneObjMap& Screen::objs()
{
	return m_sceneObjs;
}

const SceneObjMap& Screen::objs() const
{
	return m_sceneObjs;
}

HeroMap& Screen::heros()
{
	return m_heros;
}

const HeroMap& Screen::heros() const
{
	return m_heros;
}

PetMap& Screen::pets()
{
    return m_pets;
}

const PetMap& Screen::pets() const
{
    return m_pets;
}

FireMap& Screen::fires()
{
    return m_fires;
}

const FireMap& Screen::fires() const
{
    return m_fires;
}

TriggerMap& Screen::triggers()
{
    return m_triggers;
}

const TriggerMap& Screen::triggers() const
{
    return m_triggers;
}

}
