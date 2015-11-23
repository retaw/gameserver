/*
 * Author: zhupengfei
 *
 * Created: 2015-06-16 11:05 +0800
 *
 * Modified: 2015-06-16 11:05 +0800
 *
 * Description: 英雄
 */

#ifndef PROCESS_WORLD_HERO_HPP
#define PROCESS_WORLD_HERO_HPP

#include "position.h"
#include "pk.h"
#include "level_props.h"
#include "package_set.h"
#include "wash.h"
#include "wing.h"
#include "zhuansheng.h"

#include "water/common/herodef.h"
#include "water/componet/class_helper.h"

#include "water/componet/fast_travel_unordered_map.h"

#include "protocol/rawmsg/private/hero.h"
#include "protocol/rawmsg/private/hero.codedef.private.h"
#include <memory>

namespace world{

class Role;

/*********************************************/
class Hero : public PK 
{
public:
	TYPEDEF_PTR(Hero)
	CREATE_FUN_MAKE(Hero) 

public:
	explicit Hero(HeroId heroId, std::shared_ptr<Role> role, const HeroInfoPra& info, const PrivateRaw::RetHeroSerializeData* rev);

	Hero() = default;
	virtual ~Hero() = default;

private:
	void init(const HeroInfoPra& info, const PrivateRaw::RetHeroSerializeData* rev);

	//初始化属性对象
	void initAttrMember();

public:
	std::shared_ptr<Role> getOwner() const; 
    PKId getOwnerId() const override;
    Job getOwnerJob() const override;
    SceneItemType getOwnerSceneItemType() const override;
    
	const Sex& sex() const;

	bool sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const override;
    void timerLoop(StdInterval interval, const water::componet::TimePoint& now) override;

	void afterEnterScene() override;
    void beforeLeaveScene() override;
    void offline();

    void enterVisualScreens(const std::vector<Screen*>& screens) const override;
    void leaveVisualScreens(const std::vector<Screen*>& screens) const override;

    void sendMainToMe() override;
    void syncScreenDataTo9() const override;

	bool changePos(Coord2D newPos, componet::Direction dir, MoveType type) override;
	void syncNewPosTo9Screens(MoveType type) const override;    
	
	void fillScreenData(HeroScreenData* data) const;  
	void fillMainData(HeroMainData* data) const;

    void moveToNearby(Coord2D goal, Coord1D nearby = 0); //尝试寻路并移动到goal附近
    //void moveTo(Coord2D goal) const;
    bool stop();
    enum class AIMode
    {
        peaceful = 0, passive = 1, initiative = 2
    };
    void setAIMode(AIMode mode);

private:
	void fillBasicData(HeroBasicData* data) const;   
	void sendSummonHeroTo9();

	TplId getTplIdByObjChildType(ObjChildType childType) const;

    void followThePath(const componet::TimePoint& now);
    bool backToOwnerNearby(bool forceBack = false);
    void aiLoop(const componet::TimePoint& now);
public:
	PackageType getEquipPackageType() const;
	PackageType getStonePackageType() const;


public:
	bool isDead() const override;
	void toDie(PK::Ptr attacker) override;
	void reset();

	void setLevel(uint32_t level) override;
	void setTurnLifeLevel(TurnLife level) override;  

    void handleInterrupt(interrupt_operate op, SceneItemType interruptModel) override;

    void initSkill(std::vector<SkillData>& data);
    void initPassiveSkillCD(std::vector<PKCdStatus>& data);
    void sendSkillListToMe();

	void addExp(uint64_t exp);
	uint64_t getExp() const;

	void judgeLevelUp();
	void levelUp(uint32_t upNum = 1, bool GmFlag = false);

	bool updateLevelAndExpToDB();
	void sendHeroCurLevelGotExp();

	//当前等级已获得的经验值 == 累计经验值 - 当前等级需要经验值
	uint64_t getCurLevelGotExp() const;

	//升级需要经验值 == 下一级需要经验值 - 当前等级需要经验值
	uint64_t getLevelUpNeedExp() const;

    void onOwnerAttackSth(PK::Ptr target, const componet::TimePoint& tp);
    void lockOnAndAttack(PK::Ptr target);
    void lockOnTarget(PK::Ptr target, bool silence = false);

    void underAttack(PK::Ptr atk) override;

private:
	uint32_t getHeroCanLevelUpNum() const; 
    void dieDropEquip();

	bool updateTurnLifeLevelToDB();
    void addEnemy(PK::Ptr atk);

	void autoAddHpAndMp(); 


public:
    uint32_t getMaxHp() const override;
    uint32_t getHpRatio() const override;
    uint32_t getHpLv() const override;
    uint32_t getMaxMp() const override;
    uint32_t getMpRatio() const override;
    uint32_t getMpLv() const override;

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
    uint32_t getTotalDmgAddLv() const override; 
    uint32_t getTotalDmgReduce() const override;
    uint32_t getTotalDmgReduceLv() const override;

    uint32_t getTotalAntiDropEquip() const override;
   
public:
	uint32_t getMinStrongLevel() const;
	uint32_t getStoneTotalLevel() const;

private:
	const std::weak_ptr<Role> m_owner;	//Role::WPtr
	const Sex m_sex;
	LevelProps  m_levelProps;

public:
	PackageSet  m_packageSet;

    //宠物数据
    PKAttr     m_petPoisonAttr;
    std::vector<BuffData> m_petBuffs;


private:
	uint64_t m_exp;
    bool    m_dead;

public:
	Wash		m_wash;
	Wing		m_wing;
	Zhuansheng	m_zhuansheng;

private:
	std::vector<Attribute*> m_attrMembers;  

private: //ai
    componet::TimePoint m_lastTimerTime;
    struct AI
    {
        enum 
        { 
            MAX_ACTION_DISTANCE = 8,
            MAX_FOLLOW_DISTANCE = 12,
            ATTBACK_DURATION_SEC = 30,
        };

        struct
        {
            std::chrono::milliseconds stepWalkInterval;
            std::chrono::milliseconds stepRunInterval;
            std::chrono::milliseconds stepDuration;

            componet::TimePoint nextStepAble = componet::EPOCH;   //上一步完成的时间
            componet::TimePoint lastStepDone = componet::EPOCH;
            std::list<Coord2D> path;  //当前要走的路径

            Coord2D goal;
            Coord1D nearby;
        } moveData;

        AIMode mode = AIMode::peaceful;

        enum class TargetPriority
        {
            manual = 0, attByOwner = 1, attUs = 2, foundByMyself = 3,
        };
        /*
        const std::array<TargetPriority, 4> allTargetPriority;
        {
            TargetPriority::manual,
            TargetPriority::attByOwner,
            TargetPriority::attUs,
            TargetPriority::foundByMyself
        };*/
        std::map<TargetPriority, PK::WPtr> targets;
        TplId jointSkillId = 0;
        componet::TimePoint ownerLastAttSthTime;
        componet::TimePoint beAttackTime;
    } m_ai;
};

typedef water::componet::FastTravelUnorderedMap<HeroId, Hero::Ptr> HeroMap; 

}

#endif
