#include "zone_manager.h"

//#include "water/componet/random.h"
#include "water/componet/logger.h"
#include "water/componet/xmlparse.h"

#include "protocol/protobuf/private/super.codedef.h"
#include "protocol/rawmsg/private/relay.codedef.private.h"

namespace super{

ZoneManager& ZoneManager::me()
{
    static ZoneManager me;
    return me;
}

Platform ZoneManager::getPlatformByZoneId(ZoneId zoneId)
{
    return static_cast<Platform>(zoneId % 10000);
}

/**********************************************************/

void ZoneManager::loadConfig(const std::string& cfgDir)
{
    const std::string fileName = "zone_list.xml";
    
    const std::string cfgFile = cfgDir + "/" + fileName;

    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode platformNode = root.getChild("platform"); platformNode; ++platformNode)
    {
        Platform platform = static_cast<Platform>(platformNode.getAttr<uint8_t>("id"));
        auto& platformZone = m_allPlatformZone[platform];

        for(XmlParseNode zoneNode = platformNode.getChild("zone"); zoneNode; ++zoneNode)
        {
            auto zoneId = zoneNode.getAttr<uint16_t>("id");
            platformZone[zoneId].fromString(zoneNode.getAttr<std::string>("gateway"));
        }
    }
}

void ZoneManager::timerExec(const water::componet::TimePoint& now)
{
}

void ZoneManager::onZoneDisconnect(ProcessIdentity pid)
{
    ProcessType type = pid.type();
    if(type != ProcessIdentity::stringToType("router"))
        return;

    Platform platform = getPlatformByZoneId(pid.zoneId());

    {
        LockGuard lock(m_curActiveZone.lock);
        m_curActiveZone.zones[platform].erase(pid.zoneId());
    }

    LOG_DEBUG("zone disconnected, id={}", pid.zoneId());
}

void ZoneManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_PROTO_PRIVATE(ZoneActiveInfo, std::bind(&ZoneManager::zonemsg_ZoneActiveInfo, this, _1, _2));
}

void ZoneManager::zonemsg_ZoneActiveInfo(const ProtoMsgPtr& proto, ProcessIdentity pid)
{
    auto rev = std::static_pointer_cast<PrivateProto::ZoneActiveInfo>(proto);

    Platform platform = getPlatformByZoneId(pid.zoneId());

    {
        LockGuard lock(m_curActiveZone.lock);
        m_curActiveZone.zones[platform][pid.zoneId()] = rev->player_load_reserve();
    }

    LOG_DEBUG("zone info updated, id={}", pid.zoneId());
}

Endpoint ZoneManager::getGatewayEndpintByZoneId(ZoneId zoneId) const
{
    Endpoint ret;
    Platform platform = getPlatformByZoneId(zoneId);
    
    auto it1 = m_allPlatformZone.find(platform);
    if(it1 == m_allPlatformZone.end())
        return ret;
    
    auto it2 = it1->second.find(zoneId);
    if(it2 == it1->second.end())
        return ret;

    return it2->second;
}

ZoneId ZoneManager::selectZone(const Platform& platform)
{
    ZoneId zoneId = water::process::INVALID_ZONE_ID;
    {
        LockGuard lock(m_curActiveZone.lock);
        auto it1 = m_curActiveZone.zones.find(platform);
        if(it1 == m_curActiveZone.zones.end())
            return zoneId;

        auto& platfromZones = it1->second;
        for(auto it2 = platfromZones.begin(); it2 != platfromZones.end(); ++it2)
        {
            if(it2->second > 0)
                zoneId = it2->first;
        }
    }
    
    return zoneId;
}


}
