#include "../limited_size_unordered_map_cache.h"
#include "test.h"

using namespace water;
using namespace componet;

using namespace test;

int main()
{
    LimitedSizeUnorderedMapCache<int, int, 10> cache;

    for(int i = 0; i < 12; ++i)
    {
        cache.insert(std::make_pair(i, 100 + i));
    }
    cout << cache.getValues() << endl;

    auto it5 = cache.find(5);
    cout << cache.getValues() << endl;

    cache.insert({20, 120});
    cout << cache.getValues() << endl;

    cache.erase(it5);
    cout << cache.getValues() << endl;

    cache.erase(6);
    cout << cache.getValues() << endl;


    return 0;
}


