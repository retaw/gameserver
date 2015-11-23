/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-10 17:53 +0800
 *
 * Modified: 2015-04-10 17:53 +0800
 *
 * Description: 二维坐标位置
 */

#ifndef WATER_COMPONET_COORD_H
#define WATER_COMPONET_COORD_H

#include <string>
#include <array>
#include <cmath>

namespace water{
namespace componet{

//方向
enum class Direction : uint32_t
{//这里初始的枚举值定义的应该按照时针方向依次累加, 造成不方便相对方向计算;
   // 现在没法改了, 有机会重构时要纠正
    none     = 0,
    leftup   = 1, up   = 2, rightup   = 3,
    left     = 4,           right     = 5,
    leftdown = 6, down = 7, rightdown = 8,
};

//相反的方向
Direction reverseDirection(Direction dir);
Direction relativeDirection_2_absoluteDirection(Direction origion, Direction relative);

//1维坐标
typedef int16_t Coord1D;

//2维坐标
struct Coord2D 
{
public:
    Coord2D(Coord1D x_ = 0, Coord1D y_ = 0);
    explicit Coord2D(const std::string& str);

    void fromString(const std::string& str);
    std::string toString() const;

    uint32_t toInt() const
    {
        return (uint32_t(ux) << 16) | uy;
    }

    bool operator != (const Coord2D& other) const
    {
        return toInt() != other.toInt();
    }

    bool operator == (const Coord2D& other) const
    {
        return toInt() == other.toInt();
    }

    //由toInt的算法保证x优先, x相同时比较y的值
    bool operator < (const Coord2D& other) const
    {
        return toInt() < other.toInt();
    }

    int32_t manhattanDistance(const Coord2D& other) const
    {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    //到目标点的方向, 不在8个标准方向上的点取最近似的标准方向
    Direction direction(const Coord2D& pos) const;

    //邻居
    Coord2D neighbor(Direction dir) const;

    //全部邻居
    std::array<Coord2D, 8> allNeighbors() const;

    //已自己为中心的9个格子, 返回值已按 operator< 规则排序
    std::array<Coord2D, 9> meAndAllNeighbors() const;

    //到另一个格子的中点
    Coord2D middle(const Coord2D& other) const;

    Coord1D distance(const Coord2D& other) const
    {
        Coord1D diffX = x - other.x;
        Coord1D diffY = y - other.y;
        return std::sqrt(diffX * diffX + diffY * diffY);
    }

    static Coord1D distance(const Coord2D& c1, const Coord2D& c2)
    {
        return c1.distance(c2);
    }



public:
    union
    {
        Coord1D x;
        uint16_t ux;
    };
    union
    {
        Coord1D y;
        uint16_t uy;
    };
};


}}


//使Coord2D可以放入hash容器作为key
namespace std{

template<>
class hash<water::componet::Coord2D>
{
public:
    size_t operator()(const water::componet::Coord2D& coord) const
    {
        return hash<uint32_t>()(coord.toInt());
    }
};

}

#endif

