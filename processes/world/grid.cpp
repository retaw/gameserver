#include "grid.h"


namespace world{


Grid::Grid()
{
}

Role::Ptr Grid::getRole(RoleId roleId) const
{
    auto it = m_roles.find(roleId);
    if(it == m_roles.end())
        return nullptr;

    return it->second;
}

bool Grid::addRole(Role::Ptr role)
{
	if(role == nullptr)
		return false;

    //是否格子已满
    if(!enterable(SceneItemType::role))
        return false;
    
    m_roles.erase(role->id());   
    return m_roles.insert(std::make_pair(role->id(), role));
}

void Grid::eraseRole(Role::Ptr role)
{
	if(role == nullptr)
		return;

    m_roles.erase(role->id());
    m_deadRoles.erase(role->id());
}

void Grid::addDeadRole(RoleId roleId)
{
    m_deadRoles.insert(roleId);
}

void Grid::eraseDeadRole(RoleId roleId)
{
    m_deadRoles.erase(roleId);
}

uint32_t Grid::roleSize() const
{
    return m_roles.size();
}

bool Grid::isRoleBlock() const
{
    return m_roles.size() > m_deadRoles.size();
}

bool Grid::addNpc(Npc::Ptr npc)
{
	if(npc == nullptr)
		return false;

    if(!enterable(SceneItemType::npc))
        return false;
        
    m_npcs.erase(npc->id());
    return m_npcs.insert(std::make_pair(npc->id(), npc));
}

void Grid::eraseNpc(Npc::Ptr npc)
{
	if(npc == nullptr)
		return;

    m_npcs.erase(npc->id());
}

std::vector<Npc::Ptr> Grid::getAllNpcs() const
{
    return m_npcs.getValues();
}

uint32_t Grid::npcSize() const
{
    return m_npcs.size();
}

SceneObject::Ptr Grid::getObj(ObjectId objId) const
{
    auto it = m_sceneObjs.find(objId);
    if(it == m_sceneObjs.end())
        return nullptr;

    return it->second;
}

bool Grid::addObj(SceneObject::Ptr obj)
{
	if(obj == nullptr)
		return false;

    //是否格子已满
    if(!enterable(SceneItemType::object))
        return false;
    
    m_sceneObjs.erase(obj->objId());   
    return m_sceneObjs.insert(std::make_pair(obj->objId(), obj));
}

void Grid::eraseObj(SceneObject::Ptr obj)
{
	if(obj == nullptr)
		return;

    m_sceneObjs.erase(obj->objId());
}

uint32_t Grid::objSize() const
{
    return m_sceneObjs.size();
}

Hero::Ptr Grid::getHero(HeroId heroId) const
{
    auto it = m_heros.find(heroId);
    if(it == m_heros.end())
        return nullptr;

    return it->second;
}

bool Grid::addHero(Hero::Ptr hero)
{
	if(hero == nullptr)
		return false;

    //是否格子已满
    if(!enterable(SceneItemType::hero))
        return false;
    
    m_heros.erase(hero->id());   
    return m_heros.insert(std::make_pair(hero->id(), hero));
}

void Grid::eraseHero(Hero::Ptr hero)
{
	if(hero == nullptr)
		return;

    m_heros.erase(hero->id());
}

uint32_t Grid::heroSize() const
{
    return m_heros.size();
}

Pet::Ptr Grid::getPet(PKId petId) const
{
    auto it = m_pets.find(petId);
    if(it == m_pets.end())
        return nullptr;

    return it->second;
}

bool Grid::addPet(Pet::Ptr pet)
{
    if(nullptr == pet)
        return false;

    if(!enterable(SceneItemType::pet))
        return false;

    m_pets.erase(pet->id());
    return m_pets.insert(std::make_pair(pet->id(), pet));
}

void Grid::erasePet(Pet::Ptr pet)
{
    if(nullptr == pet)
        return;

    m_pets.erase(pet->id());
}

uint32_t Grid::petSize() const
{
    return m_pets.size();
}

bool Grid::addFire(Fire::Ptr fire)
{
    if(nullptr == fire)
        return false;

    if(!enterable(SceneItemType::fire))
        return false;

    m_fires.erase(fire->id());
    return m_fires.insert(std::make_pair(fire->id(), fire));
}

void Grid::eraseFire(Fire::Ptr fire)
{
    if(nullptr == fire)
        return;

    m_fires.erase(fire->id());
}

bool Grid::addTrigger(Trigger::Ptr trigger)
{
    if(nullptr == trigger)
        return false;

    if(!enterable(SceneItemType::trigger))
        return false;

    if(trigger->triggerType() == TriggerType::block)
        setBlock();
    m_triggers.erase(trigger->id());
    return m_triggers.insert(std::make_pair(trigger->id(), trigger));
}

void Grid::eraseTrigger(Trigger::Ptr trigger)
{
    if(nullptr == trigger)
        return;
    if(trigger->triggerType() == TriggerType::block)
        unsetBlock();
    m_triggers.erase(trigger->id());
}

void Grid::setBlock()
{
    m_isBlock = true;
}

void Grid::unsetBlock()
{
    m_isBlock = false;
}

bool Grid::enterable(SceneItemType itemType) const
{
    if(m_isBlock)
        return false;

    switch (itemType)
    {
    case SceneItemType::role:
	case SceneItemType::hero:
        if(!isArea(AreaType::collision))
            return true;
		return !isRoleBlock() && m_heros.empty() && m_npcs.empty();
    case SceneItemType::npc:
        return m_npcs.empty() && !isRoleBlock() && m_heros.empty() && m_pets.empty();
	case SceneItemType::object:
		return m_sceneObjs.empty() && !isRoleBlock() && m_npcs.empty() && m_heros.empty() && m_pets.empty();
    case SceneItemType::pet:
        return true;
    case SceneItemType::fire:
        return m_fires.empty();
    case SceneItemType::trigger:
        return m_triggers.empty();
    default:
        break;
    }
    return false;
}

void Grid::setArea(AreaType type)
{
    m_areaTypes = m_areaTypes | static_cast<uint16_t>(type);
}

void Grid::unsetArea(AreaType type)
{
    m_areaTypes = m_areaTypes & (~static_cast<uint16_t>(type));
}

bool Grid::isArea(AreaType type) const
{
    return m_areaTypes & static_cast<uint16_t>(type);
}

bool Grid::empty() const
{
    return m_roles.empty() && m_npcs.empty() && m_heros.empty() && m_pets.empty();
}

void Grid::execGrid(std::function<void (PK::Ptr)> exec)
{
    for(auto& iter : m_roles)
        exec(iter.second);
    for(auto& iter : m_npcs)
        exec(iter.second);
    for(auto& iter : m_heros)
        exec(iter.second);
    for(auto& iter : m_pets)
        exec(iter.second);
}

}
