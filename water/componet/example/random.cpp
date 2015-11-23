#include "test.h"

#include "../random.h"

using namespace water;
using namespace componet;
using namespace test;

int main()
{
    {
        auto random = new Random<int, DistributionPattern::UNIFORM>(1, 6);

        for(int i = 0; i < 2; ++i)
            cout << random->get() << " ";
        cout << endl;
    }

    
    {
        auto random = new Random<float, DistributionPattern::NORMAL>(1, 6);

        for(int i = 0; i < 2; ++i)
            cout << random->get() << " ";
        cout << endl;
    }

    return 0;
}

