#ifndef PROCESS_WORLD_PET_BASE_H
#define PROCESS_WORLD_PET_BASE_H

#include "pkdef.h"
#include "water/common/roledef.h"
#include "water/componet/class_helper.h"

#include <unordered_map>

namespace world{

struct PetTpl
{
    TYPEDEF_PTR(PetTpl)
    CREATE_FUN_MAKE(PetTpl)

    TplId id;
    std::string name;
    Job job;
    std::vector<std::pair<uint32_t, uint32_t>> skillIDs;
    std::vector<std::pair<PropertyType, uint32_t>> props; //基础属性
};


#define pet_level_hash(id, level) (id*1000+level)
struct PetLevelTpl
{
    TYPEDEF_PTR(PetLevelTpl)
    CREATE_FUN_MAKE(PetLevelTpl)

    uint32_t skillId;
    uint32_t skillLv;
    std::vector<std::pair<PropertyType, uint32_t>> props; //基础属性
};


class PetBase
{
public:
    ~PetBase() = default;

private:
    PetBase() = default;

public:
    void loadConfig(const std::string& cfgdir);
    PetTpl::Ptr getPetTpl(TplId petTplId);
    PetLevelTpl::Ptr getPetLevelTpl(uint32_t skillId, uint32_t skillLv);

    static PetBase& me();

private:
    std::unordered_map<TplId, PetTpl::Ptr> m_petTpls;
    std::unordered_map<uint32_t, PetLevelTpl::Ptr> m_petLevelTpls;
};

}

#endif
