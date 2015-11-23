/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-13 16:24 +0800
 *
 * Modified: 2015-04-14 11:58 +0800
 *
 * Description: 场景的抽象
 */

#ifndef PROCESSES_WORLD_SCREEN_H
#define PROCESSES_WORLD_SCREEN_H


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

class Screen
{
public:
    enum {WIDTH = 10, HEIGHT = 14};
    TYPEDEF_PTR(Screen);

    bool addRole(Role::Ptr);
    void eraseRole(Role::Ptr);

    bool addNpc(Npc::Ptr);
    void eraseNpc(Npc::Ptr);

    bool addObj(SceneObject::Ptr obj);
    void eraseObj(SceneObject::Ptr obj);

    bool addHero(Hero::Ptr hero);
    void eraseHero(Hero::Ptr hero);

    bool addPet(Pet::Ptr pet);
    void erasePet(Pet::Ptr pet);

    bool addFire(Fire::Ptr fire);
    void eraseFire(Fire::Ptr fire);

    bool addTrigger(Trigger::Ptr trigger);
    void eraseTrigger(Trigger::Ptr trigger);

    RoleMap& roles();
    const RoleMap& roles() const;

    NpcMap& npcs();
    const NpcMap& npcs() const;
	
	SceneObjMap& objs();
	const SceneObjMap& objs() const;

	HeroMap& heros();
	const HeroMap& heros() const;

    PetMap& pets();
    const PetMap& pets() const;

    FireMap& fires();
    const FireMap& fires() const;

    TriggerMap& triggers();
    const TriggerMap& triggers() const;

private:
    RoleMap m_roles;
    NpcMap m_npcs;
	SceneObjMap m_sceneObjs;
	HeroMap m_heros;
    PetMap m_pets;
    FireMap m_fires;
    TriggerMap m_triggers;
};


}

#endif
