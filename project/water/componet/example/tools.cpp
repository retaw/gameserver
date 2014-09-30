#include "../tools.h"

#include "test.h"

int main()
{
    int i = 101;
    int j = 0;

    std::string str1 = water::componet::toString(i);
    j = water::componet::fromString<int>(str1);

    cout << j << endl;
}
