#include "stdint.h"


double sqrt64(double number)
{
    union
    {
        uint64_t i;
        double y;
    } num;

    const double x = number * 0.5;
    const double threehalfs = 1.5;

    num.y = number;
    num.i = 0x5fe6eb50c7b537a9 - (num.i >> 1);
    num.y = num.y * (threehalfs - (x * num.y * num.y));
   return num.y * number;
} 


/*

    uint64_t i;
    double x2, y;
    const double threehalfs = 1.5;

    x2 = number * 0.5;
    y  = number;
    i  = *reinterpret_cast<uint64_t*>(&y);
    i  = 0x5fe6eb50c7b537a9 - (i >> 1); 
    y  = *reinterpret_cast<double*>(&i);
    y  = y * (threehalfs - (x2 * y * y )); 
    return y * number;
}
*/

float sqrt32( float number )
{
    union Num
    {
        uint32_t i;
        float y;
    } num;

    const float x = number * 0.5F;
    const float threehalfs = 1.5F;

    num.y = number;
    num.i = 0x5f375a86 - (num.i >> 1);
    num.y = num.y * (threehalfs - (x * num.y * num.y));
    return num.y * number;
}

/*
float sqrt32( float number )
{
    uint32_t i;
    float x2, y;
    const float threehalfs = 1.5F;
 
    x2 = number * 0.5F;
    y  = number;
    i  = *reinterpret_cast<uint32_t*>(&y);
    i  = 0x5f375a86 - ( i >> 1 );
    y  = *reinterpret_cast<float*>(&i); 
    y  = y * (threehalfs - (x2 * y * y));
 
    return y * number;
}

*/
