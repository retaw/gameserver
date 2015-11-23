/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-21 16:31 +0800
 *
 * Description: 
 */

#include "msg_queue.h"

#include "packet.h"
#include "../componet/lock_free_circular_queue_ss.h"

#include <mutex>
#include <queue>

namespace water{
namespace net{

template <typename T>
class PacketQueue
{
    class Queue
    {
    public:
        std::queue<T>& data();

    private:
        std::mutex m_mutex;
        std::queue<T> m_queue;
    };

public:
    PacketQueue(uint32_t circularSize, uint32_t extendSizeLimit);
    ~PacketQueue();

private:
    componet::LockFreeCircularQueueSPSC<Packet::Ptr> m_circularQueue;
    Queue m_extendQueue;
    const uint32_t m_extendSizeLimit;
};


}}

