#include "test.h"
#include "../lock_free_circular_queue_ss.h"

using namespace water;
using namespace componet;

int main()
{
    std::vector<componet::LockFreeCircularQueueSPSC<std::shared_ptr<int>>*> queues;
    for(int i= 0; i < 10000; ++i)
    {
        queues.emplace_back(new componet::LockFreeCircularQueueSPSC<std::shared_ptr<int>>(9));
    }

    int i;
    cin >> i;
    return 0;
}
