#include "scene_object.h"
#include "scene.h"
#include "screen.h"
#include "scene_manager.h"
#include "role.h"

#include "water/common/commdef.h"

#include "protocol/rawmsg/public/object_scene.h"
#include "protocol/rawmsg/public/object_scene.codedef.public.h"

namespace world{

using namespace water::componet;

SceneObject::SceneObject(ObjBasicData info, uint16_t item, Bind bind, std::vector<RoleId> ownerVec, const TimePoint destoryOwnerTime)
: m_data(info)
, m_item(item)
, m_bind(bind)
, m_ownerVec(ownerVec)
, m_destoryOwnerTime(destoryOwnerTime)
, m_createTime(componet::Clock::now())
, m_eraseFlag(false)
, m_sceneId(0)
, m_pos(0,0)
{

}


std::string SceneObject::name() const
{
	return m_data.name;
}

ObjectId SceneObject::objId() const
{
	return m_data.objId;
}

TplId SceneObject::tplId() const
{
	return m_data.tplId;
}

BroadCast SceneObject::broadCast() const
{
    return m_data.broadCast;
}

uint16_t SceneObject::item() const
{
	return m_item;
}

Bind SceneObject::bind() const
{
	return m_bind;
}

uint32_t SceneObject::skillId() const
{
	return m_data.skillId;
}

uint8_t SceneObject::strongLevel() const
{
	return m_data.strongLevel;
}

ObjParentType SceneObject::parentType() const
{
	uint16_t type = SAFE_DIV(m_data.childType, 100) * 100;
	return (ObjParentType)type;
}

ObjChildType SceneObject::childType() const
{
	return (ObjChildType)m_data.childType;
}

uint16_t SceneObject::maxStackNum() const
{
	return m_data.maxStackNum;
}

const std::vector<RoleId>& SceneObject::ownerVec() const
{
	return m_ownerVec;
}


void SceneObject::setSceneId(SceneId sceneId)
{
	m_sceneId = sceneId;
}

SceneId SceneObject::sceneId() const
{
	return m_sceneId;
}

std::shared_ptr<Scene> SceneObject::scene() const
{
	return SceneManager::me().getById(m_sceneId);
}

void SceneObject::setPos(Coord2D pos)
{
	m_pos = pos;
}

Coord2D SceneObject::pos() const
{
	return m_pos;
}

void SceneObject::afterEnterScene()
{
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;

	enterVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void SceneObject::beforeLeaveScene()
{
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;

	leaveVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void SceneObject::enterVisualScreens(const std::vector<Screen*>& screens) const
{
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;

	//自己进入别人的视野
	PublicRaw::ObjectsAroundMe send;
	fillScreenData(&(send.objects[0]));
	s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(ObjectsAroundMe), &send, sizeof(send));
	LOG_TRACE("场景物品, 进入九屏, s->c, 成功, name={}, objId={}, tplId={}, item={}, pos={}, sceneId={}",
			  name(), objId(), tplId(), item(), pos(), sceneId());
}

void SceneObject::leaveVisualScreens(const std::vector<Screen*>& screens) const
{
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;
	
	//将自己从别人的视野中删除
	PublicRaw::ObjectLeaveInfo send;  
	send.objId[0] = objId(); 
	s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(ObjectLeaveInfo), &send, sizeof(send));
	LOG_TRACE("场景物品, 离开九屏, s->c, 成功, name={}, objId={}, tplId={}, item={}, pos={}, sceneId={}",
			  name(), objId(), tplId(), item(), pos(), sceneId());
}

void SceneObject::fillScreenData(PublicRaw::ObjectScreenData* data) const
{
	data->objId = objId();
	data->tplId = tplId();
	data->posX = pos().x;
	data->posY = pos().y;
}

void SceneObject::timerLoop(StdInterval interval, const water::componet::TimePoint& now)
{
	switch(interval)
	{
		case::StdInterval::sec_1:
			checkDestoryOwner(now);
			checkEraseObj(now);
			break;
		default:
			break;
	}
}

void SceneObject::checkDestoryOwner(const TimePoint& now)
{
	if(m_ownerVec.empty())
		return;

	if(now >= m_destoryOwnerTime)
	{
		m_ownerVec.clear();
	}

	return;
}

void SceneObject::checkEraseObj(const TimePoint& now)
{
	if(EPOCH == m_createTime)
		return;

	if(m_eraseFlag)
		return;

	if(now >= m_createTime + std::chrono::seconds {m_data.lifeSpan})
	{
		m_eraseFlag = true;
	}
	
	return;
}

void SceneObject::markErase()
{
    m_eraseFlag = true;
}

bool SceneObject::needErase() const
{
	return m_eraseFlag;
}

bool SceneObject::canPickUp(RoleId roleId) const
{
	if(m_ownerVec.empty())
		return true;

	for(uint32_t i = 0; i < m_ownerVec.size(); i++)
	{
		if(m_ownerVec[i] != roleId)
			continue;

		return true;
	}

	return false;
}

}
