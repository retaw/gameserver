#include "pet_manager.h"
#include "scene_manager.h"
#include "world.h"

#include "water/componet/logger.h"

namespace world{

PetManager::PetManager()
:m_lastPetId(1)
{
}

PetManager& PetManager::me()
{
    static PetManager me;
    return me;
}

Pet::Ptr PetManager::getById(PKId id)
{
    if(m_petSet.find(id) == m_petSet.end())
        return nullptr;
    return m_petSet[id];
}

PKId PetManager::allocId()
{
    return m_lastPetId++;
}

bool PetManager::insert(Pet::Ptr pet)
{
    return m_petSet.insert(std::make_pair(pet->id(), pet)).second;
}

void PetManager::eraseFromScene(Pet::Ptr pet)
{
    if(nullptr == pet)
        return;
    auto s = pet->scene();
    if(nullptr != s)
        s->erasePet(pet);
}


/*
 * skill:技能ID,  level::技能等级
 */
Pet::Ptr PetManager::summonPet(TplId petTplId, uint32_t skill, uint32_t level, PK::Ptr owner)
{
    if(0 == petTplId || 0 == skill || 0 == level)
        return nullptr;
    if(nullptr == owner)
        return nullptr;

    auto s = owner->scene();
    if(nullptr == s)
        return nullptr;

    Pet::Ptr pet = getById(owner->petId());
    if(nullptr != pet && !pet->needErase()) //已经存在, 拉到主人身边
    {
        Coord2D ownerPos = owner->pos();
        auto callback = [&] (Coord2D pos) -> bool
        {
            if(pos == ownerPos)
                return false;

            return pet->changePos(pos, owner->dir(), MoveType::blink);
        };

        s->tryExecSpiral(ownerPos, 5, callback);
        return pet;
    }

    PetTpl::Ptr petTpl = PetBase::me().getPetTpl(petTplId);
    PetLevelTpl::Ptr petLevelTpl = PetBase::me().getPetLevelTpl(skill, level);
    if(nullptr == petTpl)
    {
        LOG_DEBUG("宠物, 召唤时找不到配置, tplId:{}, owner:{}", petTplId, owner->name());
        return nullptr;
    }
    if(nullptr == petLevelTpl)
    {
        LOG_DEBUG("宠物, 召唤时找不到配置, skillId:{}, skillLv:{}, owner:{}", skill, level, owner->name());
        return nullptr;
    }

    pet = Pet::create(allocId(), petTpl, petLevelTpl, owner);
    if(nullptr == pet)
    {
        LOG_DEBUG("宠物, Pet::create失败");
        return nullptr;
    }

    if(!insert(pet))
    {
        LOG_DEBUG("宠物, insert失败");
        return nullptr;
    }

    if(!s->addPet(pet, owner->pos(), 5))
    {
        LOG_DEBUG("宠物, addpet到场景失败");
        m_petSet.erase(pet->id());
        return nullptr;
    }
    owner->setPetId(pet->id());
    owner->setPetTplId(petTplId);
    pet->setDir(owner->dir());
    pet->initTplData();
    pet->afterEnterScene();
    return pet;
}

void PetManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::milliseconds(500), 
                         std::bind(&PetManager::timerLoop, this, _1));
}

void PetManager::timerLoop(const water::componet::TimePoint& now)
{
    auto it = m_petSet.begin();
    for(; it != m_petSet.end(); )
    {
        auto& pet = it->second;
        if(nullptr == pet)
        {
            m_petSet.erase(it++);
        }
        else if(pet->needErase())
        {
            eraseFromScene(pet);
            m_petSet.erase(it++);
        }
        else
            ++it;
    }
}

}
