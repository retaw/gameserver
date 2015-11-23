#ifndef PROCESSES_WORLD_PRIVATE_BOSS_H
#define PROCESSES_WORLD_PRIVATE_BOSS_H

#include "private_boss_base.h"
#include "water/componet/coord.h"
#include "map_base.h"
#include "npc.h"
#include "water/componet/datetime.h" 

namespace world{

using namespace water::componet;

class Role;
class PrivateBoss 
{
public:
    ~PrivateBoss() = default;
    PrivateBoss(Role& me);

    void afterEnterScene();
    
    void retPrivateBoss();
    void transFerToPrivateBoss(uint32_t bossId);
    void sendRefreshPrivateBoss();
    void sendRemainSeconds();

    void npcDie(const NpcTplId npcTplId, const std::vector<uint32_t>& objIdVec);
    void leave();
    void checkTimeOut();

private:
    void refreshnpcInScene();
    void getReward(uint32_t bossId);
    uint16_t getEnterTimes(PrivateBossTpl::Ptr bossTpl);
    void createDynamicSceneCallback(SceneId sceneId, RoleId roleId, uint32_t bossId, uint16_t posx, uint16_t posy, PrivateBossTpl bossTpl);

private:
    Role& owner;
//所有需要数据都放入杂项数据

    //场景地图初始的各个npc类型的数量，暂时缓存，每次进入场景需要初始化
    std::unordered_map<NpcTplId, uint32_t> m_npcInMap;

};


}
#endif
