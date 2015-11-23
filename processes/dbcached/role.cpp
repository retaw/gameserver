#include "role.h"
#include "dbcached.h"

#include "role_table_structure.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"

namespace dbcached{

Hero::Hero(Job job)
{
    m_job = job;
}

Role::Role(RoleId id)
:m_id(id)
{
}

RoleId Role::id() const
{
    return m_id;
}

//roleRarelyUp
const std::string& Role::name() const
{
    return m_name;
}

const std::string& Role::account() const
{
    return m_account;
}

TurnLife Role::turnLife() const
{
    return m_turnLife;
}

Job Role::job() const
{
    return m_job;
}

Sex Role::sex() const
{
    return m_sex;
}

uint64_t Role::curObjId() const
{
    return m_curObjId;
}

uint16_t Role::unlockCellNumOfRole() const
{
    return m_unlockCellNumOfRole;
}

uint16_t Role::unlockCellNumOfHero() const
{
    return m_unlockCellNumOfHero;
}

uint16_t Role::unlockCellNumOfStorage() const
{
    return m_unlockCellNumOfStorage;
}

TimePoint Role::recallHeroTime() const
{
    return m_recallHeroTime;
}

uint8_t Role::guanzhiLevel() const
{
	return m_guanzhiLevel;
}

const std::string& Role::bufferVec() const
{
	return m_bufferVec;
}

//roleOftenUp
uint32_t Role::level() const
{
    return m_level;
}

uint64_t Role::exp() const
{
    return m_exp;
}

uint32_t Role::offlnTime() const
{
    return m_offlnTime;
}

uint16_t Role::evilVal() const
{
    return m_evilVal;
}

uint8_t Role::attackMode() const
{
    return m_attackMode;
}


//roleOfflnUp
uint32_t Role::mp() const
{
    return m_mp;
}

uint32_t Role::hp() const
{
    return m_hp;
}

SceneId Role::sceneId() const
{
    return m_sceneId;
}

uint8_t Role::dir() const
{
    return m_dir;
}

uint16_t Role::posX() const
{
    return m_posX;
}

uint16_t Role::posY() const
{
    return m_posY;
}

SceneId Role::preSceneId() const
{
    return m_preSceneId;
}

uint16_t Role::prePosX() const
{
    return m_prePosX;
}

uint16_t Role::prePosY() const
{
    return m_prePosY;
}

bool Role::dead() const
{
    return m_dead;
}

TimePoint Role::deathTime() const
{
    return m_deathTime;
}

uint32_t Role::totalOnlineSec() const
{
    return m_totalOnlineSec;
}

bool Role::isOnline() const
{
    return m_isOnline;
}

void Role::online()
{
    m_isOnline = true;
}

void Role::offline()
{
    m_isOnline = false;
}

TimePoint Role::greynameTime() const
{
    return m_greynameTime;
}

uint32_t Role::curMailIndex()
{
    return ++m_curMailIndex;
}

bool Role::summonHero() const
{
	return m_summonHero;
}

ProcessIdentity Role::worldId() const
{
    return m_worldId;
}

std::string Role::stallLog() const
{
    return m_stallLog;
}

bool Role::sendToWorld(TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
    return DbCached::me().sendToPrivate(worldId(), msgCode, msg, msgSize);
}

bool Role::save(uint32_t m_lastRoleIdCounter)
{
    try
    {
        mysqlpp::Query queryFRarelyUp = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryFOftenUp = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryFOfflnUp = dbadaptcher::MysqlConnectionPool::me().getconn()->query();

        mysqlpp::Query queryS = dbadaptcher::MysqlConnectionPool::me().getconn()->query();

        RowOfRoleRarelyUp rowRarelyUp(m_id, m_name, 
                              (uint8_t)m_turnLife, 
                              m_account, 
                              (uint8_t)m_sex, 
                              (uint8_t)m_job, 
                              m_unlockCellNumOfRole, 
                              m_unlockCellNumOfHero, 
                              m_unlockCellNumOfStorage, 
                              uint8_t(m_defaultCallHero), 
							  m_guanzhiLevel,
							  m_bufferVec);

        RowOfRoleOftenUp rowOftenUp(m_id, m_level, m_exp, 
                      m_money_1, m_money_2, m_money_3, m_money_4, m_money_5, m_money_6, 
					  m_money_7, m_money_8, m_money_9, m_money_10);

        RowOfRoleOfflnUp rowOfflnUp(m_id, m_mp, m_hp, 
                      m_dir, m_sceneId, (uint32_t(m_posX) << 16u) + m_posY, 
                      m_preSceneId, (uint32_t(m_prePosX) << 16u) + m_prePosY,
                      m_dead, componet::toUnixTime(m_deathTime),
                      m_totalOnlineSec,
                      m_offlnTime,
                      m_evilVal,
                      m_attackMode);

        mysqlpp::Transaction trans(
                                   *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                   mysqlpp::Transaction::serializable,
                                   mysqlpp::Transaction::session);
        //事务start
        queryFRarelyUp.insert(rowRarelyUp);
        queryFRarelyUp.execute();

        queryFOftenUp.insert(rowOftenUp);
        queryFOftenUp.execute();
        
        queryFOfflnUp.insert(rowOfflnUp);
        queryFOfflnUp.execute();
        RoleId id = m_lastRoleIdCounter;//得到m_id的实际id
        std::string sqlFirst = "update roleRarelyUp set name = ";
        std::string sqlSecond = " where id = 0";
        queryS << sqlFirst << id << sqlSecond;
        queryS.execute();
        //事务stop
        trans.commit();
        LOG_DEBUG("DB:角色入库, 成功, id={}, account={}", m_id, m_account);
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("DB: Role::save, DB error:{}",er.what());
        return false;
    }
}

void Role::setFaction(FactionId factionId, std::string& factionName, FactionPosition position, uint32_t level)
{
    m_factionId = factionId;
    m_factionName = factionName;
    m_position = position;
    m_factionLevel = level;
}



std::unordered_map<uint64_t,RoleObjData::ObjData> Role::getObjDataByKeyMap()
{
    return m_objDataByKeyMap;
}

}
