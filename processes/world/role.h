/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 13:43 +0800
 *
 * Modified: 2015-04-14 13:43 +0800
 *
 * Description: 角色
 */

#ifndef PROCESSES_WORLD_ROLE_H
#define PROCESSES_WORLD_ROLE_H

#include "water/componet/fast_travel_unordered_map.h"
#include "water/process/process_id.h"
#include "water/process/tcp_message.h"
#include "water/common/roledef.h"
#include "water/common/channeldef.h"
#include "water/common/scenedef.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

#include "protocol/rawmsg/private/faction.h"
#include "protocol/rawmsg/private/team.h"
#include "protocol/rawmsg/rawmsg_manager.h"

#include "position.h"
#include "package_set.h"
#include "pk.h"
#include "level_props.h"
#include "hero_manager.h"
#include "role_counter.h"
#include "attack_mode.h"
#include "mail.h"
#include "guanzhi.h"
#include "horse.h"
#include "title.h"
#include "wash.h"
#include "wing.h"
#include "dragon_ball.h"
#include "use_object.h"
#include "zhuansheng.h"
#include "fenjie.h"
#include "exp_area.h"
#include "role_sundry.h"
#include "function_icon.h"
#include "role_stall.h"
#include "task.h"
#include "factiontask.h"
#include "dragon_heart.h"
#include "daily_task.h"

#include <unordered_set>
#include "private_boss.h"

namespace world{

using water::process::ProcessIdentity;
using water::process::TcpMsgCode;


class Role : public PK
{
public:
    TYPEDEF_PTR(Role)
    CREATE_FUN_NEW(Role)

public:
    ~Role() = default;

    const std::string& account() const;

    std::string toString() const;

    //发送消息给自己的端
    bool sendToMe(TcpMsgCode msgCode) const override;
    bool sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const override;


    ProcessIdentity gatewayId();
    void setGatewayId(ProcessIdentity pid);

    void afterEnterScene() override;
    void beforeLeaveScene() override;
    void login();
    void offline();
	void retRoleIntoSceneToSession();

    void sendMainToMe() override;
    void syncScreenDataTo9() const override;

    void enterVisualScreens(const std::vector<Screen*>& screens) const override;
    void leaveVisualScreens(const std::vector<Screen*>& screens) const override;

    bool changePos(Coord2D pos, componet::Direction dir, MoveType type) override;
    void correctPosToMe();
    void syncNewPosTo9Screens(MoveType type) const override;

    void timerLoop(StdInterval interval, const componet::TimePoint& now) override;


public:
	void setDead(bool value);
    void dieDropEquip();

    bool isDead() const override;
    void toDie(PK::Ptr attacker) override;
	
    //打断操作
    void handleInterrupt(interrupt_operate op, SceneItemType interruptModel) override;

    uint16_t jointSkillReadyTime() const override;

    void lockOnTarget(PK::Ptr target);
    PK::Ptr target() const;

public:

    //初始化属性对象
    void initAttrMember();

    //db数据初始化技能
    void loadSkill(std::vector<SkillData>& data);
    //db数据初始化被动技能cd
    void loadPassiveSkillCD(std::vector<PKCdStatus>& cdstatus);
    //下发技能数据
    void sendSkillListToMe() const;
    //升级技能
    void upgradeSkill(uint32_t id, uint32_t upNum = 1, bool GmFlag = false);
    //强化技能
    void strengthenSkill(uint32_t id);
    
    //db loadbuff
    void loadBuff(std::vector<BuffData>& buffdata);
    //下发buff列表
    void sendBuffListToMe() const;
    //请求选中目标buff列表
    void reqSelectedBuff(PKId targetId, uint8_t sceneItem);
    //请求选中buff tips
    void reqSelectedBuffTips(PKId targetId, uint8_t sceneItem, uint32_t buffId);

    //在这里得到序列化的变长数据并给需要的成员统一使用
    void getAndInitVariableData(const PrivateRaw::RoleIntoScene* rev);

    void setTeamInfo(const PrivateRaw::UpdateTeamInfo* data);
    TeamId teamId() const;
    void setEnemy(std::unordered_set<RoleId>& enemy);
    void setBlack(std::unordered_set<RoleId>& black);
    bool inBlackList(const RoleId roleId) const;

    void setFaction(const PrivateRaw::UpdateFaction* rev);
    FactionId factionId() const;
    FactionPosition factionPosition() const;
    void addFaction(uint64_t factionExp, uint64_t factionResource, uint64_t banggong);
    void setFactionLevel(const uint32_t level);
    uint32_t factionLevel();

public:
    //请求进副本
    void intoCopyMap(MapId mapId);
    //请求退副本
    void exitCopyMap();

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
    void setLevel(uint32_t level) override;
	void setTurnLifeLevel(TurnLife level) override; 

	Sex sex() const;

private:
    Role(const PrivateRaw::RoleIntoScene* rev);

    void sendMapIdToMe() const;
    void fillBasicData(RoleBasicData* data) const;
    void fillScreenData(RoleScreenData* data) const;

    //跨天处理
    void dealNotSameDay();

    void underAttack(PK::Ptr atk) override;
public:
	void fillMainData(RoleMainData* data) const;

public:	
	void setDeathTimePoint(const water::componet::TimePoint& dieTimePoint);
    water::componet::TimePoint getDeathTimePoint() const;

	void addExp(uint64_t exp);
	uint64_t getExp() const;

	void judgeLevelUp();
	void levelUp(uint32_t upNum = 1, bool GmFlag = false);

	bool updateLevelAndExpToDB();
	void sendRoleCurLevelGotExp();

	//当前等级已获得的经验值 == 累计经验值 - 当前等级需要经验值
	uint64_t getCurLevelGotExp() const;

	//升级需要经验值 == 下一级需要经验值 - 当前等级需要经验值
	uint64_t getLevelUpNeedExp() const;

	//请求复活角色
	void requestRelive(const uint8_t* msgData, uint32_t msgSize);

private:
	uint32_t getRoleCanLevelUpNum() const;
	void updateFuncAndSessionRoleLevel();

	bool updateTurnLifeLevelToDB();

	void autoAddHpAndMp();

private:
	void syncRoleDie(PK::Ptr attacker);

	//检查是否可以自动回城复活
	bool checkAutoRelive(const water::componet::TimePoint& now);

	//复活点复活(地图没有复活点, 回主城复活)
	void reliveAreaRelive(bool autoRelive = false);

    //主城复活
    void goBackCityRelive();
	
	//完美原地复活
	void perfectRelive();

	//虚弱原地复活
	void weakRelive();

    //通知func添加仇人
    void addEnemy(PK::Ptr attacker);


public:
	void relive(const uint32_t percent = 100);

	//到本地图默认复活点复活
	void reliveByDefaultReliveArea(bool autoRelive = false);


private:
	TplId getTplIdByObjChildType(ObjChildType childType) const;

public:
	std::string getMoneyName(MoneyType type) const;
	uint64_t getMoney(MoneyType type) const;
	bool checkMoney(MoneyType type, uint64_t needMoney);

	template<typename... Args>    
	bool addMoney(MoneyType type, uint64_t money, const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		return addMoneyPrivate(type, money, text);
	}
	
	template<typename... Args>    
	bool reduceMoney(MoneyType type, uint64_t money, const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		return reduceMoneyPrivate(type, money, text);
	}

	bool checkMoneyByObj(TplId tplId, uint16_t objNum);
    bool checkMoneyByObjs(const std::vector<std::pair<uint32_t, uint16_t> >& objVec);
	//一键自动扣除道具对应的元宝数量
    bool autoReduceObjMoney(TplId tplId, uint16_t objNum, const std::string& text);

    bool checkBanggong(uint64_t num);
    bool reduceBanggong(uint64_t num, const std::string& text);
    uint64_t banggong();
    void setBanggongWithoutSysn(const uint64_t bangong);
    void setBanggong(const uint64_t banggong);
    

private:
	bool addMoneyPrivate(MoneyType type, uint64_t money, const std::string& text);
	bool reduceMoneyPrivate(MoneyType type, uint64_t money, const std::string& text);
	//扣绑定金币时，先扣绑定金币，再扣非绑金币(特殊处理)
	bool reduceGoldCoin(MoneyType type, uint64_t money, const std::string& text);

	bool setMoney(MoneyType type, uint64_t money);
	bool updateMoneyToDB(MoneyType type);

    void synBanggong();

public:
	template<typename... Args>    
	void sendSysChat(const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		sendSysChatPrivate(ChannelType::system_msgbox, text);
		return;
	}

	template<typename... Args>    
	void sendSysChat(ChannelType type, const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		sendSysChatPrivate(type, text);
		return;
	}

private:
	void sendSysChatPrivate(ChannelType type, const std::string& text);
	void sendSysMsgToMe(ChannelType type, const std::string& text);
	void sendSysNotifyToGlobal(ChannelType type, const std::string& text);

public:
    //商城道具每日购买处理
    void addBuyDayLimit(uint32_t objId, uint16_t num);
    uint16_t getBuyDayLimit(uint32_t objId) const;

public:
	void autoAddTotalOnlineSec();
	uint32_t getTotalOnlineSec() const;

	void autoAddCurObjId();
	uint64_t getCurObjId() const;

	void setDefaultCallHero(Job job);
	Job getDefaultCallHero() const;

private:
	void updateDefaultCallHeroToDB();

public:
    uint8_t nameColor() const;
    //设置名称颜色 如红名/黄名等
    void setNameColor(name_color, bool sync=true);
    //clear
    void clearNameColor(name_color);
    //check
    bool issetNameColor(name_color);
    //update grey time
    void updateGreynameTime();
    void checkAndClearGreyNameColor(const componet::TimePoint& now);
    uint32_t offlineTime() const;
    TimePoint greynameTime() const;

public:
	bool checkPutObj(const std::vector<ObjItem>& objVec);
	bool checkPutObj(TplId tplId, uint32_t num, Bind bind, const PackageType packageType = PackageType::role);

	//返回放入背包失败的物品集合
	std::vector<ObjItem> putObj(const std::vector<ObjItem>& objVec);
	uint32_t putObj(TplId tplId, uint32_t num, Bind bind, const PackageType packageType = PackageType::role, const uint32_t skillId = (uint32_t)-1, const uint8_t strongLevel = 0, const uint8_t luckyLevel = 0);

	bool eraseObj(TplId tplId, uint16_t num, PackageType packageType, const std::string& text);
	bool eraseObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType, const std::string& text);
	Object::Ptr eraseObjByCell(uint16_t cell, PackageType packageType, const std::string& text);
	Object::Ptr eraseObjByCell(uint16_t cell, uint16_t num, PackageType packageType, const std::string& text);

	uint16_t getObjNum(TplId tplId, const PackageType packageType = PackageType::role) const;   
	uint16_t getObjNumByCell(uint16_t cell, const PackageType packageType = PackageType::role) const;
	Object::Ptr getObjByCell(uint16_t cell, const PackageType packageType = PackageType::role) const;
	Bind getBindByCell(uint16_t cell, const PackageType packageType = PackageType::role) const;

	void setSummonHeroFlag(bool flag);
	bool getSummonHeroFlag() const;

	PackageType getPackageTypeByHeroJob(Job job) const;
	Job getHeroJobByPackageType(PackageType packageType) const;
	Hero::Ptr getHeroByJob(Job job) const;

	void addTitle(uint32_t typeId);
	uint32_t getUsedTitleIdByType(TitleType type) const;

	void setBufferData(const std::vector<uint16_t>& bufferVec);
    void loadBufferData(const std::string& bufferStr);

    //龙心能量上限
    uint16_t energeLimit() const;
    //增加龙心能量
    bool addEnerge(uint16_t energe);
    //check
    bool checkEnerge(uint16_t energe) const;
    //扣除龙心能量
    void subEnerge(uint16_t energe);
    //请求充能
    void requestAddEnerge(uint8_t autoyb);
    //更新龙心能量
    void retEnerge() const;

    //怒气
    bool checkAnger() const;
    void addAnger(uint16_t num);
    void subAnger();
    void refreshAnger() const;

    //传送门
    void transferByTriggerDoor(uint32_t triggerTplId);
    //世界boss宝箱拾取
    void pickupWorldBossBox();
    //清除世界boss宝箱持有状态
    bool clearWorldBossBoxStatus();
    //判断世界boss宝箱归属
    void judgeBoxBelong();
    //持有宝箱倒计时
    uint16_t holdBoxLeftTime() const;


    //摆摊
    void setStall();
    bool clearStall();
    bool isStall() const;

    //采集
    void startCollect(PKId npcId);
    //采集中断
    void interruptCollect();
    //完成采集
    void finishCollect();

private:
	void sendBufferDataToMe();
	void updateBufferDataToDB();

public:
	uint32_t getMinStrongLevel() const;
	uint32_t getStoneTotalLevel() const;

	bool inArea(AreaType type);

	uint8_t vipLevel() const;
    uint32_t mohun() const;

    SceneId preSceneId() const;
    Coord2D prePos() const;

public:
	using PosChangeEvent = Event<void (Role::Ptr role, Coord2D oldPos, Coord2D newPos)>;
	PosChangeEvent e_onPosChanged;

private:
    const std::string m_account;
    const Sex m_sex;

private:
	uint64_t m_money_1;	//绑定金币
	uint64_t m_money_2;	//非绑金币
	uint64_t m_money_3;	//绑定元宝
	uint64_t m_money_4;	//非绑元宝
	uint64_t m_money_5;	//声望
	uint64_t m_money_6;	//强化值
	uint64_t m_money_7;	//战功
	uint64_t m_money_8;	//角色灵力值
	uint64_t m_money_9;	//英雄灵力值
	uint64_t m_money_10;//龙魂

private:
	uint64_t	m_curObjId;					
	uint64_t	m_exp;
    bool        m_dead;
    componet::TimePoint m_deathTimePoint;	//角色死亡时间点
	uint32_t	m_totalOnlineSec;			//角色累计在线时间
    uint32_t    m_offlineTime;				//上一次离线时间
    componet::TimePoint m_greynameTime;		//设置灰名时间
	Job			m_defaultCallHero;			//默认召唤的英雄
	bool		m_summonHeroFlag;			//是否召唤英雄标识，仅用于跨场景
    uint8_t     m_nameColor;			

    uint16_t    m_anger;                    //合击怒气
    uint8_t     m_stall;                    //摆摊状态
    PKId        m_collectNpcId;
    componet::TimePoint m_collectTp;        //开始采集时间

public:
    componet::TimePoint m_holdBoxTime;      //持有世界boss宝箱开始时间


public:
	PackageSet  m_packageSet;
    LevelProps  m_levelProps;
	HeroManager m_heroManager;
    RoleCounter m_roleCounter;
    AttackMode  m_attackMode;
    Mail        m_mail;
	Guanzhi     m_guanzhi;
    Horse       m_horse;
	Title		m_title;
	Wash		m_wash;
	Wing		m_wing;
	DragonBall  m_dragonBall;
	UseObject   m_useObject;
	Zhuansheng	m_zhuansheng;
	Fenjie		m_fenjie;
    DragonHeart m_dragonHeart;
	ExpArea		m_expArea;
    RoleStall   m_roleStall;
	DailyTask   m_dailyTask;

public:
    //宠物数据
    PKAttr      m_petPoisonAttr;
    std::vector<BuffData> m_petBuffs;

public:
    //任务
    void dispatchTask(TaskContent content, TaskParam param);
    RoleTask    m_roleTask;
    RoleFactionTask m_roleFactionTask;


private:
	ProcessIdentity m_gatewayId;
    SceneId     m_preSceneId;
    Coord2D     m_prePos;

public:
    PrivateBoss m_privateBoss;

private:
	//社会关系
    TeamId m_teamId = 0;
    FactionId m_factionId = 0;
    std::string m_factionName;
    FactionPosition m_factionPosition = FactionPosition::none;
    uint32_t m_factionLevel = 0;
    std::unordered_set<RoleId> m_enemy;
    std::unordered_set<RoleId> m_blackList;

private:
    std::vector<Attribute*> attrMembers;

	std::vector<uint16_t> m_bufferVec;		//角色的缓存数据(供客户端使用)
    uint64_t m_banggong = 0;

    uint32_t m_vipLevel = 0;
    uint32_t m_mohun = 0;
    PK::WPtr m_target;


//杂项数据
public:
    RoleSundry  m_roleSundry;

};

typedef water::componet::FastTravelUnorderedMap<RoleId, Role::Ptr> RoleMap;

Role::Ptr getRole(PK::Ptr me);

}


#endif
