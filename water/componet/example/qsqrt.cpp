#include "../qsqrt.h"

#include "test.h"

using namespace water;
using namespace componet;

uint32_t XX = 20;
uint32_t YY = 20;
vector<vector<int32_t>> vec(XX);

void funSqrt()
{
    for(uint32_t x = 0; x < XX; ++x)
    {
        for(uint32_t y = 0; y < YY; ++y)
            vec[x][y] = std::sqrt(x*x + y*y);
    }

    return;
}

void funQSqrt()
{
    for(uint32_t x = 0; x < XX; ++x)
    {
        for(uint32_t y = 0; y < YY; ++y)
            vec[x][y] = sqrt64(x*x + y*y);
    }

    return;
}

void funPow()
{
    for(uint32_t x = 0; x < XX; ++x)
    {
        for(uint32_t y = 0; y < YY; ++y)
            vec[x][y] = std::pow(x*x + y*y, 0.5);
    }

    return;
}

int main()
{
    for(uint32_t x = 0; x < XX; ++x)
        vec[x].resize(YY);

    performance(funSqrt, 1000, "Sqrt");
    for(int i = 0; i < XX; ++i)
    {
        for(int j = 0; j < YY; ++j)
            cout << vec[i][j] << " ";
        cout << endl;
    }

    performance(funQSqrt, 1000, "QSqrt");
    for(int i = 0; i < XX; ++i)
    {
        for(int j = 0; j < YY; ++j)
            cout << vec[i][j] << " ";
        cout << endl;
    }

    performance(funPow, 1000, "pow");
    for(int i = 0; i < XX; ++i)
    {
        for(int j = 0; j < YY; ++j)
            cout << vec[i][j] << " ";
        cout << endl;
    }
}
