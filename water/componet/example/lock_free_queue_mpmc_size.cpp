#include "test.h"
#include "../lock_free_circular_queue_ss.h"

int main()
{
    int a;
    using Ptr = std::shared_ptr<int>;
    cout << 1 << endl;
    cin >> a;
    cout << 2 << endl;
    auto tmp = new water::componet::LockFreeCircularQueueSPSC<std::pair<Ptr, Ptr>>[5000];
    cin >> a;
    return 0;
}
