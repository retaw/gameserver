#ifndef PROCESS_WORLD_PET_H
#define PROCESS_WORLD_PET_H

/*
 * 宠物(召唤兽)
 */

#include "pk.h"
#include "attribute.h"
#include "pet_base.h"
#include "level_props.h"
#include "water/componet/class_helper.h"
#include "water/componet/fast_travel_unordered_map.h"

#include "protocol/rawmsg/public/pet_scene.h"

namespace world{

class PetProps : public Attribute
{
public:
    PetProps() = default;
    ~PetProps() = default;
};

class Pet : public PK, public Attribute
{
public:
    TYPEDEF_PTR(Pet)
    CREATE_FUN_NEW(Pet)


public:
    explicit Pet(PKId id, PetTpl::Ptr petTpl, PetLevelTpl::Ptr petLevelTpl, PK::Ptr me);
    ~Pet() = default;

public:
    void fillScreenData(PublicRaw::PetScreenData* data) const;
    void initTplData();
    
    PK::Ptr getOwner() const;
    PKId getOwnerId() const override;
    SceneItemType getOwnerSceneItemType() const override;
    Job getOwnerJob() const override;
    void separateFromOwner();
    void clearSelfBuff();


public:
    void afterEnterScene() override;
    void beforeLeaveScene() override;

    void enterVisualScreens(const std::vector<Screen*>& screens) const override;
    void leaveVisualScreens(const std::vector<Screen*>& screens) const override;

    bool changePos(Coord2D newPos, componet::Direction dir, MoveType type) override;
    void syncNewPosTo9Screens(MoveType type) const override;

    void timerLoop(StdInterval interval, const water::componet::TimePoint& now) override;
    void syncScreenDataTo9() const override;


public:
    void toDie(PK::Ptr atk) override;
    bool isDead() const override;
    void erase(bool);


public:
    uint32_t getMaxHp() const override;
    uint32_t getMaxMp() const override;

    uint32_t getTotalPAtkMin() const override;         //物攻min
    uint32_t getTotalPAtkMax() const override;         //物攻max
    uint32_t getTotalMAtkMin() const override;         //魔法min
    uint32_t getTotalMAtkMax() const override;         //魔法max
    uint32_t getTotalWitchMin() const override;        //道术min
    uint32_t getTotalWitchMax() const override;        //道术max
    uint32_t getTotalPDefMin() const override;         //物防min
    uint32_t getTotalPDefMax() const override;         //物防max
    uint32_t getTotalMDefMin() const override;         //魔防min
    uint32_t getTotalMDefMax() const override;         //魔防max
    uint32_t getTotalLucky() const override;           //幸运
    uint32_t getTotalEvil() const override;            //诅咒

    uint32_t getTotalShot() const override;            //命中
    uint32_t getTotalShotRatio() const override;       //命中率
    uint32_t getTotalPEscape() const override;         //物闪
    uint32_t getTotalMEscape() const override;         //魔闪
    uint32_t getTotalEscapeRatio() const override;     //闪避率
    uint32_t getTotalCrit() const override;            //暴击
    uint32_t getTotalCritRatio() const override;       //暴击率
    uint32_t getTotalAntiCrit() const override;        //防暴
    uint32_t getTotalCritDmg() const override;         //暴伤

    uint32_t getTotalDmgAdd() const override;          //增伤
    uint32_t getTotalDmgReduce() const override;       //减伤

private:
    std::weak_ptr<PK>   m_owner;
    SceneItemType       m_ownerSceneItem;
    PetTpl::Ptr         m_petTpl;
    PetLevelTpl::Ptr    m_petLevelTpl;

    bool                m_dead;
};

typedef water::componet::FastTravelUnorderedMap<PKId, Pet::Ptr> PetMap; 
}

#endif
