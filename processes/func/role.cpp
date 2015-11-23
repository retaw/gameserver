#include "role.h"

#include "func.h"
#include "role_manager.h"
#include "channel.h"
#include "friend_manager.h"
#include "faction_manager.h"

#include "water/componet/scope_guard.h"
#include "water/componet/logger.h"

#include "protocol/rawmsg/private/relay.h"
#include "protocol/rawmsg/private/relay.codedef.private.h"

#include "protocol/rawmsg/private/friend.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"

#include "protocol/rawmsg/public/channel_info.h"
#include "protocol/rawmsg/public/channel_info.codedef.public.h"

#include "protocol/rawmsg/private/faction.h"
#include "protocol/rawmsg/private/faction.codedef.private.h"

#include "protocol/rawmsg/private/role_scene.h"
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include "water/componet/serialize.h"

namespace func{


Role::Role(RoleId id, const std::string& name, const std::string& account, uint32_t level, Job job)
: m_id(id), m_name(name), m_account(account), m_level(level), m_job(job)
{
}

RoleId Role::id() const
{
    return m_id;
}

const std::string& Role::name() const
{
    return m_name;
}

const std::string& Role::account() const
{
    return m_account;
}

uint32_t Role::level()
{
    return m_level;
}

Job Role::job() const
{
    return m_job;
}

void Role::setLevel(uint32_t level)
{
    m_level = level;
}

void Role::setTeamId(TeamId teamId)
{
    m_teamId = teamId;
}

const TeamId Role::teamId() const
{
    return m_teamId;
}

void Role::setFactionIdNotSysnc(FactionId factionId)
{
    m_factionId = factionId;
}

void Role::setFactionId(FactionId factionId)
{
    m_factionId = factionId;
    //同步world,dbcached
    PrivateRaw::UpdateFaction send;
    send.factionId = factionId;
    send.roleId = id();

    std::memset(send.factionName, 0, NAME_BUFF_SZIE);
    send.position = factionPosition();
    auto fac = FactionManager::me().getById(factionId);
    if(fac != nullptr)
    {
        fac->name().copy(send.factionName, NAME_BUFF_SZIE);
        send.level = fac->level();
    }
    Func::me().sendToPrivate(worldId(), RAWMSG_CODE_PRIVATE(UpdateFaction), (uint8_t*)&send, sizeof(send));

    ProcessIdentity dbcachedId("dbcached", 1);
    Func::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateFaction), &send, sizeof(send));
}

const FactionId Role::factionId() const
{
    return m_factionId;
}

void Role::setFactionPositionNotSysnc(FactionPosition position)
{
    m_factionPosition = position;
}

void Role::setFactionPosition(FactionPosition position)
{
    m_factionPosition = position;
    //同步world 
    PrivateRaw::UpdateFaction send;
    send.factionId = factionId();
    send.roleId = id();
    std::string factionName = FactionManager::me().getFactionName(factionId());
    factionName.copy(send.factionName, NAME_BUFF_SZIE);
    send.position = position;
    Func::me().sendToPrivate(worldId(), RAWMSG_CODE_PRIVATE(UpdateFaction), (uint8_t*)&send, sizeof(send));

    ProcessIdentity dbcachedId("dbcached", 1);
    Func::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(UpdateFaction), &send, sizeof(send));

}

const FactionPosition Role::factionPosition() const
{
    return m_factionPosition;
}

std::string Role::toString() const
{
    return water::componet::format("[{}, {}, {}]", m_id, m_name, m_account);
}

ProcessIdentity Role::worldId() const
{
    return m_worldId;
}

void Role::setWorldId(ProcessIdentity pid)
{
    m_worldId = pid;
}

ProcessIdentity Role::gatewayId() const
{
    return m_gatewayId;
}

void Role::setGatewayId(ProcessIdentity pid)
{
    m_gatewayId = pid;
}

bool Role::sendToMe(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    LOG_DEBUG("sendToMe, code={}, size={}, role={}", msgCode, msgSize, *this);

    const uint32_t bufSize = sizeof(PrivateRaw::RelayMsgToClient) + msgSize;

    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf); 

    auto* send = new(buf) PrivateRaw::RelayMsgToClient();
    send->rid     = id();
    send->msgCode = msgCode;
    send->msgSize = msgSize;
    std::memcpy(send->msgData, msg, msgSize);
    return Func::me().sendToPrivate(m_gatewayId, RAWMSG_CODE_PRIVATE(RelayMsgToClient), buf, bufSize);
}

bool Role::sendToMe(TcpMsgCode msgCode) const
{
    LOG_DEBUG("sendToMe, code={}, role={}", msgCode, *this);

    const uint32_t bufSize = sizeof(PrivateRaw::RelayMsgToClient);

    uint8_t* buf = new uint8_t[bufSize];
    ON_EXIT_SCOPE_DO(delete[] buf); 

    auto* send = new(buf) PrivateRaw::RelayMsgToClient();
    send->rid     = id();
    send->msgCode = msgCode;
    send->msgSize = 0;
    return Func::me().sendToPrivate(m_gatewayId, RAWMSG_CODE_PRIVATE(RelayMsgToClient), buf, bufSize);
}

bool Role::sendToWorld(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    return Func::me().sendToPrivate(worldId(), msgCode, msg, msgSize);
}

SceneId Role::sceneId() const
{
    return m_sceneId;
}

void Role::setSceneId(SceneId sceneId)
{
    m_sceneId = sceneId;
}

void Role::gotoOtherScene(SceneId newSceneId, Coord2D newPos)
{
    PrivateRaw::SessionFuncRoleReqChangeScene send;
    send.rid = id();
    send.newSceneId = newSceneId;
    send.newPosx = newPos.x;
    send.newPosy = newPos.y;

    sendToWorld(RAWMSG_CODE_PRIVATE(SessionFuncRoleReqChangeScene), &send, sizeof(send));
}

void Role::online()
{
    FriendManager::me().fillRoleRelationer(id());  //初始化role内的好友id，暂时是上线即从库取出放入缓存，下线清空
    FriendManager::me().tellFriendOnln(id());//通知好友上线
}

void Role::offline()
{
    //通知好友下线
    FriendManager::me().tellFriendOffln(id());
    //清除自己的添加好友的列表，即去掉一直没有响应的好友请求列表
    FriendManager::me().clearOfflnRequetRecord(id());
}

void Role::sendSysChatPrivate(ChannelType type, const std::string& text)
{
	switch(type)
	{
	case ChannelType::system:
	case ChannelType::system_msgbox:
	case ChannelType::screen_right_down:
		sendSysMsgToMe(type, text);
		break;
	case ChannelType::screen_top:
	case ChannelType::screen_middle:
		Channel::me().sendSysNotifyToGlobal(type, text);
		break;
	default:
		break;
	}

	return;
}

void Role::sendSysMsgToMe(ChannelType type, const std::string& text)
{
	if(text.empty())
		return;

	const ArraySize textSize = text.size() + 1;
	const uint32_t bufSize = sizeof(const PublicRaw::SystemChannelMsgToClient) + textSize;
	auto buf = new uint8_t[bufSize];
	ON_EXIT_SCOPE_DO(delete[] buf);

	auto send = new(buf) PublicRaw::SystemChannelMsgToClient();
	send->type = type;
	send->textSize = textSize;
	memcpy(send->text, text.c_str(), textSize);

	sendToMe(RAWMSG_CODE_PUBLIC(SystemChannelMsgToClient), buf, bufSize);
	return;
}

void Role::synWorldEnemy()
{
    std::vector<uint8_t> buf;
    buf.reserve(420);
    componet::Serialize<std::string> ss;
    ss.reset();
    std::unordered_set<RoleId> enemySet;
    enemySet.insert(m_enemyList.begin(), m_enemyList.end());
    ss << enemySet;
    buf.resize(buf.size() + ss.tellp());
    auto* msg = reinterpret_cast<PrivateRaw::UpdateWorldEnemy*>(buf.data());
    msg->roleId = id();
    msg->size = enemySet.size();
    std::memcpy(msg->buf, ss.buffer()->data(), ss.tellp());
    Func::me().sendToPrivate(worldId(), RAWMSG_CODE_PRIVATE(UpdateWorldEnemy), buf.data(), buf.size());
}

void Role::synBanggong(const uint64_t banggong)
{
    PrivateRaw::SynBanggong send;
    send.roleId = id();
    send.banggong = banggong;
    Func::me().sendToPrivate(worldId(), RAWMSG_CODE_PRIVATE(SynBanggong), (uint8_t*)&send, sizeof(send));
}

void Role::sysnFactionLevel(const uint32_t level)
{
    PrivateRaw::SysnFactionLevel send;
    send.roleId = id(); 
    send.level = level;
    Func::me().sendToPrivate(worldId(), RAWMSG_CODE_PRIVATE(SynBanggong), (uint8_t*)&send, sizeof(send));
}

bool Role::isExistFriend(RoleId friendId)
{
   auto it = m_friend.find(friendId);
   if(it == m_friend.end())
       return false;
   return true;
}

bool Role::insertFriend(RoleId friendId)
{
    m_friend.insert(friendId);
    return true;
}

bool Role::eraseFriend(RoleId friendId)
{
    m_friend.erase(friendId);
    return true;
}

uint32_t Role::friendNum()
{
    return uint32_t(m_friend.size());
}

std::unordered_set<RoleId> Role:: getFriendSet()
{
    return m_friend;
}

void Role::setFriendSet(std::unordered_set<RoleId> friendSet)
{
    m_friend = friendSet;
}

bool Role::isExistEnemy(RoleId enemyId)
{
    auto it = m_enemyId2Index.find(enemyId);
    if(it == m_enemyId2Index.end())
        return false;
    return true;
}

void Role::setEnemyList(std::list<RoleId>& enemyList)
{
    m_enemyList.swap(enemyList); 
}

bool Role::insertEnemy(RoleId enemyId, uint16_t limit)
{
    m_enemyList.push_front(enemyId);
    m_enemyId2Index.emplace(enemyId, m_enemyList.begin());
    if(m_enemyList.size() > limit)
    {
        //超过50个则删除最早的一个
        m_enemyId2Index.erase(m_enemyList.back());
        m_enemyList.pop_back();
    }
    synWorldEnemy();
    return true;
}

bool Role::eraseEnemy(RoleId roleId)
{
    auto it = m_enemyId2Index.find(roleId);
    if(it == m_enemyId2Index.end())
        return true;
    m_enemyList.erase(it->second);
    m_enemyId2Index.erase(it);

    return true;
}

uint32_t Role::enemyNum()
{
    return m_enemyList.size();
}

std::vector<RoleId> Role::getEnemys()
{
    //这里直接返回unordered_map效率更好
    std::vector<RoleId> ret;
    ret.insert(ret.begin(), m_enemyList.begin(), m_enemyList.end());
    return ret;
}

std::list<RoleId> Role::getEnemyList()
{
    return m_enemyList;
}

}
