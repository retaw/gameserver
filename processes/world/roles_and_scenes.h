/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-16 15:28 +0800
 *
 * Modified: 2015-04-16 15:28 +0800
 *
 * Description:  处理角色和场景关系 相关的逻辑
 */

#ifndef PROCESS_WORLD_ROLE_AND_SCENE_H
#define PROCESS_WORLD_ROLE_AND_SCENE_H

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include "position.h"

#include <cstdint>
#include <unordered_map>
#include "scene.h"


namespace world{

using water::process::ProcessIdentity;

class RolesAndScenes
{
public:
	static RolesAndScenes& me();

public:
    void regMsgHandler();
    
private:
    void servermsg_RoleIntoScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
	void servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize);
	void servermsg_SessionReqTargetRoleScenePos(const uint8_t * msgData, uint32_t msgSize, ProcessIdentity rid);
	void servermsg_SessionFuncRoleReqChangeScene(const uint8_t * msgData, uint32_t msgSize);

	void clientmsg_GotoOtherSceneByTransmission(const uint8_t * msgData, uint32_t msgSize, uint64_t rid);
	void clientmsg_RequestFeixieTransfer(const uint8_t * msgData, uint32_t msgSize, uint64_t rid);
	void clientmsg_RoleIntoCopyMap(const uint8_t * msgData, uint32_t msgSize, uint64_t rid);
	void clientmsg_RoleLeaveCopyMap(const uint8_t * msgData, uint32_t msgSize, uint64_t rid);

private:
    //下线进场景前，如果是进入特定场景（boss之家）需要检查是否下线超过一定时间
    void changeSceneByofflnTimeAndCopyMap(Scene::Ptr& scene, SceneId& sceneId, Coord2D& pos, const SceneId& preSceneId, const Coord2D& prePos, uint32_t offlnTime);
public:
	bool gotoOtherScene(RoleId roleId, SceneId newSceneId, Coord2D newPos);

    uint32_t m_offlnKeepRedution = 60*3;    //先写死,个别地图下线超过一定时间后再上线会回到之前的普通地图,changeSceneByofflnTimeAndCopyMap中使用

};

}

#endif
