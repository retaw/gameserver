#ifndef PROCESS_WORLD_PET_MANAGER_H
#define PROCESS_WORLD_PET_MANAGER_H

#include "pet.h"
#include <unordered_map>

namespace world{



class PetManager final
{
private:
    PetManager();

public:
    ~PetManager() = default;

    static PetManager& me();
    PKId allocId();
    Pet::Ptr getById(PKId);

    bool insert(Pet::Ptr);
    void eraseFromScene(Pet::Ptr);

    Pet::Ptr summonPet(TplId petTplId, uint32_t skill, uint32_t level, PK::Ptr owner);

    void regTimer();
    void timerLoop(const water::componet::TimePoint& now);

private:
    PKId m_lastPetId;
    std::unordered_map<PKId, Pet::Ptr> m_petSet;
};

}

#endif

