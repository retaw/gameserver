/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server 的进程定义
 */

#include "water/process/process.h"
#include "common/commdef.h"
#include "water/process/tcp_message.h"

namespace protocol{namespace protobuf{ class Message; } };

namespace func{

using namespace water;
using namespace process;

class Func : public process::Process
{
public:
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code);
    bool sendToPrivate(ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size);

    bool relayToPrivate(uint64_t sourceId, ProcessIdentity pid, TcpMsgCode code, const void* raw, uint32_t size);

	void broadcastToWorlds(TcpMsgCode code, const void* raw, uint32_t size); 

private:
    Func(int32_t num, const std::string& configDir, const std::string& logDir);


    void tcpPacketHandle(TcpPacket::Ptr packet, 
                         TcpConnectionManager::ConnectionHolder::Ptr conn,
                         const componet::TimePoint& now) override;

    void init() override;
	void stop() override;
//    void lanchThreads() override;
    void registerTcpMsgHandler();
    void registerTimerHandler();

public:
    static void init(int32_t num, const std::string& configDir, const std::string& logDir);
    static Func& me();

    void loadConfigAndDB();

private:
    static Func* m_me;
};

}
