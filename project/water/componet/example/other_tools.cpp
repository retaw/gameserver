#include "test.h"
#include "../other_tools.h"

enum class E : int
{
    a, b, c
};


int main()
{
    E e = E::b;

    cout << water::e2i(e) << endl;
}
