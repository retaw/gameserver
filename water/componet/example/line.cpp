#include "../line.h"
#include "test.h"


using namespace water;
using namespace componet;

struct Point
{
    Point(int n, Coord2D c)
    {
        num = n;
        coord = c;
    }
    int num;
    Coord2D coord;
};

std::vector<Point> points =
{
    {1, {2, 1}},
    {2, {2.5, 0}},
    {3, {1, 3.0}},
    {4, {1, 3.1}},
    {5, {1, 3.001}},
    {6, {-2, -1}},
    {7, {2.5, 0}},
};

Line2D l1(Coord2D(0, 0), Coord2D(20, 10));
Line2D l2 = l1.vertical(Coord2D(0, 0));

Line2D l = l2;

bool operator < (const Point& p1, const Point& p2)
{
    return l.distance(p1.coord) < l.distance(p2.coord);
}


int main()
{
    for(auto& p : points)
        cout << l2.distance(p.coord) << ", ";
    cout << endl;

    std::multiset<Point> s(points.begin(), points.end());
    cout << std::distance(points.begin(), points.end()) << endl;
    cout << s.size() << endl;
    for(auto& p : s)
        cout << p.num << " < ";
    cout << endl;

    Line2D ll(Coord2D(2, 1), Coord2D(1, 3));
    Line2D ly(INFINITY, 0);
    Line2D lx(0, 0);

    cout << ll.cross(ly).toString() << endl;
    cout << ll.cross(lx).toString() << endl;

    Line2D l45(Coord2D(0, 0), Vector2D::unit(3.14159/4));
    Line2D lv1(Coord2D(1, 0), INFINITY);
    cout << Line2D::cross(l45, lv1).toString() << endl;

    auto radian = Vector2D(-1, -1).unit().radian();
    cout << radian << endl;
    cout << Vector2D::unit(radian).toString() << endl;


    auto pos = Coord2D(1, 2);
    auto dir = Vector2D(-1, -1).unit();
    auto length = 5;

    Line2D l0(pos, dir);
    auto c0x = Line2D::cross(lx, l0);
    auto c0y = Line2D::cross(ly, l0);
    cout << Line2D::cross(ly, l0).toString() << endl;
    cout << Line2D::cross(lx, l0).toString() << endl;


    return 0;
}
