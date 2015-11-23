#include "pk_state.h"
#include "pk.h"
#include "world.h"
#include "scene.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/pk_state.h"
#include "protocol/rawmsg/public/pk_state.codedef.public.h"


namespace world{

using namespace water;
using namespace water::componet;

//=====================================================
PKState::PKState(PK& me)
    : m_owner(me)
    , m_status(0)
{
}


uint32_t PKState::pkstatus() const
{
    return m_status;
}

void PKState::loadStatus(uint32_t status)
{
    m_status |= status;
}

void PKState::loadStatus(visual_status status)
{
    m_status |= static_cast<uint32_t>(status);
}

bool PKState::issetStatus(visual_status status) const
{
    return m_status & static_cast<uint32_t>(status);
}

bool PKState::issetStatus(uint32_t status) const
{
    return m_status & status;
}

bool PKState::setStatus(uint32_t status)
{
    if(0 == status || issetStatus(status))
        return false;

    m_status |= status;
    showStatusTo9(status);
    return true;
}

bool PKState::setStatus(visual_status status)
{
    if(visual_status::none == status || issetStatus(status))
        return false;

    m_status |= static_cast<uint32_t>(status);
    showStatusTo9(static_cast<uint32_t>(status));
    return true;
}

bool PKState::clearStatus(visual_status status)
{
    if(visual_status::none == status || !issetStatus(status))
        return false;
    m_status &= ~static_cast<uint32_t>(status);
    unshowStatusTo9(static_cast<uint32_t>(status));
    return true;
}


bool PKState::clearStatus(uint32_t status)
{
    if(0 == status || !issetStatus(status))
        return false;
    m_status &= ~status;
    unshowStatusTo9(status);
    return true;
}


void PKState::showStatusTo9(uint32_t status) const
{
	Scene::Ptr s = m_owner.scene();
	  if(s == nullptr)
		  return;

    PublicRaw::ShowPKStatusTo9 send;
    send.id = m_owner.id();
    send.sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    send.status = status;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(ShowPKStatusTo9), &send, sizeof(send), m_owner.pos());
}


void PKState::unshowStatusTo9(uint32_t status) const
{
	Scene::Ptr s = m_owner.scene();
	  if(s == nullptr)
		  return;

    PublicRaw::UnshowPKStatusTo9 send;
    send.id = m_owner.id();
    send.sceneItem = static_cast<uint8_t>(m_owner.sceneItemType());
    send.status = status;
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(UnshowPKStatusTo9), &send, sizeof(send), m_owner.pos());
}

}




