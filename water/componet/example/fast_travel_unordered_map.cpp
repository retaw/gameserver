#include "../fast_travel_unordered_map.h"
#include "../random.h"
#include "test.h"



using namespace water;
using namespace componet;
using namespace test;

int main()
{
    FastTravelUnorderedMap<uint32_t, uint32_t> ftum;
    std::unordered_map<uint32_t, uint32_t> um;

    const uint64_t banch = 30000000; // 3kw

    cout << "filling data ..." << endl;
    for(uint32_t i = 0; i < banch; ++i)
    {
        if(i % 1000 == 0)
            cout << "filled " << i << " items" << endl;

        Random<uint32_t> random;
        uint32_t k = random.get();
        uint32_t v = random.get();

        ftum.insert(std::make_pair(k,v));
        um.insert(std::make_pair(k,v));
    }

    cout << "fill data done" << endl;
    cout << "checking contintor ..." << endl;

    //size
    if(ftum.size() != um.size())
    {
        cout << "check size failed!!! ftum.size()=" << ftum.size() << ", um.size()=" << um.size() << endl;
        return 0;
    }
    cout << "check size ok, ftum.size()=" << ftum.size() << ", um.size()=" << um.size() << endl;

    //itmes
    cout << "checking items ..." << endl;
    int i = 0;
    for(auto iter = um.begin(); iter != um.end(); ++iter)
    {
        if(i++ % 1000 == 0)
            cout << "checked " << i << " items" << endl;

        auto ftIter = ftum.find(iter->first);
        if(ftIter->second != iter->second)
        {
            cout << "check items failed, found diffence. ftumItem" << *ftIter << "umItem" << *iter << endl;
            return 0;
        }
    }

    cout << "check data done" << endl;
    return 0;
}
