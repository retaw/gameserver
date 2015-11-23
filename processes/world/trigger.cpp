#include "trigger.h"
#include "scene_manager.h"

#include "protocol/rawmsg/public/trigger_scene.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

Trigger::Trigger(TriggerType type, uint32_t triggerTplId, TriggerId id)
: m_triggerType(type)
, m_triggerTplId(triggerTplId)
, m_id(id)
, m_eraseFlag(false)
, m_createTime(Clock::now())
, m_lifetime((uint16_t)-1)
{
}

TriggerId Trigger::id() const
{
    return m_id;
}

TriggerType Trigger::triggerType() const
{
    return m_triggerType;
}

uint32_t Trigger::triggerTplId() const
{
    return m_triggerTplId;
}

void Trigger::setPos(Coord2D pos)
{
    m_pos = pos;
}

Coord2D Trigger::pos() const
{
    return m_pos;
}

void Trigger::setSceneId(SceneId sceneId)
{
    m_sceneId = sceneId;
}

Scene::Ptr Trigger::scene() const
{
    return SceneManager::me().getById(m_sceneId);
}

void Trigger::afterEnterScene() const
{
    auto s = scene();
    if(nullptr == s)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(32);
    buf.resize(sizeof(PublicRaw::TriggerAroundMe) + sizeof(PublicRaw::TriggerScreenData));
    auto msg = reinterpret_cast<PublicRaw::TriggerAroundMe*>(buf.data());
    fillScreenData(msg->data);
    msg->size = 1;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(TriggerAroundMe), buf.data(), buf.size(), pos());
}

void Trigger::beforeLeaveScene()
{
    auto s = scene();
    if(nullptr == s)
        return;

    std::vector<uint8_t> buf;
    buf.reserve(32);
    buf.resize(sizeof(PublicRaw::TriggerLeaveInfo) + sizeof(PublicRaw::TriggerScreenData));
    auto msg = reinterpret_cast<PublicRaw::TriggerLeaveInfo*>(buf.data());
    fillScreenData(msg->data);
    msg->size = 1;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(TriggerLeaveInfo), buf.data(), buf.size(), pos());
}

void Trigger::fillScreenData(PublicRaw::TriggerScreenData* data) const
{
    data->triggerTplId = triggerTplId();
    data->id = id();
    data->posx = pos().x;
    data->posy = pos().y;
    data->reserve = 0;
    switch(triggerType())
    {
    case TriggerType::box:
        {
            data->reserve = leftTime();
        }
        break;
    default:
        break;
    }
}

void Trigger::markErase()
{
    m_eraseFlag = true;
    beforeLeaveScene();
}

bool Trigger::needErase() const
{
    return m_eraseFlag;
}

void Trigger::setLifetime(uint16_t sec)
{
    if(0 == sec)
        return;
    m_lifetime = sec;
}

uint16_t Trigger::leftTime() const
{
    if((uint16_t)-1 == m_lifetime)
        return (uint16_t)-1;
    using namespace std::chrono;
    uint16_t existSec = duration_cast<seconds>(Clock::now() - m_createTime).count();
    return SAFE_SUB(m_lifetime, existSec);
}

}
