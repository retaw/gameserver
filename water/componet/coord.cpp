#include "coord.h"

#include "string_kit.h"

#include <vector>
#include <map>

namespace water{
namespace componet{


Direction reverseDirection(Direction dir)
{
    auto value = static_cast<uint32_t>(dir);
    return static_cast<Direction>(9 - value);
}

Direction relativeDirection_2_absoluteDirection(Direction origion, Direction relative)
{
    enum Dir
    {
        none = 8,
        leftup   = 7, up   = 0, rightup   = 1,
        left     = 6,           right     = 2,
        leftdown = 5, down = 4, rightdown = 3,
    };
    Dir dirs[] = {none,
        leftup,   up,   rightup,
        left,           right,
        leftdown, down, rightdown};
    Direction directions[] = {Direction::none,
        Direction::leftup,   Direction::up,   Direction::rightup,
        Direction::left,                      Direction::right,
        Direction::leftdown, Direction::down, Direction::rightdown};
    
    std::map<Dir, Direction> dir_2_direction;
    std::map<Direction, Dir> direction_2_dir;
    for(auto i = 0; i < 9; ++i)
    {
        dir_2_direction[dirs[i]] = directions[i];
        direction_2_dir[directions[i]] = dirs[i];
    }

    Dir origionDir  = direction_2_dir[origion];
    Dir relativeDir = direction_2_dir[relative];
    Dir retDir = static_cast<Dir>((origionDir + (relativeDir - up)) % 8);

    return dir_2_direction[retDir];
}

/**********************************************/

Coord2D::Coord2D(Coord1D x_, Coord1D y_)
: x(x_), y(y_)
{   
}   

Coord2D::Coord2D(const std::string& str)
{
    fromString(str);
}   
void Coord2D::fromString(const std::string& str)
{
    std::vector<Coord1D> croods;
    water::componet::fromString(&croods, str, ",");
    if(croods.size() < 2)
        return;

    x = croods[0];
    y = croods[1];
}

std::string Coord2D::toString() const
{   
    char buf[128];
    snprintf(buf, sizeof(buf), "%d, %d", x, y); 
    return buf;
}


Direction Coord2D::direction(const Coord2D& pos) const
{
    if(pos == *this)
        return Direction::none;

    const int32_t subX = (pos.x - x);
    const int32_t subY = (pos.y - y);

    if(subX == 0)
        return subY > 0 ? Direction::down : Direction::up;
    else if(subY == 0)
        return subX > 0 ? Direction::right : Direction::left;

    const int32_t tan = 1000 * subY / subX;

    //1000 * tan(pi/8)  == 414
    //1000 * tan(3pi/8) == 2414
    //1000 * tan(-pi/8) == -414
    //1000 * tan(-3pi/8)== -2414

    if(tan >= 2414 || tan < -2414) //&& tan < infinite && tan > -infinite
    {
        return subY > 0 ? Direction::down : Direction::up;
    }
    else if(tan >= 414)// && tan < 2414) //水平方向
    {
        return subX > 0 ? Direction::rightdown : Direction::leftup;
    }
    else if(tan >= -414)// && tan < 414) //正斜率倾斜
    {
        return subX > 0 ? Direction::right : Direction::left;
    }
    else if(tan >= -2414)// && tan < -414) //负斜率倾斜
    {
        return subX > 0 ? Direction::rightup : Direction::leftdown;
    }

    return Direction::none;
}

Coord2D Coord2D::neighbor(Direction dir) const
{
    //方向向量
    struct Vec2D
    {
        Coord1D x;
        Coord1D y;
    };

    //方向的向量单位值
    const Vec2D dirVecUnit[] = 
    {
        {0, 0}, 
        {-1, -1}, {0, -1}, {+1, -1}, 
        {-1,  0},          {+1,  0}, 
        {-1, +1}, {0, +1}, {+1, +1}, 
    };
    Vec2D vec = dirVecUnit[static_cast<uint32_t>(dir)];
    return Coord2D(x + vec.x, y + vec.y);
}

std::array<Coord2D, 8> Coord2D::allNeighbors() const
{
    std::array<Coord2D, 8> ret =
    {
        neighbor(Direction::leftup),   neighbor(Direction::up),   neighbor(Direction::rightup),
        neighbor(Direction::left),                                neighbor(Direction::right),
        neighbor(Direction::leftdown), neighbor(Direction::down), neighbor(Direction::rightdown),
    };

    return ret;
}

std::array<Coord2D, 9> Coord2D::meAndAllNeighbors() const
{
    std::array<Coord2D, 9> ret =
    {
        *this,
        neighbor(Direction::leftup),   neighbor(Direction::up),   neighbor(Direction::rightup),
        neighbor(Direction::left),                                neighbor(Direction::right),
        neighbor(Direction::leftdown), neighbor(Direction::down), neighbor(Direction::rightdown),
    };

    return ret;
}

Coord2D Coord2D::middle(const Coord2D& other) const
{
    return Coord2D((x + other.x) / 2, (y + other.y) / 2);
}

}}
