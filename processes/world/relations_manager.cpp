#include "relations_manager.h"
#include "world.h"

#include "role_manager.h"
#include "water/componet/logger.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/team.h"
#include "protocol/rawmsg/public/team.codedef.public.h"
#include "protocol/rawmsg/private/team.codedef.private.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"
#include "protocol/rawmsg/private/faction.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "water/componet/serialize.h"


namespace world{

RelationsManager& RelationsManager::me()
{
    static RelationsManager me;
    return me;
}

void RelationsManager::regMsgHandler()
{
    using namespace std::placeholders;

    REG_RAWMSG_PRIVATE(UpdateTeamInfo, std::bind(&RelationsManager::servermsg_UpdateTeamInfo, this, _1, _2));
    REG_RAWMSG_PRIVATE(UpdateFaction, std::bind(&RelationsManager::servermsg_UpdateFacion, this, _1, _2));
    REG_RAWMSG_PRIVATE(UpdateWorldEnemy, std::bind(&RelationsManager::servermsg_UpdateWorldEnemy, this, _1, _2));
    REG_RAWMSG_PRIVATE(CreateFactionCost, std::bind(&RelationsManager::servermsg_CreateFactionCost, this, _1, _2));
    REG_RAWMSG_PRIVATE(SynBanggong, std::bind(&RelationsManager::servermsg_SynBanggong, this, _1, _2));
    REG_RAWMSG_PRIVATE(SysnFactionLevel, std::bind(&RelationsManager::servermsg_SysnFactionLevel, this, _1, _2));
    REG_RAWMSG_PRIVATE(UpdateWorldBlack, std::bind(&RelationsManager::servermsg_UpdateWorldBlack, this, _1, _2));

}

std::vector<RoleId> RelationsManager::getMemberVecByTeamId(TeamId teamId)
{
    auto it = m_teams.find(teamId);
    std::vector<RoleId> ret;
    if(it == m_teams.end())
    {
        LOG_DEBUG("查询队伍所有队员, teamId无效, teamId={}", teamId);
        ret.clear();
        return ret;
    }
    for(auto& id : it->second)
        ret.push_back(id);
    return ret;
}

float RelationsManager::getTeamMeberExtraAddExpRatio(Role::Ptr role)
{
    if(role == nullptr)
        return 0;
    if(role->teamId() == 0)
        return 0;
    auto sameSceneNum = getSameSceneNumByRole(role);
    return teamMemberAddExpRatio * sameSceneNum;
    LOG_DEBUG("获得组队杀怪的额外经验加成系数: {} * {}", teamMemberAddExpRatio, sameSceneNum);
}

void RelationsManager::servermsg_UpdateTeamInfo(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateTeamInfo*>(msgData);
    LOG_DEBUG("收到队伍更新消息");
    updateTeamInfo(rev);
}

void RelationsManager::servermsg_UpdateFacion(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateFaction*>(msgData);
    LOG_DEBUG("收到帮派更新消息, roleId={}, factionId={}, position={}", rev->roleId, rev->factionId, rev->position);
    updateFaction(rev);
}

void RelationsManager::updateFaction(const PrivateRaw::UpdateFaction* rev)
{
   auto role = RoleManager::me().getById(rev->roleId);
   if(role == nullptr)
       return;

   role->setFaction(rev);
}

void RelationsManager::servermsg_SynBanggong(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SynBanggong*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    //从func来，就不要同步到func了（这里其实是把func看做dbcache）
    role->setBanggongWithoutSysn(rev->banggong);
}

void RelationsManager::servermsg_SysnFactionLevel(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SysnFactionLevel*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    role->setFactionLevel(rev->level);
}

void RelationsManager::servermsg_UpdateWorldEnemy(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateWorldEnemy*>(msgData);
    LOG_DEBUG("收到仇人更新消息");
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    std::string ss;
    ss.append((const char*)rev->buf, (std::string::size_type)rev->size);
    Deserialize<std::string> ds(&ss);
    std::unordered_set<RoleId> enemy;
    ds >> enemy;
    role->setEnemy(enemy);
}

void RelationsManager::servermsg_UpdateWorldBlack(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateWorldBlack*>(msgData);
    LOG_DEBUG("收到黑名单同步");
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    std::string ss;
    ss.append((const char*)rev->buf, (std::string::size_type)rev->size);
    Deserialize<std::string> ds(&ss);
    std::unordered_set<RoleId> black;
    ds >> black;
    role->setBlack(black);
}

void RelationsManager::servermsg_CreateFactionCost(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::CreateFactionCost*>(msgData);
    LOG_DEBUG("收到创建帮派消耗, roleId={}", rev->roleId);
    createFactionCost(rev->roleId, rev->factionName, 
                      rev->propId, rev->propNum,
                      (const MoneyType)rev->moneyId, rev->moneyNum);
}

void RelationsManager::updateTeamInfo(const PrivateRaw::UpdateTeamInfo* rev)
{
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    if(rev->insertOrErase)  //插入
    {
        m_teams[rev->teamId].insert(rev->roleId);
        LOG_DEBUG("同步func的队伍信息, teamId={}, roleId={}", rev->roleId, rev->teamId);
    }
    else    //删除
    {
        auto it = m_teams.find(role->teamId());
        if(it == m_teams.end())
        {
            LOG_ERROR("逻辑错误, 角色的队伍已经不在队伍列表中, roleId={}, teamId={}",
                      role->id(), role->teamId());
            return;
        }
        it->second.erase(role->id());
        if(it->second.size() == 0)
        {
            m_teams.erase(it);
            LOG_DEBUG("同步func的队伍信息, 队伍已经没人, 删除队伍");
        }
        LOG_DEBUG("同步func的队伍信息, teamId={}, roleId={}", rev->roleId, rev->teamId);
    }
    role->setTeamInfo(rev);
    LOG_DEBUG("角色的队伍信息更新成功, roleId = {}, teamId = {}", rev->roleId, rev->teamId);
}

void RelationsManager::createFactionCost(const RoleId roleId, const char* name, const uint64_t propId, const uint32_t propNum, const MoneyType type, const uint32_t moneyNum)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    //检查金币是否足够
    if(!role->checkMoney(type, moneyNum))
    {
        LOG_DEBUG("帮派创建, 金币不足, type={}, num={}", type, moneyNum);
        return;
    }
    //检查道具是否足够
    if(role->m_packageSet.getObjNum(propId, PackageType::role) < propNum)
    {
        LOG_DEBUG("帮派创建, 道具不足, objId={}, num={}", propId, propNum);
        return;
    }
    //消耗
    role->reduceMoney(type, moneyNum, "帮派创建");
    role->m_packageSet.eraseObj(propId, propNum, PackageType::role, "帮派创建");
    PrivateRaw::CreateFaction send;
    send.roleId = roleId;
    std::memcpy(send.factionName, name, NAME_BUFF_SZIE);
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(CreateFaction), &send, sizeof(send));
}

uint16_t RelationsManager::getSameSceneNumByRole(Role::Ptr role)
{
    if(role == nullptr)
        return 0;
    auto it = m_teams.find(role->teamId());
    if(it == m_teams.end())
        return 0;
    uint16_t ret;
    for(auto& id : it->second)
    {
        auto member = RoleManager::me().getById(id);
        if(member == nullptr)
            continue;
        if(member->sceneId() == role->sceneId())
            ret++;
    }
    return ret;
}

}
