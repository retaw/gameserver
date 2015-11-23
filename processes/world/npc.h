/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-13 17:37 +0800
 *
 * Modified: 2015-04-13 17:37 +0800
 *
 * Description:  NPC 的抽象
 */

#ifndef PROCESSES_WORLD_NPC_H
#define PROCESSES_WORLD_NPC_H


#include "npc_base.h"

#include "pk.h"
#include "attribute.h"
#include "ai.h"

#include "water/componet/class_helper.h"
#include "water/componet/fast_travel_unordered_map.h"
#include "water/componet/datetime.h"

#include "protocol/rawmsg/public/npc_scene.h"
#include "water/componet/random.h"


#include <list>


namespace world{

typedef PKId NpcId;

class Npc : public PK, public Attribute
{
public:
    TYPEDEF_PTR(Npc)
    CREATE_FUN_MAKE(Npc)

    Npc(NpcId id, NpcTpl::Ptr tpl);
    ~Npc() = default;

    void initTplData();//这里调用了shared_from_this, 不能放入构造函数, 只能两步初始化, 破坏掉raii了......
    void setOriginalPos(Coord2D);
    const Coord2D& originalPos() const;
    void setHomePos(Coord2D pos);
    const Coord2D& homePos() const;
    uint16_t extend() const; //扩展字段
    uint16_t belongtime() const; //归属时间

    const NpcTpl& tpl() const;
    NpcTplId tplId() const;
    NpcType type() const;

    uint32_t aiTplId() const;
    void resetAI();
    void handleAIEvent(const ai::AIEvent* aiEvent);
    AIData& aiData();

    std::string toString() const;

    void fillScreenData(PublicRaw::NpcScreenData* data) const;

    void setOwnerId(PKId ownerId);
    PKId getOwnerId() const override;

    void toDie(PK::Ptr atk) override;
    void setDead();
    bool isDead() const override;

    void afterEnterScene() override;
    void beforeLeaveScene() override;

    void enterVisualScreens(const std::vector<Screen*>& screens) const override;
    void leaveVisualScreens(const std::vector<Screen*>& screens) const override;

    bool changePos(Coord2D newPos, componet::Direction dir, MoveType type) override;
    void syncNewPosTo9Screens(MoveType type) const override;

    void tryIdelMove(); //空闲时的随机移动, 随机方向走一格
    void moveToNearby(Coord2D goal, Coord1D nearby = 0); //尝试寻路并移动到goal附近

    void timerLoop(StdInterval interval, const componet::TimePoint& now) override;

    void releaseSkill(int32_t skillTplId, const Coord2D& pos);

    void setCollectRoleId(PKId id);
    PKId collectRoleId() const;

public:
    uint32_t getMaxHp() const override;
    uint32_t getMaxMp() const override;

    uint32_t getTotalPAtkMin() const override;
    uint32_t getTotalPAtkMax() const override;
    uint32_t getTotalMAtkMin() const override;
    uint32_t getTotalMAtkMax() const override;
    uint32_t getTotalWitchMin() const override;
    uint32_t getTotalWitchMax() const override;
    uint32_t getTotalPDefMin() const override;
    uint32_t getTotalPDefMax() const override;
    uint32_t getTotalMDefMin() const override;
    uint32_t getTotalMDefMax() const override;
    uint32_t getTotalLucky() const override;
    uint32_t getTotalEvil() const override;

    uint32_t getTotalShot() const override;
    uint32_t getTotalShotRatio() const override;
    uint32_t getTotalPEscape() const override;
    uint32_t getTotalMEscape() const override;
    uint32_t getTotalEscapeRatio() const override;
    uint32_t getTotalCrit() const override;
    uint32_t getTotalCritRatio() const override;
    uint32_t getTotalAntiCrit() const override;
    uint32_t getTotalCritDmg() const override;

    uint32_t getTotalDmgAdd() const override;
    uint32_t getTotalDmgReduce() const override;
    
private:
    void followThePath(const componet::TimePoint& now);

    //
    void checkDealCorpseAndrelive(const componet::TimePoint& now);
    void dropObj(PK::Ptr atk);
    //bool isGridEmpty(Coord2D pos);
private:
    NpcTpl::Ptr m_tpl;
    Coord2D m_destatination;

    bool m_dead;
    componet::TimePoint m_dieTime;  //npc死亡时间戳
    bool m_isCorpseExist = false;   //是否死亡尸体存在
    Coord2D m_originalPos;
    Coord2D m_homePos;
    PKId m_collectRoleId = 0;       //采集者id
    componet::TimePoint m_collectTp;
    PKId m_ownerId = 0;             //主人id

    ai::AI::Ptr m_ai;

    struct
    {
        std::chrono::milliseconds stepInterval;
        std::chrono::milliseconds stepDuration;

        componet::TimePoint nextStepAble = componet::EPOCH;   //上一步完成的时间
        componet::TimePoint lastStepDone = componet::EPOCH;
        std::list<Coord2D> path;  //当前要走的路径

        Coord2D goal;
        Coord1D nearby;
    } m_moveData;

    AIData m_aiData;
};

typedef water::componet::FastTravelUnorderedMap<NpcId, Npc::Ptr> NpcMap;

}

#endif
