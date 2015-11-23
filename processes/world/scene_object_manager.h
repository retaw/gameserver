/*
 * Author: zhupengfei
 *
 * Created: 2015-06-01 +0800
 *
 * Modified: 2015-06-01 +0800
 *
 * Description: 场景掉落物品管理
 */

#ifndef PROCESS_WORLD_SCENE_OBJECT_MANAGER_HPP
#define PROCESS_WORLD_SCENE_OBJECT_MANAGER_HPP

#include "scene_object.h"
#include "position.h"

#include "water/common/scenedef.h"

namespace world{

class SceneObjectManager 
{
public:
	~SceneObjectManager() = default;
	static SceneObjectManager& me();
private:
	static SceneObjectManager m_me;

public:
    SceneObject::Ptr getById(ObjectId objId);

    SceneObject::Ptr createObj(TplId tplId, uint16_t item, Bind bind, std::vector<RoleId> ownerVec, const TimePoint destoryOwnerTime = EPOCH, const uint32_t skillId = (uint32_t)-1, const uint8_t strongLevel = 0, const uint8_t luckyLevel = 0);

	//obj从场景中删除，但不直接在管理器中删除，管理器中所有obj从定时器中删除
	void eraseObj(ObjectId objId);

public:
    void regTimer();

private:
    void timerLoop(StdInterval interval, const water::componet::TimePoint& now);

private:
    ObjectId m_lastSceneObjId = 0;
	SceneObjMap m_sceneObjs;
};

#endif

}
