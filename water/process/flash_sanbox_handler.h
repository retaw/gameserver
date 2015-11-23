/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-26 17:35 +0800
 *
 * Modified: 2015-03-26 17:35 +0800
 *
 * Description: 处理flash 连接的安全沙箱验证
 */

#ifndef GATEWAY_FLASH_SANDBOX_HANDLER
#define GATEWAY_FLASH_SANDBOX_HANDLER

//#include "common/commdef.h"
#include "componet/spinlock.h"
#include "componet/datetime.h"
#include "net/packet_connection.h"
#include "process_thread.h"

#include <list>
#include <memory>

namespace water{
namespace process{

class FlashSandboxHandler : public ProcessThread
{
public:
    TYPEDEF_PTR(FlashSandboxHandler)
    CREATE_FUN_MAKE(FlashSandboxHandler)

    FlashSandboxHandler() = default;
    ~FlashSandboxHandler() = default;

    void addFlashSandboxConn(net::PacketConnection::Ptr conn);

private:
    bool exec() override;

private:
    enum class ConnState
    {
        recving,   //接收
        sending,   //发送
        waitclose, //等对方关闭
    };

    struct ConnHolder
    {
        ConnState state;
        componet::TimePoint tp;
        net::PacketConnection::Ptr conn;
    };

    std::list<ConnHolder> m_conns;
    componet::Spinlock m_lock;
};

}}

#endif

