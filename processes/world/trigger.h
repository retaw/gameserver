#ifndef PROCESS_WORLD_TRIGGER_H
#define PROCESS_WORLD_TRIGGER_H

/*
 *
 * 场景机关物件
 *
 */
#include "trigger_cfg.h"

#include "water/common/scenedef.h"
#include "water/componet/coord.h"
#include "water/componet/datetime.h"
#include "water/componet/class_helper.h"
#include "water/componet/fast_travel_unordered_map.h"

#include "protocol/rawmsg/public/trigger_scene.h"
#include <memory>

namespace world{

using namespace water;
using namespace water::componet;

class Scene;
class Trigger : public std::enable_shared_from_this<Trigger>
{
public:
    TYPEDEF_PTR(Trigger)
    CREATE_FUN_MAKE(Trigger)

public:
    Trigger(TriggerType type, uint32_t triggerTplId, TriggerId id);
    ~Trigger() = default;

public:
    TriggerId id() const;
    TriggerType triggerType() const;
    uint32_t triggerTplId() const;

    void setPos(Coord2D pos);
    Coord2D pos() const;

    void setSceneId(SceneId sceneId);
    std::shared_ptr<Scene> scene() const;

    void afterEnterScene() const;
    void beforeLeaveScene();

    void fillScreenData(PublicRaw::TriggerScreenData* data) const;

    void markErase();
    bool needErase() const;
    void setLifetime(uint16_t sec);
    uint16_t leftTime() const;


private:
    const TriggerType   m_triggerType;
    const uint32_t      m_triggerTplId;
    const TriggerId     m_id;
    SceneId             m_sceneId; 
    Coord2D             m_pos;
    bool                m_eraseFlag;
    TimePoint           m_createTime;  //机关创建时间
    uint16_t            m_lifetime; //生命周期(秒)
};

typedef water::componet::FastTravelUnorderedMap<TriggerId, Trigger::Ptr> TriggerMap;
}

#endif
