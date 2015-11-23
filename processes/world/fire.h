#ifndef PROCESS_WORLD_FIRE_H
#define PROCESS_WORLD_FIRE_H

/*
 * 火墙
 */

#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "water/componet/coord.h"
#include "water/componet/datetime.h"
#include "water/componet/class_helper.h"
#include "water/componet/fast_travel_unordered_map.h"
#include <vector>
#include <unordered_map>
#include <memory>

#include "protocol/rawmsg/public/fire_scene.h"

namespace world{

using namespace water;
using namespace water::componet;
class PK;
class Scene;
class Fire : public std::enable_shared_from_this<Fire>
{
public:
    TYPEDEF_PTR(Fire)
    CREATE_FUN_MAKE(Fire)

public:
    Fire(PKId id, std::shared_ptr<PK> owner);
    ~Fire() = default;

public:
    void setLifeTime(uint16_t addSec);
    void setOwnerAttr(const PKAttr& attr);
    bool checkTimeOut(const TimePoint& now) const;

public:
    PKId id() const;

    void setPos(Coord2D pos);
    Coord2D pos() const;

    void setSceneId(SceneId sceneId);
    std::shared_ptr<Scene> scene() const;

    void afterEnterScene() const;
    void leaveScene();

    void fillScreenData(PublicRaw::FireScreenData* data) const;

public:
    //生效
    void action();
    //伤害计算
    void computeFireDamage(std::shared_ptr<PK> def) const;

private:
    const PKId          m_id;
    std::weak_ptr<PK>   m_owner;
    SceneId             m_sceneId;
    Coord2D             m_pos;
    PKAttr              m_ownerAttr;
    TimePoint           m_lifeEnd = EPOCH;
    uint16_t            m_tick = 0; //滴答计时器
};

typedef water::componet::FastTravelUnorderedMap<PKId, Fire::Ptr> FireMap;


class FireManager final
{
private:
    FireManager();

public:
    ~FireManager() = default;


public:
    static FireManager& me();

    //召唤火墙
    void summonFires(std::shared_ptr<PK> owner, Coord2D center, uint16_t radius, uint16_t addSec, const PKAttr& attr);
    
    void erase(PKId ownerId);

    void regTimer();
    void timerLoop(const TimePoint& now);

private:
    std::unordered_map<PKId, std::vector<Fire::Ptr>> m_fires; //<主人ID>
    PKId        m_lastFireId = 0;
};

}

#endif

