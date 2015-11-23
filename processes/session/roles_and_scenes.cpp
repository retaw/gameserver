#include "roles_and_scenes.h"

#include "session.h"
#include "role_manager.h"
#include "scene_dispenser.h"

#include "water/componet/logger.h"
#include "water/componet/scope_guard.h"
#include "water/componet/xmlparse.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"

#include "protocol/rawmsg/private/friend.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include <memory>

namespace session{

RolesAndScenes& RolesAndScenes::me()
{
    static RolesAndScenes me;
    return me;
}

void RolesAndScenes::loadConfig(const std::string& cfgDir)
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;
    const std::string configFile = cfgDir + "/loginInit.xml";
    LOG_TRACE("读取配置文件 {}", configFile);
    XmlParseDoc doc(configFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, configFile + " parse root node failed");
    XmlParseNode defaultPosNode = root.getChild("defaultPos");
    m_mapId = defaultPosNode.getAttr<uint16_t>("mapId");
    m_posx = defaultPosNode.getAttr<uint16_t>("posX");
    m_posy = defaultPosNode.getAttr<uint16_t>("posY");
    m_dir = defaultPosNode.getAttr<uint16_t>("dir");
}

void RolesAndScenes::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(RoleOnline, std::bind(&RolesAndScenes::servermsg_RoleOnline, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RetRoleData, std::bind(&RolesAndScenes::servermsg_RetRoleData, this, _1, _2));
    REG_RAWMSG_PRIVATE(RetRoleIntoScene, std::bind(&RolesAndScenes::servermsg_RetRoleIntoScene, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RoleOffline, std::bind(&RolesAndScenes::servermsg_RoleOffline, this, _1, _2, _3));
    REG_RAWMSG_PRIVATE(RoleChangeScene, std::bind(&RolesAndScenes::servermsg_RoleChangeScene, this, _1, _2));
    REG_RAWMSG_PRIVATE(FuncQuestSyncAllOnlineRoleInfo, std::bind(&RolesAndScenes::servermsg_FuncQuestSyncAllOnlineRoleInfo, this));
    REG_RAWMSG_PRIVATE(UpdateFuncAndSessionRoleLevel, std::bind(&RolesAndScenes::servermsg_UpdateSessionRoleLevel, this, _1, _2));

    REG_RAWMSG_PRIVATE(RoleGotoTargetRoleScene, std::bind(&RolesAndScenes::servermsg_RoleGotoTargetRoleScene, this, _1, _2));
    REG_RAWMSG_PRIVATE(WorldRetTargetRoleScenePos, std::bind(&RolesAndScenes::servermsg_WorldRetTargetRoleScenePos, this, _1, _2));
    REG_RAWMSG_PRIVATE(WorldCatchRole, std::bind(&RolesAndScenes::servermsg_WorldCatchRole, this, _1, _2));
}

//处理上线的消息
void RolesAndScenes::servermsg_RoleOnline(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleOnline*>(msgData);
    LOG_TRACE("角色上线, rid={}, loginId{}", rev->rid, rev->loginId);

    m_loginingRoles.insert(rev->loginId);

    PrivateRaw::GetRoleData send;
    send.usage = PrivateRaw::RoleDataUsage::login;
    send.loginId = rev->loginId;
    send.rid = rev->rid;
    send.gatewayId = pid.value();
    ProcessIdentity receiver("dbcached", 1);
    Session::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(GetRoleData), &send, sizeof(send));
}

void RolesAndScenes::servermsg_RetRoleData(const uint8_t* msgData, uint32_t msgSize)
{
    LOG_DEBUG("角色上线, db返回角色数据");
    auto rev = reinterpret_cast<const PrivateRaw::RetRoleData*>(msgData);
    Role::Ptr role;
    if(rev->usage == PrivateRaw::RoleDataUsage::login) 
    {
        if(m_loginingRoles.find(rev->loginId) == m_loginingRoles.end())
        {
            LOG_TRACE("角色上线, db返回角色数据时网关连接已断开, 不用继续了, loginId={}", rev->loginId);
            return;
        }
        m_loginingRoles.erase(rev->loginId);

        role = RoleManager::me().getById(rev->basic.id);
        if(role != nullptr) //已经在线了
        {
            LOG_ERROR("重复上线, loginId={}, 对应角色:{}", rev->loginId, *role);
            return;
        }

        role = Role::create(rev->basic.id, rev->basic.name, rev->account, rev->level, rev->basic.job);

        if(!RoleManager::me().insert(role))
        {
            LOG_ERROR("角色上线, 插入管理器失败, 通知gateway下线, {}", *role);
            //通知gateway下线
            return;
        }
        role->setGatewayId(rev->gatewayId);
        LOG_DEBUG("角色上线, 成功, 开始进入进场景流程, {}", *role);
    }
    else
    {
        if(m_changingSceneRoles.find(rev->basic.id) == m_changingSceneRoles.end())
        {
            LOG_TRACE("角色切换场景, db返回角色数据时, 切换请求已失效, roleId={}", rev->basic.id);
            return;
        }
        m_changingSceneRoles.erase(rev->basic.id);

        role = RoleManager::me().getById(rev->basic.id);
        if(role == nullptr)
        {
            LOG_DEBUG("角色切换场景, db返回时找不到role, roleId={}", rev->basic.id);
            return;
        }

        LOG_DEBUG("角色切换场景, 成功, 开始进入进场景流程, {}", *role);
    }

    SceneId sceneId = rev->sceneId;
    Coord2D pos(rev->posX, rev->posY);
    uint8_t dir = rev->dir;
    //如果是新创建角色，sceneId为0，则读取配置中的默认出生点
    if(rev->sceneId == 0)
    {
        sceneId = m_mapId;
        pos = Coord2D(m_posx, m_posy);
        dir = m_dir;
    }

    auto worldId = SceneDispenser::me().sceneId2WorldId(sceneId);
    if(!worldId.isValid()) //场景不存在
    {
        if(isStaticSceneId(sceneId)) //静态图
        {
            LOG_ERROR("角色进场景, 静态场景没找到, sceneId={}, {}", sceneId, *role);
            //通知gateway下线
            return;
        }
        else //副本
        {
            LOG_TRACE("角色进场景, 副本没找到, 直接进新手村, sceneId={}, {}", sceneId, *role);
            //副本已销毁,进之前静态地图
            sceneId = rev->preSceneId;
            pos = Coord2D(rev->prePosX, rev->prePosY);
            worldId = SceneDispenser::me().sceneId2WorldId(sceneId);
            if(!worldId.isValid())
            {
                LOG_DEBUG("角色进场景, 副本没找到, 之前静态地图也找不到, 奇葩, sceneId={}", sceneId);

                //强制进新手村
                sceneId = m_mapId; //先写死, 以后改
                pos = Coord2D(m_posx, m_posy);
                dir = m_dir;
                worldId = SceneDispenser::me().sceneId2WorldId(sceneId);
            }
        }
    }

    //发往world, 进场景
	std::vector<uint8_t> buf(msgSize);
	auto send = new(buf.data()) PrivateRaw::RoleIntoScene();
	std::memcpy(buf.data(), msgData, msgSize);
	send->sceneId = sceneId;
    send->posX = pos.x;
    send->posY = pos.y;
    send->dir = dir;

    Session::me().sendToPrivate(worldId, RAWMSG_CODE_PRIVATE(RoleIntoScene), buf.data(), buf.size());
    LOG_TRACE("角色进场景, worldId={}, sceneId={}, {}", worldId, sceneId, *role);
	
}

void RolesAndScenes::servermsg_RetRoleIntoScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RetRoleIntoScene*>(msgData);
    LOG_DEBUG("角色进场景, world返回进场景结果: {}", rev->isSuccessful ? "成功" : "失败");
    auto role = RoleManager::me().getById(rev->rid);
    if(!rev->isSuccessful || role == nullptr) //正常的话, 只有一个可能, 就是跳地图期间, session处理了role下线的消息
    {
        LOG_ERROR("角色进场景, world进场景成功, 但session没找到这个角色, rid={}", rev->rid);
        /*
        //通知其他进程, 让这个角色异常下线
        PrivateRaw::RoleOffline error;
        error.type = PrivateRaw::OfflineType::sessionError;
        error.rid = rev->rid;
        Session::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RoleOffline), &error, sizeof(error));

        //发往gateway的
        Session::me().sendToPrivate(rev->gatewayId, RAWMSG_CODE_PRIVATE(RoleOffline), &error, sizeof(error));
        */
        return;
    }
	role->setSceneId(rev->sceneId);

    //更新worldId
    if(pid != role->worldId())
    {
        role->setWorldId(pid);
    
        PrivateRaw::UpdateRoleWorldId msg1;
        msg1.rid = role->id();
        msg1.worldId = pid.value();
        Session::me().sendToPrivate(rev->gatewayId, RAWMSG_CODE_PRIVATE(UpdateRoleWorldId), &msg1, sizeof(msg1));
        LOG_DEBUG("角色进场景, 通知网关更新worldId");


        ProcessIdentity receiver("dbcached", 1);
        Session::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(UpdateRoleWorldId), &msg1, sizeof(msg1));
    }

	PrivateRaw::SyncOnlineRoleInfoToFunc msg2;
	role->fillFuncSyncData(&msg2);
	ProcessIdentity receiver("func", 1);
	Session::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(SyncOnlineRoleInfoToFunc), &msg2, sizeof(msg2));
	LOG_DEBUG("角色进场景, 同步更新到func");
}

void RolesAndScenes::servermsg_RoleOffline(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleOffline*>(msgData);
    LOG_DEBUG("角色下线, 收到下线消息, rid={}, loginId={}, type={}", rev->rid, rev->loginId, rev->type);

    //如果正在上线流程的去db取数据阶段, 则中断上线流程即可
    if(m_loginingRoles.find(rev->loginId) != m_loginingRoles.end())
    {
        m_loginingRoles.erase(rev->loginId);
        return;
    }

    //如果是正在跳地图, 暂时不在任何场景上, 则中断跳地图流程
    if(m_changingSceneRoles.find(rev->rid) != m_changingSceneRoles.end())
    {
        m_changingSceneRoles.erase(rev->rid);
        return;
    }

    auto role = RoleManager::me().getById(rev->rid);
    if(role == nullptr)//已经不在线了, 直接忽略
        return;

    RoleManager::me().erase(role);

    //转发给world 和 func, 让其销毁角色数据
    Session::me().sendToPrivate(role->worldId(), RAWMSG_CODE_PRIVATE(RoleOffline), msgData, msgSize);
    Session::me().sendToPrivate(ProcessIdentity("func", 1), RAWMSG_CODE_PRIVATE(RoleOffline), msgData, msgSize);
    LOG_DEBUG("角色下线, 通知场景, rid={}, loginId={}, type={}", rev->rid, rev->loginId, rev->type);
}

void RolesAndScenes::servermsg_RoleChangeScene(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleChangeScene*>(msgData);

    Role::Ptr role = RoleManager::me().getById(rev->rid);
    if(role == nullptr) //如果没找到, 可能是正在下线, 直接忽略这条消息即可
        return;

    //记下来, 正在跳场景
    m_changingSceneRoles.insert(rev->rid);

    PrivateRaw::GetRoleData send;
    send.usage = PrivateRaw::RoleDataUsage::changeScene;
    send.loginId = 0;
    send.rid = rev->rid;
    send.gatewayId = role->gatewayId().value();
    send.newSceneId = rev->newSceneId;
    send.posX = rev->posX;
    send.posY = rev->posY;
    ProcessIdentity receiver("dbcached", 1);
    Session::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(GetRoleData), &send, sizeof(send));
}

void RolesAndScenes::servermsg_FuncQuestSyncAllOnlineRoleInfo()
{
    const uint32_t bufSize = sizeof(PrivateRaw::SessionSyncAllOnlineRoleInfoToFunc) + sizeof(PrivateRaw::SyncOnlineRoleInfoToFunc) * RoleManager::me().size();
    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf);

    auto send = reinterpret_cast<PrivateRaw::SessionSyncAllOnlineRoleInfoToFunc*>(buf);
    send->size = 0;
    for(auto it = RoleManager::me().begin(); it != RoleManager::me().end(); ++it)
    {
        Role::Ptr role = *it;
        role->fillFuncSyncData(send->roleDataList + send->size);
        ++send->size;
    }

    ProcessIdentity receiver("func", 1);
    Session::me().sendToPrivate(receiver, RAWMSG_CODE_PRIVATE(SessionSyncAllOnlineRoleInfoToFunc), buf, bufSize);
    LOG_TRACE("批量同步角色信息到func, size={}", send->size);
}

void RolesAndScenes::servermsg_UpdateSessionRoleLevel(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateFuncAndSessionRoleLevel*>(msgData);
    LOG_DEBUG("角色级别同步, roleId = {}, level = {}",
              rev->roleId, rev->level);
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    role->setLevel(rev->level);
}

void RolesAndScenes::servermsg_RoleGotoTargetRoleScene(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::RoleGotoTargetRoleScene*>(msgData);
    auto targetRole = RoleManager::me().getByName(rev->targetName);
    if(targetRole == nullptr)
        return;

    PrivateRaw::SessionReqTargetRoleScenePos send;
    send.rid = rev->rid;
    send.targetRoleId = targetRole->id();
    targetRole->sendToWorld(RAWMSG_CODE_PRIVATE(SessionReqTargetRoleScenePos), &send, sizeof(send));
}

void RolesAndScenes::servermsg_WorldRetTargetRoleScenePos(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::WorldRetTargetRoleScenePos*>(msgData);
    auto role = RoleManager::me().getById(rev->rid);
    if(role == nullptr)
        return;

    role->gotoOtherScene(rev->targetSceneId, Coord2D(rev->targetPosx, rev->targetPosy));
}

void RolesAndScenes::servermsg_WorldCatchRole(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::WorldCatchRole*>(msgData);
    auto targetRole = RoleManager::me().getByName(rev->targetName);
    if(targetRole == nullptr)
        return;

    targetRole->gotoOtherScene(rev->newSceneId, Coord2D(rev->newPosx, rev->newPosy));
}

}

