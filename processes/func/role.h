/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-25 13:54 +0800
 *
 * Modified: 2015-05-06 11:21 +0800
 *
 * Description: func上需要知道的角色信息
 */

#ifndef PROCESSES_FUNC_ROLE_H
#define PROCESSES_FUNC_ROLE_H

#include "water/common/commdef.h"
#include "water/common/scenedef.h"
#include "water/common/frienddef.h"
#include "water/common/channeldef.h"
#include "water/common/factiondef.h"
#include "water/common/role_container.h"
#include "water/process/process_id.h"
#include "water/process/tcp_message.h"
#include "water/net/packet_connection.h"
#include "water/componet/datetime.h"
#include "water/componet/coord.h"


#include <atomic>
#include <list>
#include <mutex>
#include <unordered_set>

namespace func{

using namespace water;
using water::process::ProcessIdentity;
using water::process::TcpMsgCode;
using water::componet::Coord2D;


class Role
{
public:
    TYPEDEF_PTR(Role)
    CREATE_FUN_MAKE(Role)

    Role(RoleId id, const std::string& name, const std::string& account, uint32_t level, Job job);
    ~Role() = default;

    RoleId id() const;
    const std::string& name() const;
    const std::string& account() const;
    uint32_t level();
    Job job() const;
    void setLevel(uint32_t level);
    void setTeamId(TeamId teamId);
    const TeamId teamId() const;
    //faction
    void setFactionId(FactionId factionId);
    void setFactionIdNotSysnc(FactionId factionId);
    const FactionId factionId() const;

    void setFactionPositionNotSysnc(FactionPosition position);//不要同步到world进程
    void setFactionPosition(FactionPosition position);//同步到world进程
    const FactionPosition factionPosition() const;

    std::string toString() const;

    ProcessIdentity worldId() const;
    void setWorldId(ProcessIdentity pid);

    ProcessIdentity gatewayId() const;
    void setGatewayId(ProcessIdentity pid);

    SceneId sceneId() const;
    void setSceneId(SceneId sceneId);

    bool sendToMe(TcpMsgCode msgCode) const;
    bool sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const;
    bool sendToWorld(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const;

    //切换地图
    void gotoOtherScene(SceneId newSceneId, Coord2D newPos);

    //角色上下线处理
    void online();
    void offline();

    //
    bool isExistFriend(RoleId friendId);
    bool insertFriend(RoleId friendId);
    bool eraseFriend(RoleId friendId);
    uint32_t friendNum();
    std::unordered_set<RoleId> getFriendSet();
    void setFriendSet(std::unordered_set<RoleId> friendSet);

    void setEnemyList(std::list<RoleId>& enemyList);
    bool isExistEnemy(RoleId enemyId);
    bool insertEnemy(RoleId enemyId, uint16_t limit);
    bool eraseEnemy(RoleId roleId);
    uint32_t enemyNum();
    std::vector<RoleId> getEnemys();
    std::list<RoleId> getEnemyList();

    //同步func的relation到world(队友的同步在team内做了)
    void synWorldEnemy();
    void synBanggong(const uint64_t banggong);
    void sysnFactionLevel(const uint32_t level);

public:
	template<typename... Args>    
	void sendSysChat(const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		sendSysChatPrivate(ChannelType::system_msgbox, text);
		return;
	}

	template<typename... Args>    
	void sendSysChat(ChannelType type, const std::string& formatStr, const Args&... args)
	{
		const std::string text = componet::format(formatStr, args...); 
		sendSysChatPrivate(type, text);
		return;
	}

private:
	void sendSysChatPrivate(ChannelType type, const std::string& text);
	void sendSysMsgToMe(ChannelType type, const std::string& text);

private:
    const RoleId m_id;
    const std::string m_name;
    const std::string m_account;
    uint32_t m_level;
    Job m_job;
    TeamId m_teamId = 0;
    FactionId m_factionId = 0;
    FactionPosition m_factionPosition = FactionPosition::none;

    std::unordered_set<RoleId> m_friend;

    std::list<RoleId> m_enemyList;
    std::unordered_map<RoleId, std::list<RoleId>::iterator> m_enemyId2Index;
    
    ProcessIdentity m_gatewayId;
    ProcessIdentity m_worldId;

    SceneId m_sceneId;
};


}

#endif
