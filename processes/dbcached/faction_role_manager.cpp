#include "faction_role_manager.h"
#include "faction_table_structure.h"

#include "water/componet/serialize.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

#include "protocol/rawmsg/private/faction.h"
#include "protocol/rawmsg/private/faction.codedef.private.h"
#include "protocol/rawmsg/rawmsg_manager.h"


namespace dbcached{

using namespace water::componet;

FactionRoleManager& FactionRoleManager::me()
{
    static FactionRoleManager me;
    return me;
}

void FactionRoleManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(UpdateFaction, std::bind(&FactionRoleManager::servermsg_UpdateFaction, this, _1, _2));
}

void FactionRoleManager::servermsg_UpdateFaction(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::UpdateFaction*>(msgData);
    auto role = RoleManager::me().m_contrRoles.getById(rev->roleId);
    if(role == nullptr)
        return;
    std::string factionName;
    factionName.append(rev->factionName, NAME_BUFF_SZIE);
    role->setFaction(rev->factionId, factionName, rev->position, rev->level);
    LOG_DEBUG("帮派信息同步, roleId={}, factionId={}, factionName={}, level={}", rev->roleId, rev->factionId, factionName, rev->level);
}

void FactionRoleManager::fillRoleFactionInfo(Role::Ptr role)
{
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select * from roleInFaction where id = " << role->id();
        std::vector<RowOfRoleInFaction> res;
        query.storein(res);
        if(res.empty())
            return;
        if(res[0].factionId == 0)
            return;

        std::vector<RowOfFaction> facRes;
        query.reset();
        query << "select * from faction where factionId = " << res[0].factionId;
        query.storein(facRes);
        if(facRes.empty())
        {
            LOG_ERROR("加载帮派信息错误, 数据库表faction中不存在角色的帮派id, roleId = {}, factionId = {}", role->id(), res[0].factionId);
            return;
        }

        FactionPosition position = FactionPosition::ordinary;
        if(role->id() == facRes[0].leader)
            position = FactionPosition::leader;
        else if(role->id() == facRes[0].warriorLeader)
            position = FactionPosition::warriorLeader;
        else if(role->id() == facRes[0].magicianLeader)
            position = FactionPosition::magicianLeader;
        else if(role->id() == facRes[0].taoistLeader)
            position = FactionPosition::taoistLeader;
        else
        {
            std::string ss = facRes[0].viceLeaders;
            std::unordered_set<RoleId> viceLeaderSet;
            water::componet::Deserialize<std::string> ds(&ss);
            ds >> viceLeaderSet;
            if(viceLeaderSet.find(role->id()) != viceLeaderSet.end())
                position = FactionPosition::viceLeader;
        }

        role->setFaction(res[0].factionId, facRes[0].name, position, facRes[0].level);
    }
}

}
