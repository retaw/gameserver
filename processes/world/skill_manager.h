/*
 *
 * 技能模块
 *
 */


#ifndef PROCESS_WORLD_SKILL_MGR_H
#define PROCESS_WORLD_SKILL_MGR_H

#include "water/common/commdef.h"
#include "water/common/roledef.h"
#include "protocol/rawmsg/public/role_pk.h"
#include "protocol/rawmsg/public/role_pk.codedef.public.h"
#include "skill_effect_element.h"
#include "config_table_manager.h"

#include <vector>
#include <set>
#include <unordered_map>

namespace world{

class Skill final
{
public:
    explicit Skill();
    ~Skill() = default;

public:
    TYPEDEF_PTR(Skill)


public:
    TplId getID() const;
    std::string getName() const;
    uint32_t getLevel() const;
    std::pair<Job, Job> job() const;
    uint32_t getStrengthenLv() const;
    uint32_t getExp() const;
    uint8_t getPos() const;
    SkillType getType() const;
    bool isInjury() const;
    CenterType getCenter() const;
    uint16_t getMaxDistance() const;
    uint16_t getMaxHeroDistance() const;
    skill_kind getKind() const;
    uint32_t getCDTime() const;
    void clearCD(std::shared_ptr<PK> owner);
    bool checkCDTime() const;

    static bool checkTarget(std::shared_ptr<PK> atk, std::shared_ptr<PK> def, const TargetType& target_type);

private:
    bool checkCostMp(std::shared_ptr<PK> atk);
    void costMp(std::shared_ptr<PK> atk);
    bool checkReleaseJointSkill(std::shared_ptr<PK> atk, Coord2D pos);

    void insertEffect(SkillEffectBase::Ptr sptr);
    void deleteEffect(SkillEffectBase::Ptr sptr);
    void doEffect(std::shared_ptr<PK> atk, SkillEffectBase::Ptr sptr, SkillEffectEle& see);

public:
    static Skill::Ptr create(TplId id, uint32_t level=1);
    bool initCTData(TplId id, uint32_t level);
    
    //
    void addExp(uint32_t exp);
    void setExp(uint32_t exp);
    void setStrengthenLv(uint32_t level);
    void setPos(uint8_t pos);


    //技能升级
    bool checkUpgradeSkill(std::shared_ptr<PK> owner, const SkillBase::Ptr nextptr);
    //技能强化(错误码返回与升级共用)
    bool checkStrengthenSkill(std::shared_ptr<PK> owner, const SkillStrengthenBase::Ptr nextptr);
    //初始化技能强化效果
    void initStrengthenEffect();
    //删除强化效果
    void deleteStrengthenEffect();
    //初始化技能效果
    void initEffect();

    //技能释放入口
    AttackRetcode action(std::shared_ptr<PK> atk, const PublicRaw::RoleRequestAttack* cmd);

    //除skill_kind::passiveDamage, relive外的所有被动技能
    bool processNormalPassive(std::shared_ptr<PK> atk);

    //区分对象且附带伤害的被动技能(skill_kind::passiveDamage)
    bool processDamagePassive(std::shared_ptr<PK>);

    //死亡复活类技能
    bool processRelivePassive(std::shared_ptr<PK>);

    //处理被动技能
    bool processPassive(std::shared_ptr<PK>);

private:
    //主动技能分支
    AttackRetcode processActive(std::shared_ptr<PK> atk, const PublicRaw::RoleRequestAttack* cmd);


private:
    TplId       m_id;
    uint32_t    m_strengthenLevel;    //技能强化等级
    uint32_t    m_exp;                //经验值
    uint8_t     m_pos;               //技能快捷键位置
    std::vector<SkillEffectBase::Ptr> m_effectList; //效果列表(如果存在查找效率问题,可直接存SkillEffectBase::WPtr)
    SkillBase::Ptr m_skillB;

    water::componet::TimePoint m_lastUseTime;
    Range       m_changedRangeType; //改变的作用范围类型(特殊处理, 呕心的一毛线)
    uint16_t    m_changedRangeParam1;
    uint16_t    m_changedRangeParam2;

    uint32_t    m_changedPetId; //强化后的宠物ID(特殊处理)
    uint16_t    m_fireAddSec; //火墙增加的持续时间(秒)
};



//*****************************技能管理************************
class SkillManager final
{
public:
    explicit SkillManager(PK& me);
    ~SkillManager() = default;

public:
    void loadFromDB(std::vector<SkillData>& data);


public:
    bool initSkill(TplId id, uint32_t level=1);
    bool initSkill(const SkillData& data);

    Skill::Ptr find(TplId id);

    //功能开放开启合击技能
    void openJointSkill();

    //随等级解锁技能
    void unlockSkill(bool refresh = true);

private:
    void initSkillEffect(Skill::Ptr sp);
    void updateToDB(Skill::Ptr sp) const;

    bool addSkill(Skill::Ptr s);
    void removeSkill(Skill::Ptr s);
    void removeSkill(TplId id);

public:
    //技能列表数据下发
    void sendSkillListToMe() const;
    //单个技能刷新数据
    void refreshSkill(Skill::Ptr sp) const;
    void refreshSkill(TplId id) const;
    //技能升级
    void upgradeSkill(TplId id, std::shared_ptr<PK> owner, uint32_t upNum = 1, bool GmFlag = false);
    //技能强化
    void strengthenSkill(TplId id, std::shared_ptr<PK> owner);

    //处理被动技能
    bool processPassiveSkill(const skill_kind& kind);

    //被动技能cd
    bool checkPassiveSkillCD(TplId id, uint32_t cd);
    void updatePassiveSkillCD(TplId );
    void insertPassiveSkillCD(const PKCdStatus& status);

    //添加装备被动技能
    void putEquipPassiveSkill(std::vector<TplId>& ids);
    //离开场景需要缓存被动技能cd(蛋疼)
    void leaveScene();

private:
    PK& m_owner;

    //主动技能
    std::unordered_map<TplId, Skill::Ptr> m_activeSkill;

    //被动技能
    std::unordered_map<TplId, Skill::Ptr> m_passiveSkill;

    //装备被动技能cd保存(对策划无语)
    std::unordered_map<TplId, uint32_t> m_cdSkills;
};

}

#endif

