#ifndef PROCESS_WORLD_HORSE_H
#define PROCESS_WORLD_HORSE_H

#include "water/common/roledef.h"
#include "attribute.h"
#include "horse_base.h"
#include <set>

namespace world{

enum class RideState
{
    off     = 0,    //下马
    on      = 1,    //上马
};


class HorseLevelProp : public Attribute
{
public:
    explicit HorseLevelProp(const SceneItemType sceneItem);
    ~HorseLevelProp() = default;

public:
    void updateLevelProp(HorseTrainTpl::Ptr);

private:
    const SceneItemType m_sceneItem;
};


class HorseSkinProp : public Attribute
{
public:
    HorseSkinProp() = default;
    ~HorseSkinProp() = default;

public:
    void updateSkinProp(HorseSkinTpl::Ptr);
};



//==================== 华丽丽的分割线 ========================
class Role;
class Horse
{
public:
    explicit Horse(Role& me, uint8_t state);
    ~Horse() = default;

public:
    void initBasicData();
    void initSkinList();
    void initRaiseRate();

private:
    void setHorseTrainTpl();
    void initFirstSkin();
    bool isMaxStar() const;

    //暴击倍率相关处理
    uint8_t getRaiseRate() const;
    void addRaiseCount();
    void subRateEffectNum(uint8_t rate);

public:
    //开启坐骑功能
    void openHorse();
    bool isOpen() const;
    //返回培养界面信息
    void retRaiseInfo() const;
    //播放上下马九屏
    void broadcastRideTo9() const;
    //上马
    void onRide();
    //下马
    void offRide(bool isDead=false);
    //是否骑马状态
    bool isRide() const;

    //培养
    void raiseHorse(TplId objId, uint8_t autoyb);
    //返回培养结果
    void retRaiseResult(uint8_t retcode);
    //升星(阶)
    void upStar();
    //获得皮肤
    void activeSkin(uint16_t skin);
    //幻化
    void changeSkin(uint16_t skin);
    //返回已激活皮肤列表
    void retActiveSkins();

    //enterscene
    void afterEnterScene();
    //leavescene saveDB
    void leaveScene();
    //加载
    void loadFromDB(const std::string& horseStr);
    //保存db
    void saveDB();
    //跨天处理
    void zeroClear();

public:
    RideState rideState() const;
    void clearRideState();
    uint8_t star() const;
    uint32_t curskin() const;

public:
    uint32_t getMaxHp(const SceneItemType) const;
    uint32_t getHpRatio(const SceneItemType) const;
    uint32_t getHpLv(const SceneItemType) const;
    uint32_t getMaxMp(const SceneItemType) const;
    uint32_t getMpRatio(const SceneItemType) const;
    uint32_t getMpLv(const SceneItemType) const;

    uint32_t getPAtkMin(const SceneItemType) const;
    uint32_t getPAtkMax(const SceneItemType) const;
    uint32_t getMAtkMin(const SceneItemType) const;
    uint32_t getMAtkMax(const SceneItemType) const;
    uint32_t getWitchMin(const SceneItemType) const;
    uint32_t getWitchMax(const SceneItemType) const;
    uint32_t getPDefMin(const SceneItemType) const;
    uint32_t getPDefMax(const SceneItemType) const;
    uint32_t getMDefMin(const SceneItemType) const;
    uint32_t getMDefMax(const SceneItemType) const;

    uint32_t getLucky(const SceneItemType) const;
    uint32_t getEvil(const SceneItemType) const;
    uint32_t getShot(const SceneItemType) const;
    uint32_t getShotRatio(const SceneItemType) const;
    uint32_t getPEscape(const SceneItemType) const;
    uint32_t getMEscape(const SceneItemType) const;
    uint32_t getEscapeRatio(const SceneItemType) const;
    uint32_t getCrit(const SceneItemType) const;
    uint32_t getCritRatio(const SceneItemType) const;
    uint32_t getAntiCrit(const SceneItemType) const;
    uint32_t getCritDmg(const SceneItemType) const;

    uint32_t getDmgAdd(const SceneItemType) const;
    uint32_t getDmgAddLv(const SceneItemType) const; 
    uint32_t getDmgReduce(const SceneItemType) const;
    uint32_t getDmgReduceLv(const SceneItemType) const;

    uint32_t getAntiDropEquip(const SceneItemType) const;


private:
    uint8_t     m_open; //是否开启该功能
    bool        m_saveFlag;
    Role&       m_owner;
    RideState   m_rideState;
    uint8_t     m_star;     //星(阶)
    uint32_t    m_exp;
    HorseTrainTpl::Ptr m_horseTrainTpl;

    HorseLevelProp m_roleProps; //坐骑等级给主角加的属性
    HorseLevelProp m_heroProps; //坐骑等级给主角英雄加的属性
    HorseSkinProp  m_skinProps; //坐骑皮肤给主角增加的属性值

    uint16_t    m_curskin;        //当前使用的皮肤
    std::set<uint16_t> m_skinSet; //已经激活的皮肤列表

    std::unordered_map<uint8_t, std::pair<uint16_t, uint8_t>> m_raiseRate; //暴击数据集 key:暴击倍率  pair.first:暴击经验  pair.second:暴击作用次数
};

}

#endif
