/*
 * Author: LiZhaojia
 *
 * Created: 2015-08-18 11:54 +0800
 *
 * Modified: 2015-08-18 20:45 +0800
 *
 * Description: 游戏区管理
 */

#include "water/componet/spinlock.h"
#include "water/net/endpoint.h"

#include "water/process/process_id.h"
#include "water/process/tcp_message.h"

#include "protocol/protobuf/proto_manager.h"

#include <atomic>
#include <list>
#include <mutex>


namespace super{

using water::process::Platform;
using water::process::ZoneId;
using water::process::ProcessType;
using water::process::ProcessIdentity;
using water::process::TcpMsgCode;
using water::net::Endpoint;
using protocol::protobuf::ProtoMsgPtr;

class ZoneManager
{
public:
    void loadConfig(const std::string& cfgDir);
    void timerExec(const water::componet::TimePoint& now);

    void onZoneDisconnect(ProcessIdentity pid);

    void regMsgHandler();

    ZoneId selectZone(const Platform& platform);
    Endpoint getGatewayEndpintByZoneId(ZoneId zoneId) const;

private://消息处理
    void zonemsg_ZoneActiveInfo(const ProtoMsgPtr& proto, ProcessIdentity pid);

private:
    typedef std::lock_guard<water::componet::Spinlock> LockGuard;


    std::map<Platform, std::map<ZoneId, Endpoint>> m_allPlatformZone;

    struct 
    {
        mutable water::componet::Spinlock lock;
        std::map<Platform, std::map<ZoneId, uint16_t>> zones; //
    } m_curActiveZone;

public:
    static ZoneManager& me();
    static Platform getPlatformByZoneId(ZoneId zoneId);
};


}
