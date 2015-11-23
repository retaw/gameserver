#include "scene_object_manager.h"
#include "object_config.h"

#include "scene.h"
#include "scene_manager.h"
#include "world.h"
#include "water/componet/logger.h"

namespace world{

SceneObjectManager SceneObjectManager::m_me;

SceneObjectManager& SceneObjectManager::me()
{
	return m_me;
}


SceneObject::Ptr SceneObjectManager::getById(ObjectId objId)
{
    auto pos = m_sceneObjs.find(objId);
    if(pos == m_sceneObjs.end())
	{
		return nullptr;
	}

	return pos->second;
}

SceneObject::Ptr SceneObjectManager::createObj(TplId tplId, uint16_t item, Bind bind, std::vector<RoleId> ownerVec, const TimePoint destoryOwnerTime, const uint32_t skillId, const uint8_t strongLevel, const uint8_t luckyLevel)
{
	if(0 == item)
		return nullptr;

	if(bind != Bind::no && bind != Bind::yes)
		return nullptr;

	const auto& cfg = ObjectConfig::me().objectCfg;
    auto pos = cfg.m_objBasicDataMap.find(tplId);
    if(pos == cfg.m_objBasicDataMap.end())
    {
        LOG_TRACE("场景物品, object创建失败, tplId={}不存在", tplId);
        return nullptr;
    }

	if(item > pos->second.maxStackNum)
		return nullptr;

	m_lastSceneObjId += 1;
	ObjBasicData data;
	data = pos->second;
	data.objId = m_lastSceneObjId;
	data.skillId = skillId;
	data.strongLevel = strongLevel;
	data.luckyLevel = luckyLevel;

	SceneObject::Ptr obj = std::make_shared<SceneObject>(data, item, bind, ownerVec, destoryOwnerTime);
	if(obj == nullptr)
		return nullptr;

	if(!m_sceneObjs.insert(std::make_pair(obj->objId(), obj)))
		return nullptr;

    return obj;
}

void SceneObjectManager::regTimer()
{
    using namespace std::placeholders;
    World::me().regTimer(std::chrono::seconds(1), 
                         std::bind(&SceneObjectManager::timerLoop, this, StdInterval::sec_1, _1));
}

void SceneObjectManager::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
    for(auto pos = m_sceneObjs.begin(); pos != m_sceneObjs.end();)
	{
		if(pos->second == nullptr)
		{
			++pos;
			continue;
		}

        pos->second->timerLoop(interval, now);
		
		if(pos->second->needErase())
		{
			pos->second->beforeLeaveScene();
			
			SceneId sceneId = pos->second->sceneId();
			Scene::Ptr scene = SceneManager::me().getById(sceneId);
			if(scene != nullptr)
			{
				scene->eraseObj(pos->second);
			}
			pos = m_sceneObjs.erase(pos);
		}
		else
		{
			++pos;
		}
	}

	return;
}

//obj从场景中删除，但不直接在管理器中删除，管理器中所有obj从定时器中删除
void SceneObjectManager::eraseObj(ObjectId objId)
{
	auto pos = m_sceneObjs.find(objId);
	if(pos == m_sceneObjs.end())
		return;

	SceneObject::Ptr sceneObjPtr = pos->second;
	if(sceneObjPtr == nullptr)
		return;

	sceneObjPtr->beforeLeaveScene();

	SceneId sceneId = sceneObjPtr->sceneId();
	Scene::Ptr scene = SceneManager::me().getById(sceneId);
	if(scene != nullptr)
	{
		scene->eraseObj(pos->second);
	}

	//标记obj可删除
	sceneObjPtr->markErase();
	return;
}

}
