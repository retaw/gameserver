/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-08 19:37 +0800
 *
 * Modified: 2015-04-08 19:37 +0800
 *
 * Description: 
 */

#ifndef PROCESS_SCENE_PK_H
#define PROCESS_SCENE_PK_H

#include "water/componet/class_helper.h"
#include "water/componet/logger.h"
#include "water/common/commdef.h"
#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "water/process/tcp_message.h"
#include "water/componet/datetime.h"
#include "position.h"
#include "skill_manager.h"
#include "skill_effect_manager.h"
#include "buff_manager.h"
#include "pk_state.h"

#include "protocol/rawmsg/public/role_pk.h"
#include "protocol/rawmsg/public/role_pk.codedef.public.h"

namespace world{


using water::process::TcpMsgCode;
using namespace water::componet;


class Pet;
class Scene;
class Screen;
class PK : public std::enable_shared_from_this<PK> 
{
public:
    explicit PK(PKId id, const std::string& name, const Job& job, const SceneItemType& sceneItem);
    TYPEDEF_PTR(PK)

    virtual ~PK() = default;

public:
    PKId id() const;
    const std::string& name() const;
    const Job& job() const;
    SceneItemType sceneItemType() const;

    virtual void setLevel(uint32_t level);
    uint32_t level() const;

	virtual void setTurnLifeLevel(TurnLife level);
	TurnLife turnLifeLevel() const;
    
	void setDir(componet::Direction dir);
    componet::Direction dir() const;

    //获取主人相关数据
    virtual PKId getOwnerId() const;
	virtual SceneItemType getOwnerSceneItemType() const;
    virtual Job getOwnerJob() const;


    void setPos(Coord2D pos);
    Coord2D pos() const;
    Coord2D backPos() const;

    void setSceneId(SceneId id);
    SceneId sceneId() const;
    std::shared_ptr<Scene> scene() const;

    virtual void afterEnterScene() = 0;
    virtual void beforeLeaveScene() = 0;

    virtual void enterVisualScreens(const std::vector<Screen*>& screens) const = 0;
    virtual void leaveVisualScreens(const std::vector<Screen*>& screens) const = 0;

    virtual bool changePos(Coord2D newPos, componet::Direction dir, MoveType type) = 0;
    virtual void syncNewPosTo9Screens(MoveType type) const = 0;

    virtual void timerLoop(StdInterval interval, const water::componet::TimePoint& now);


public:
    //简易的发起攻击入口
    AttackRetcode launchAttack(uint32_t skillId, Coord2D center);

    //攻击发起入口
    AttackRetcode launchAttack(const PublicRaw::RoleRequestAttack* rev);
    
    //攻击伤害的计算
    void attackMe(PK::Ptr atk, uint8_t hit, const skill_kind& kind, uint8_t continued);

    //自身生命值变化九屏(被攻击者调用)
    void changeHpAndNotify(PK::Ptr atk, int32_t changeVal, HPChangeType type, uint8_t continued=1);

    //自身魔法值变化九屏
    void changeMpAndNotify(int32_t changeVal, bool noticeNine=false);

    //攻击动作成功返回
    void retAttackCmdTo9(componet::Direction dir, uint32_t skillId, uint32_t skillLevel, Coord2D pos);

    //攻击失败
    void retAttackFail(uint32_t skillId, AttackRetcode ret);

    //冲锋入口
    void chongfeng(TplId skillId, uint32_t lv, Direction dir, uint32_t cell);

    //技能释放之前需要清除的一些数据
    void beforeSkillActionClear();

    //技能释放后需要清除的一些数据
    void afterSkillActionClear(TplId skillId);

    //伤害触发处理函数
    void onDamage(PK::Ptr atk, int32_t DM);

    //设置上一次攻击成功的技能id
    void setLastAttackSuccessSkill(TplId skillId);
    uint32_t lastAttackSuccessSkill() const;

private:
    //个人技能伤害计算
    HPChangeType calcSkillDamage(PK::Ptr atk, uint8_t hit, const skill_kind& kind, int32_t* DM);

    //合击技能伤害计算
    HPChangeType calcJointSkilldamage(PK::Ptr atk, uint8_t hit, const skill_kind& kind, int32_t* DM);

    bool isShot(PK::Ptr atk, const skill_kind& kind);
    uint64_t calcFinalAtt(PK::Ptr atk, const skill_kind& kind);
    uint64_t calcFinalDef(PK::Ptr atk, const skill_kind& kind);
    uint64_t calcFinalCritdmg(PK::Ptr atk);
    uint64_t calcFinalDmgadd(PK::Ptr atk, bool joint);

    virtual void underAttack(PK::Ptr atk); //被攻击事件

public:
    //合击技能准备时长
    virtual uint16_t jointSkillReadyTime() const;
    //进入合击准备状态
    void setJointReadyState();
    void clearJointReadyState();
    bool isJointReadyState() const;
    bool canJointSkillReady(const std::pair<Job, Job>& jobs);


public:
    //施毒术技能伤害计算(特殊, 近似于离线数据pk)
    void computePoisonSkillDamage();

    //缓存中毒时攻击者属性到db
    void cachePoisonAttrDB(RoleId roleId) const;


public:
    //消息发送接口
    virtual bool sendToMe(TcpMsgCode msgCode) const;
    virtual bool sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const;

    //自己9屏数据同步9屏
    virtual void syncScreenDataTo9() const;
    //
    virtual void sendMainToMe();

    void chongfengMsgTo9(TplId skillId, uint32_t level, PK::Ptr atk, Direction dir, MoveType type, Coord2D newPos);



public:
    //数值属性接口
    virtual uint32_t getMaxHp() const { return 0; }
    virtual uint32_t getHpRatio() const { return 0; }              //生命加成
    virtual uint32_t getHpLv() const { return 0; }                 //生命等级
    virtual uint32_t getMaxMp() const { return 0; }
    virtual uint32_t getMpRatio() const { return 0; }              //魔法加成
    virtual uint32_t getMpLv() const { return 0; }                 //魔法等级

    virtual uint32_t getTotalPAtkMin() const { return 0; }         //物攻min
    virtual uint32_t getTotalPAtkMax() const { return 0; }         //物攻max
    virtual uint32_t getTotalMAtkMin() const { return 0; }         //魔法min
    virtual uint32_t getTotalMAtkMax() const { return 0; }         //魔法max
    virtual uint32_t getTotalWitchMin() const { return 0; }        //道术min
    virtual uint32_t getTotalWitchMax() const { return 0; }        //道术max
    virtual uint32_t getTotalPDefMin() const { return 0; }         //物防min
    virtual uint32_t getTotalPDefMax() const { return 0; }         //物防max
    virtual uint32_t getTotalMDefMin() const { return 0; }         //魔防min
    virtual uint32_t getTotalMDefMax() const { return 0; }         //魔防max
    virtual uint32_t getTotalLucky() const { return 0; }           //幸运
    virtual uint32_t getTotalEvil() const { return 0; }            //诅咒

    virtual uint32_t getTotalShot() const { return 0; }            //命中
    virtual uint32_t getTotalShotRatio() const { return 0; }       //命中率
    virtual uint32_t getTotalPEscape() const { return 0; }         //物闪
    virtual uint32_t getTotalMEscape() const { return 0; }         //魔闪
    virtual uint32_t getTotalEscapeRatio() const { return 0; }     //闪避率
    virtual uint32_t getTotalCrit() const { return 0; }            //暴击
    virtual uint32_t getTotalCritRatio() const { return 0; }       //暴击率
    virtual uint32_t getTotalAntiCrit() const { return 0; }        //防暴
    virtual uint32_t getTotalCritDmg() const { return 0; }         //暴伤

    virtual uint32_t getTotalDmgAdd() const { return 0; }          //增伤
    virtual uint32_t getTotalDmgReduce() const { return 0; }       //减伤
    virtual uint32_t getTotalDmgAddLv() const { return 0; }        //增伤等级
    virtual uint32_t getTotalDmgReduceLv() const { return 0; }     //减伤等级

    //防爆装备属性
    virtual uint32_t getTotalAntiDropEquip() const { return 0; }
    //...
    

public:
    virtual void setSpeed(uint32_t speed);
    uint32_t getSpeed() const;


    bool changeHp(int32_t dwHp);
    bool changeMp(int32_t dwMp);
    void setHp(uint32_t dwHp);
    void setMp(uint32_t dwMp);
    void resetHpMp();
    uint32_t getHp() const;
    uint32_t getMp() const;



public:
    virtual void toDie(PK::Ptr atk) = 0;
    virtual bool isDead() const = 0;
    //假死
    bool isFeignDead() const;
    void setFeignDead();
    void clearFeignDead();
    void setFeignDeadRelivePercent(uint32_t percent) ;
    uint32_t feignDeadRelivePercent() const;
    bool dealFeignDead();


public:
    void markErase(bool flag);
    bool needErase() const;



public:
    bool checkPublicCDTime() const;
    void setSkillPublicTime();
    TimePoint getSkillPublicTime() const;
    void clearSkillCD(uint32_t skillId);

    uint32_t shotk() const;
    uint32_t critk() const;
    uint32_t constDamage(PK::Ptr atk, bool joint);

public:
    static PK::Ptr getPkptr(PKId id, SceneItemType sceneItem);


public:
    void updateHpChangeList(PK::Ptr def, int32_t value, HPChangeType type);
    void clearHpChangeList();

    //非法攻击设置灰名
    void setGreyName(PK::Ptr def);
    //同步九屏名字颜色
    void syncNameColorTo9(uint8_t);
    //加罪恶值
    void addEvil(PK::Ptr deadPK);
    //是否友方
    bool isFriend(PK::Ptr atk);

    //判断两个对象是否敌对关系
    static bool isEnemy(PK::Ptr atk, PK::Ptr def);

    //是否战斗状态
    bool isFight();
    //更新战斗时间点
    void updateFightTime();
    //清除隐身
    void unHide();


    //宠物
    void setPetTplId(TplId petTplId);
    void setPetSkillId(uint32_t petSkillId);    //召唤术技能id
    void setPetLevel(uint32_t level);           //召唤术技能等级
    void setPetId(PKId petId);
    PKId petId() const;
    TplId petTplId() const;
    uint32_t petSkillId() const;
    uint32_t petLevel() const;
    std::shared_ptr<Pet> pet() const;


    //打断事件
    virtual void handleInterrupt(interrupt_operate op, SceneItemType interruptModel);


public:
    SkillManager    m_skillM;
    SkillEffectMgr  m_skillEffectM;
    PKValue         m_pkvalue;
    PKState         m_pkstate;
    BuffManager     m_buffM;



private:
    const PKId      m_id;         //对象唯一ID
    const std::string m_name;
    const Job       m_job;
    const SceneItemType m_sceneItem;//pk对象在场景中的类型
    
    //uint8_t         m_dir;
    componet::Direction m_dir;
    uint32_t		m_level;
	uint32_t        m_speed;
	TurnLife		m_turnLifeLevel;	//转生等级
    Coord2D         m_pos;
    SceneId         m_sceneId;
    TimePoint       m_lastSkillTime;//之前一次释放技能的时间点

    uint32_t        m_hp;
    uint32_t        m_mp;

    uint32_t        m_shotk;
    uint32_t        m_critk;

private:
    //战士刺杀参与最后伤害计算的有效攻击需要缓存,用来计算第二格(呕心)
    uint64_t        m_tempFinalAtk;
    bool            m_feignDead;
    uint32_t        m_feignDeadRelivePercent;

    TimePoint       m_fightTime; //战斗状态最新时间点
    PKId            m_petId; //宠物ID
    TplId           m_petTplId;
    uint32_t        m_petSkillId;
    uint32_t        m_petLevel;

    bool            m_eraseFlag; //从场景删除标志
    TimePoint       m_jointStart; //合击准备状态开始

    uint32_t        m_lastAttackSuccessSkill; //上一次播放攻击成功动作的技能id

public:
    //反弹(物理伤害百分比)
    //uint32_t        m_reflectPdamagePer;

    //反弹(魔法伤害百分比)
    //uint32_t        m_reflectMdamagePer;

    //中毒时攻击者属性缓存
    PKAttr          m_poisonAttackerAttr;
    //发起一次攻击时缓存的伤害列表
    std::vector<std::pair<PK::Ptr, DamageUnit>> m_attackHpChangeList;

    //以目标中心点范围性击退目标缓存
    std::vector<PK::Ptr> m_rangeHitBackPK;

public:
    void notifyDoubleSkillEffect(TplId skillId);
    bool issetDoubleDamageSkillLogic(TplId skillId, uint32_t logic_id) const;
    bool isDoubleEffect(TplId skillId) const;
    void eraseDoubleDamageSkillLogic(TplId skillId, uint32_t logic_id);
    //触发两次攻击伤害的技能种类集合 <std::pair<技能id, logic_id>>
    std::map<std::pair<TplId, uint32_t>, uint16_t> m_doubleDamageSkillLogic;
    uint16_t        m_doubleDamageFlag;
    uint32_t        m_doubleDamageLogic;

    //
    bool            m_demaxiya; //gm必杀技
    bool            m_wudi;     //gm无敌状态
};


}

#endif

