#ifndef PROCESS_DBCACHED_HERO_MANAGER_H
#define PROCESS_DBCACHED_HERO_MANAGER_H
#include "dbcached.h"
#include "common/roledef.h"
#include "common/herodef.h"
#include "protocol/rawmsg/private/hero.h"

namespace dbcached{

class HeroManager
{
    friend class RoleManager;
public:
    ~HeroManager() = default;
    static HeroManager& me();

    void regMsgHandler();

private:
    HeroManager() = default;
    //for reg
    void servermsg_InsertHeroInfo(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId); //更新或者insert
    void servermsg_UpdateHeroLevelExp(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId); 
    void servermsg_UpdateHeroTurnLifeLevel(const uint8_t* msgData, uint32_t msgSize); 
    void servermsg_SaveHeroOffline(const uint8_t* msgData, uint32_t msgSize); 
    void servermsg_UpdateHeroClothes(const uint8_t* msgData, uint32_t msgSize);

    //业务
    void updateOrInsert(const PrivateRaw::InsertHeroInfo* rev);
    void updateHeroLevelExp(const PrivateRaw::UpdateHeroLevelExp* rev);
    void updateHeroTurnLifeLevel(const PrivateRaw::UpdateHeroTurnLifeLevel* rev);
    void saveHeroOffline(const PrivateRaw::SaveHeroOffline* rev);
    
    //基本的hero表操作
    bool insertHero(const PrivateRaw::InsertHeroInfo* heroInfo);
    bool updateDBHeroLevelExp(const PrivateRaw::UpdateHeroLevelExp* rev);
    bool updateDBHeroTurnLifeLevel(const PrivateRaw::UpdateHeroTurnLifeLevel* rev);
    bool saveDBHeroOffline(const PrivateRaw::SaveHeroOffline* rev);
    std::vector<HeroInfoPra> getHeroInfoByRoleId(RoleId roleId);
};
    
}

#endif
