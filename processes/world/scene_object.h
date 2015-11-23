/*
 * Author: zhupengfei 
 *
 * Created: 2015-05-30 +0800 
 *
 * Modified: 2015-05-30 +0800
 *
 * Description: 场景上掉落的物品
 */

#ifndef PROCESS_WORLD_SCENE_OBJECT_HPP
#define PROCESS_WORLD_SCENE_OBJECT_HPP

#include "position.h"

#include "water/common/objdef.h"
#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "water/componet/class_helper.h"
#include "water/componet/datetime.h"

#include "water/componet/fast_travel_unordered_map.h"

#include "protocol/rawmsg/public/object_scene.h"

#include <string>

namespace world{

class Scene;
class Screen;

using namespace water::componet;
using water::componet::TimePoint;

class SceneObject 	
{            	
public:
	TYPEDEF_PTR(SceneObject);
	
	SceneObject(ObjBasicData info, uint16_t item, Bind bind, std::vector<RoleId> ownerVec, const TimePoint destoryOwnerTime = EPOCH);
	
	~SceneObject() = default;

public:
	std::string name() const;

	ObjectId objId() const;

	TplId tplId() const;

	uint16_t item() const;

	Bind bind() const;

	uint32_t skillId() const;

	uint8_t strongLevel() const;

	ObjParentType parentType() const;

	ObjChildType childType() const;

	uint16_t maxStackNum() const;

	const std::vector<RoleId>& ownerVec() const;

    BroadCast broadCast() const;
	
public:
	void setSceneId(SceneId sceneId);
	SceneId sceneId() const;
	std::shared_ptr<Scene> scene() const;

	void setPos(Coord2D pos);
	Coord2D pos() const;

	void afterEnterScene();
	void beforeLeaveScene();

	void enterVisualScreens(const std::vector<Screen*>& screens) const;
	void leaveVisualScreens(const std::vector<Screen*>& screens) const;

	void fillScreenData(PublicRaw::ObjectScreenData* data) const;

public:
	void timerLoop(StdInterval interval, const water::componet::TimePoint& now); 

	void checkDestoryOwner(const TimePoint& now);

	void checkEraseObj(const water::componet::TimePoint& now);
    void markErase();
	bool needErase() const;

	bool canPickUp(RoleId roleId) const;

private:
	const ObjBasicData m_data;
	const uint16_t m_item;
	const Bind m_bind;
	std::vector<RoleId> m_ownerVec;    
	const TimePoint m_destoryOwnerTime;	//销毁物品归属权时间点

private:
    TimePoint m_createTime;
	bool m_eraseFlag;
	SceneId m_sceneId;
	Coord2D m_pos;
};

typedef water::componet::FastTravelUnorderedMap<ObjectId, SceneObject::Ptr> SceneObjMap;    

}


#endif
