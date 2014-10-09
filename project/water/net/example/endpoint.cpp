#include "test.h"
#include "../endpoint.h"

using namespace water;
using namespace net;

int main()
{
    cout << IpV4::getAddrByIfrName("eth0") << endl;
    cout << IpV4::getAddrByIfrName("lo") << endl;
    cout << IpV4::getAddrByIfrName("em:1") << endl;
    return 0;
}
