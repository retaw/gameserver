#include "roles_and_scenes.h"


#include "world.h"
#include "role_manager.h"
#include "scene_manager.h"

#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"


#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"

#include "protocol/rawmsg/public/role_scene.h"
#include "protocol/rawmsg/public/role_scene.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <functional>

namespace world{

RolesAndScenes& RolesAndScenes::me()
{
    static RolesAndScenes me;
    return me;
}

void RolesAndScenes::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(RoleIntoScene, std::bind(&RolesAndScenes::servermsg_RoleIntoScene, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RoleOffline, std::bind(&RolesAndScenes::servermsg_RoleOffline, this, _1, _2));
	REG_RAWMSG_PRIVATE(SessionReqTargetRoleScenePos, std::bind(&RolesAndScenes::servermsg_SessionReqTargetRoleScenePos, this, _1, _2, _3));
	REG_RAWMSG_PRIVATE(SessionFuncRoleReqChangeScene, std::bind(&RolesAndScenes::servermsg_SessionFuncRoleReqChangeScene, this, _1, _2));

    REG_RAWMSG_PUBLIC(GotoOtherSceneByTransmission, std::bind(&RolesAndScenes::clientmsg_GotoOtherSceneByTransmission, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RequestFeixieTransfer, std::bind(&RolesAndScenes::clientmsg_RequestFeixieTransfer, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RoleIntoCopyMap, std::bind(&RolesAndScenes::clientmsg_RoleIntoCopyMap, this, _1, _2, _3));
	REG_RAWMSG_PUBLIC(RoleLeaveCopyMap, std::bind(&RolesAndScenes::clientmsg_RoleLeaveCopyMap, this, _1, _2, _3));
}

void RolesAndScenes::servermsg_RoleIntoScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleIntoScene*>(msgData);
	if(rev == nullptr)
		return;

    PrivateRaw::RetRoleIntoScene send;
    send.rid = rev->basic.id;
    send.gatewayId = rev->gatewayId;
    ProcessIdentity sessionId("session", 1);
    ProcessIdentity gatewayId(rev->gatewayId);

	Role::Ptr role = nullptr;
	try
	{
		role = Role::create(rev);
	}
	catch(const world::CreatePackageSetFailed& er)
	{
		LOG_DEBUG("create role 失败, {}", er);
		return;
	}

    LOG_DEBUG("收到session消息, 有角色进入world, sceneId={}, {}", rev->sceneId, *role);
    /*
     *这里应该还有role的其它一些初始化处理
     */
	SceneId sceneId = rev->sceneId;
    Coord2D destPos(rev->posX, rev->posY);
	if(rev->usage == PrivateRaw::RoleDataUsage::changeScene)
	{
		sceneId = rev->sceneId;
		destPos = Coord2D(rev->posX, rev->posY);
	}

    Scene::Ptr scene = SceneManager::me().getById(sceneId);
    if(scene == nullptr)
    {
        LOG_ERROR("角色进场景, 场景不存在, sceneId={}, {}", sceneId, *role);
        send.isSuccessful = false;
        World::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetRoleIntoScene), &send, sizeof(send));
        return;
    }
    changeSceneByofflnTimeAndCopyMap(scene, sceneId, destPos, rev->preSceneId, Coord2D(rev->prePosX, rev->prePosY), rev->offlnTime);

    //这里先写成这样, 
    //考虑中, 可以把rolemanager做成业务逻辑无关, role不在线也可以存在于manager中, 这样就自动支持了离线角色
    if(!RoleManager::me().insert(role))
    {
        LOG_ERROR("角色进场景, role加入全局管理器失败, {}", *role);
        send.isSuccessful = false;
        World::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetRoleIntoScene), &send, sizeof(send));
        return;
    }

    //const bool sceneAddRoleRet = (rev->usage == PrivateRaw::RoleDataUsage::login) ? scene->addRole(role) : scene->addRole(role, Coord2D(rev->posX, rev->posY), 10);
    if(destPos.x == 0 && destPos.y == 0)
		destPos = scene->randAvailablePos(SceneItemType::role);

	const bool sceneAddRoleRet = scene->addRole(role, destPos, 10);
    if(!sceneAddRoleRet)
    {
        LOG_ERROR("角色进场景, 加入场景失败, scenenId={}, {}", sceneId, *role);
        RoleManager::me().erase(role);

        send.isSuccessful = false;
        World::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetRoleIntoScene), &send, sizeof(send));
        return;
    }

    //role的一些初始化
    role->setSceneId(scene->id());
    role->setGatewayId(gatewayId);

    if(rev->usage == PrivateRaw::RoleDataUsage::login)
        role->login();
 
	//初始化结束, 调用进入场景事件
    role->afterEnterScene();
	return;
}

void RolesAndScenes::changeSceneByofflnTimeAndCopyMap(Scene::Ptr& scene, SceneId& sceneId, Coord2D& pos, const SceneId& preSceneId, const Coord2D& prePos, uint32_t offlnTime)
{
    //如果角色进入的是boss之家先判断是否超时
    if(scene->mapTpl()->type == CopyMap::boss_home)
    {
        uint32_t now = std::time(NULL);
        if(now - offlnTime > m_offlnKeepRedution)
        {
            sceneId = preSceneId;
            pos = prePos;
            scene = SceneManager::me().getById(sceneId);
        } 
    }
}

void RolesAndScenes::servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleOffline*>(msgData);

    LOG_DEBUG("角色下线, rid={}", rev->rid);

    auto role = RoleManager::me().getById(rev->rid);
    if(role == nullptr)
    {
        LOG_TRACE("角色下线, role没有找到, rid={}", rev->rid);
        return;
    }

    RoleManager::me().erase(role);

    Scene::Ptr scene = role->scene();
    if(scene == nullptr)
        return;
    //role->beforeLeaveScene();
    role->offline();
    scene->eraseRole(role);
}

void RolesAndScenes::clientmsg_GotoOtherSceneByTransmission(const uint8_t * msgData, uint32_t msgSize, uint64_t rid)
{
    auto rev = reinterpret_cast<const PublicRaw::GotoOtherSceneByTransmission*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;

    bool transFailed = true;
    auto noticeClientTransFailed = [&transFailed, role] () -> void
    {
        if(!transFailed)
            return;
        PublicRaw::GotoOtherSceneByTransmissionFailed send;
        role->sendToMe(RAWMSG_CODE_PUBLIC(GotoOtherSceneByTransmissionFailed), &send, sizeof(send));
    };
    ON_EXIT_SCOPE(noticeClientTransFailed);

    if(role->isDead())
        return;

    Scene::Ptr oldScene = role->scene();
    if(oldScene == nullptr)
        return;

    auto& transmissions = oldScene->transmissions();
    auto it = transmissions.find(rev->transmissionId);
    if(it == transmissions.end())
    {
        LOG_DEBUG("传送点跳地图, 非法传送点id={}, {}", rev->transmissionId, *role);
        return;
    }

    const Scene::Transmission& info = it->second;
    if(info.maxDistance < std::abs(info.grid.x - role->pos().x) ||
       info.maxDistance < std::abs(info.grid.y - role->pos().y))
    {
        LOG_DEBUG("传送点跳地图, 距离传送点过远");
        return;
    }

    if(!isStaticSceneId(info.destinationMapId))
    {
        LOG_ERROR("传送点跳地图, 目标地图id为动态id, role={}, 传送点id={}, mapId={}", 
                  *role, rev->transmissionId, info.destinationMapId);
        return;
    }
    
	const SceneId sceneId = info.destinationMapId;
	const Coord2D oldPos = role->pos();
	const Coord2D newPos(info.destinationGrid.x, info.destinationGrid.y);

	if(!gotoOtherScene(role->id(), sceneId, newPos))
		return;

    LOG_TRACE("传送点跳地图, 同world传送, role={}, sceneId={}->{}, transmissionId={}, grid=({})->({}), pos=({})->({}) ",
			  *role, oldScene->id(), sceneId, rev->transmissionId,
			  info.grid, info.destinationGrid, oldPos, role->pos());
    transFailed = false;
    return;
}

void RolesAndScenes::clientmsg_RequestFeixieTransfer(const uint8_t * msgData, uint32_t msgSize, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;

    if(role->isDead())
        return;

    if(role->isStall())
    {
        role->sendSysChat("正在摆摊, 不能传送!");
        return;
    }

    auto rev = reinterpret_cast<const PublicRaw::RequestFeixieTransfer*>(msgData);
	if(!rev)
		return;

	if(rev->bNeedObj)
	{
		if(!role->eraseObj(1005, 1, PackageType::role, "飞鞋传送"))
		{
			role->sendSysChat("小飞鞋不足, 无法传送");
			return;
		}
	}

    SceneId sceneId = rev->mapId;
	Coord2D pos(rev->posX, rev->posY);
	gotoOtherScene(role->id(), sceneId, pos);
	return;
}

bool RolesAndScenes::gotoOtherScene(RoleId roleId, SceneId newSceneId, Coord2D newPos)
{
	if(0 == newSceneId)
		return false;

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return false;

    Scene::Ptr oldScene = role->scene();
    if(oldScene == nullptr)
        return false;

    if(oldScene->id() == newSceneId)
	{
        role->changePos(newPos, role->dir(), MoveType::blink);
		return false;
	}

    MapId newMapId = sceneId2MapId(newSceneId);
    MapTpl::Ptr mapTpl = MapBase::me().getMapTpl(newMapId);
    if(mapTpl == nullptr)
        return false;

    //未达到进入地图的等级
    uint32_t needLevel = mapTpl->roleMinLevel;
    if(role->level() < needLevel)
    {
        role->sendSysChat("你需要达到{}级才可进入该地图", mapTpl->roleMinLevel);
        return false;
    }
    //未达到进入地图的转生等级
    TurnLife turnlife = mapTpl->roleMinTurnLife;;
    if(role->turnLifeLevel() < turnlife)
    {
        role->sendSysChat("你需要达到{}级转生等级才可进入该地图", uint16_t(turnlife));
        return false;
    }
    //判断并消耗道具
    if(mapTpl->objTplId != 0)
    {
        std::string text = format("人boss地图传送, mapId={}", newMapId);
        if(role->getObjNum(mapTpl->objTplId, PackageType::role) < mapTpl->objNum)
        {
            role->sendSysChat("进入该地图所需道具不足");
            return false;
        }
        role->eraseObj(mapTpl->objTplId, mapTpl->objNum, PackageType::role, text);
    }

    Scene::Ptr newScene = SceneManager::me().getById(newSceneId);
    //场景不在本进程
    if(newScene == nullptr)
    {
        LOG_TRACE("跳地图, 跨world传送, {}, {}->{} ", *role, oldScene->id(), newSceneId);

        //离开老场景
        role->beforeLeaveScene();
        oldScene->eraseRole(role);

        //从rolemanager中删除
        RoleManager::me().erase(role);

        //发消息到session, 要求跳地图
        PrivateRaw::RoleChangeScene send;
        send.rid = role->id();
        send.newSceneId = newSceneId;
        send.posX = newPos.x;
        send.posY = newPos.y;
        ProcessIdentity sessionId("session", 1);
        World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(RoleChangeScene), &send, sizeof(send));

        LOG_TRACE("跳地图, 跨world传送, 离开本场景, 并发送消息到session, {}, {}->{} ", *role, oldScene->id(), newSceneId);
        return true;
    }

    //记下在oldScene中的pos, 然后尝试把role放入新的scene
    auto oldScenePos = role->pos();
    if(!newScene->addRole(role, newPos))
    {
        LOG_ERROR("跳地图, 同world传送, 离开原场景后进入新场景失败, {}, {}->{}, {} ",
				  *role, oldScene->id(), newSceneId, newPos);
        return false;
    }
    auto newScenePos = role->pos();

    //执行离开老场景前的清理操作
    role->setPos(oldScenePos);
    role->setSceneId(oldScene->id());
    role->beforeLeaveScene();
    oldScene->eraseRole(role);

    //加入新场景
    role->setPos(newScenePos);
    role->setSceneId(newSceneId);
    role->afterEnterScene();

    LOG_TRACE("跳地图, 同world传送成功, {}, {}->{} ", *role, oldScene->id(), newSceneId);
    return true;
}

void RolesAndScenes::servermsg_SessionReqTargetRoleScenePos(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::SessionReqTargetRoleScenePos*>(msgData);
    Role::Ptr targetRole = RoleManager::me().getById(rev->targetRoleId);
    if(targetRole == nullptr)
        return;

    PrivateRaw::WorldRetTargetRoleScenePos send;
    send.rid = rev->rid;
    send.targetSceneId = targetRole->sceneId();
    send.targetPosx = targetRole->pos().x;
    send.targetPosy = targetRole->pos().y;
    World::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(WorldRetTargetRoleScenePos), &send, sizeof(send));
}

void RolesAndScenes::servermsg_SessionFuncRoleReqChangeScene(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SessionFuncRoleReqChangeScene*>(msgData);
    RolesAndScenes::me().gotoOtherScene(rev->rid, rev->newSceneId, Coord2D(rev->newPosx, rev->newPosy));
}

void RolesAndScenes::clientmsg_RoleIntoCopyMap(const uint8_t * msgData, uint32_t msgSize, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;
    auto rev = reinterpret_cast<const PublicRaw::RoleIntoCopyMap*>(msgData);
    role->intoCopyMap(rev->mapId);
}

void RolesAndScenes::clientmsg_RoleLeaveCopyMap(const uint8_t * msgData, uint32_t msgSize, uint64_t rid)
{
    auto role = RoleManager::me().getById(rid);
    if(nullptr == role)
        return;
    role->exitCopyMap();
}

} //end namespace

