/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-13 16:24 +0800
 *
 * Modified: 2015-04-14 11:58 +0800
 *
 * Description: 场景的抽象
 */

#ifndef PROCESSES_WORLD_GRID_H
#define PROCESSES_WORLD_GRID_H

#include "water/common/scenedef.h"
#include "npc.h"
#include "role.h"
#include "hero.h"
#include "scene_object.h"
#include "pet.h"
#include "fire.h"
#include "trigger.h"

#include <vector>
#include <array>

namespace world{

class Grid
{
public:
    Grid();
    ~Grid() = default;

//    Coord2D posInScene() const;

    Role::Ptr getRole(RoleId roleId) const;
    bool addRole(Role::Ptr);
    void eraseRole(Role::Ptr);
    void addDeadRole(RoleId roleId);
    void eraseDeadRole(RoleId roleId);
    uint32_t roleSize() const;
    bool isRoleBlock() const;

    bool addNpc(Npc::Ptr);
    void eraseNpc(Npc::Ptr);
    std::vector<Npc::Ptr> getAllNpcs() const;
    uint32_t npcSize() const;

    SceneObject::Ptr getObj(ObjectId objId) const;
    bool addObj(SceneObject::Ptr);
    void eraseObj(SceneObject::Ptr);
    uint32_t objSize() const;

    Hero::Ptr getHero(HeroId heroId) const;
    bool addHero(Hero::Ptr hero);
    void eraseHero(Hero::Ptr hero);
    uint32_t heroSize() const;

    Pet::Ptr getPet(PKId petId) const;
    bool addPet(Pet::Ptr pet);
    void erasePet(Pet::Ptr pet);
    uint32_t petSize() const;

    bool addFire(Fire::Ptr fire);
    void eraseFire(Fire::Ptr fire);

    bool addTrigger(Trigger::Ptr trigger);
    void eraseTrigger(Trigger::Ptr trigger);

    bool enterable(SceneItemType type) const;

    void setBlock();
    void unsetBlock();

    void setArea(AreaType type);
    void unsetArea(AreaType type);
    bool isArea(AreaType type) const;


    bool empty() const;
    void execGrid(std::function<void (PK::Ptr)> exec);


private:
    uint16_t m_areaTypes = 0;
    bool m_isBlock = false;
    RoleMap m_roles;
    NpcMap m_npcs;
	SceneObjMap m_sceneObjs;
	HeroMap m_heros;
    PetMap m_pets;
    FireMap m_fires;
    TriggerMap m_triggers;

    std::set<RoleId> m_deadRoles;   //死亡角色列表
};

}

#endif
