#include "test.h"

#include "../coord.h"

using water::componet::Coord2D;
using water::componet::Coord1D;
using water::componet::Direction;

int main()
{
    Coord1D XX = 30;
    Coord1D YY = 30;
    Coord1D X = 10;
    Coord1D Y = 10;
    Coord2D center(X,Y);

    for(Coord1D y = 0; y < XX; ++y)
    {
        for(Coord1D x = 0; x < YY; ++x)
        {
            cout << x << "," << y << "   ";
        }
        cout << endl;
    }

    for(Coord1D y = 0; y < XX; ++y)
    {
        for(Coord1D x = 0; x < YY; ++x)
        {
                cout << x - X << "," << y - Y << "   ";
        }
        cout << endl;
    }

    for(Coord1D y = 0; y < XX; ++y)
    {
        for(Coord1D x = 0; x < YY; ++x)
        {
            if(x == X)
                cout << "M" << "    ";
            else if( y == Y)
                cout << 0 << "    ";
            else
                cout << 1000*(x - X) / (y - Y) << "   ";
        }
        cout << endl;
    }

    for(Coord1D y = 0; y < XX; ++y)
    {
        for(Coord1D x = 0; x < YY; ++x)
        {
            Coord2D pos(x, y);
            cout << (int)(center.direction(pos)) << ", ";
        }
        cout << endl;
    }

    for(int i = 1; i < 9; ++i)
    {
        cout << i << ":" << endl;
        auto origionDir = static_cast<Direction>(i);
        for(int j = 1; j < 9; ++j)
        {
            auto relativeDir = static_cast<Direction>(j);
            auto absoluteDir = water::componet::relativeDirection_2_absoluteDirection(origionDir, relativeDir);
            cout << static_cast<int>(absoluteDir) << " ";
        }
        cout << endl << endl;
    }
}

