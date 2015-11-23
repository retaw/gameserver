#include "role_sundry.h"
#include "role.h"
#include "world.h"

#include "water/componet/serialize.h"

#include "protocol/rawmsg/private/sundry.h"
#include "protocol/rawmsg/private/sundry.codedef.private.h"


namespace world{

void RoleSundry::loadSundry(std::string& sundry, RoleId roleId)
{
    water::componet::Deserialize<std::string> sundryDs(&sundry);
//添加数据
/*不常更新数据*/
    sundryDs >> m_refreshedFactionShop;
    sundryDs >> m_refreshFacShopTime;
    sundryDs >> m_facShopTab;
    sundryDs >> m_facShopGoodsId;
    sundryDs >> m_buyStoreDayLimitObj;
    sundryDs >> privateBossMap;
    sundryDs >> m_dragonSkills;
    sundryDs >> m_energe;

    sundryDs >> m_heroData.heroSkillSetting;

    m_roleId = roleId;
}

void RoleSundry::loadSundryForTimer(std::string& sundry)
{
    water::componet::Deserialize<std::string> sundryDs(&sundry);
//添加数据
/*频繁定时更新数据*/
    sundryDs >> m_testTimerData;
    sundryDs >> m_worldBossDamage;
    sundryDs >> m_receivedWBDamageAward;
}

void RoleSundry::setFacShop(time_t time, uint16_t facShopTab, std::vector<uint32_t>& goosId)
{
    m_refreshedFactionShop = true;
    m_refreshFacShopTime = time;
    m_facShopTab = facShopTab;
    m_facShopGoodsId.clear();
    for(auto& it : goosId)
    {
        m_facShopGoodsId.emplace_back(it, true);
    }

    saveSundry();
}

std::set<TplId>& RoleSundry::heroSkillSetting(Job job)
{
    return m_heroData.heroSkillSetting[job];
}

void RoleSundry::timerSaveSundry()
{
    std::string sundry;
    water::componet::Serialize<std::string> ss;
    ss.reset();
//添加数据
/*频繁定时更新数据*/
    ss << m_testTimerData;
    ss << m_worldBossDamage;
    ss << m_receivedWBDamageAward;
    
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    auto msg = reinterpret_cast<PrivateRaw::TimerSundryToDB*>(buf.data());
    buf.resize(sizeof(PrivateRaw::TimerSundryToDB) + ss.tellp());
    std::memcpy(msg->data, ss.buffer()->data(), ss.tellp());
    msg->roleId = m_roleId;
    msg->size = ss.tellp();
    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(TimerSundryToDB), buf.data(), buf.size());
}

void RoleSundry::saveSundry()
{
    std::string sundry;
    water::componet::Serialize<std::string> ss;
    ss.reset();
//添加数据
/*不常更新数据*/
    ss << m_refreshedFactionShop;
    ss << m_refreshFacShopTime;
    ss << m_facShopTab;
    ss << m_facShopGoodsId;
    ss << m_buyStoreDayLimitObj;
    ss << privateBossMap;
    ss << m_dragonSkills;
    ss << m_energe;

    ss << m_heroData.heroSkillSetting;

    std::vector<uint8_t> buf;
    buf.reserve(1024);
    auto msg = reinterpret_cast<PrivateRaw::SundryToDB*>(buf.data());
    buf.resize(sizeof(PrivateRaw::SundryToDB) + ss.tellp());
    std::memcpy(msg->data, ss.buffer()->data(), ss.tellp());
    msg->roleId = m_roleId;
    msg->size = ss.tellp();
    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(SundryToDB), buf.data(), buf.size());
}

}
