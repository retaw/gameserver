#include "test.h"
#include "../astar.h"
#include "../coord.h"

using namespace water;
using namespace componet;

using Position = water::componet::Coord2D;

struct Nighbors
{
    std::array<Position, 8> operator()(const Position& pos) const
    {
        return pos.allNighbors();
    }
};

struct Heruistic
{
    int32_t operator() (const Position& pos, const Position& goal) const
    {
        return 8 * (std::abs(pos.x - goal.x) + std::abs(pos.y - goal.y));
    }
};

struct StepCost
{
    int32_t operator()(const Position& from, const Position& to) const
    {
        if(from.x == to.x || from.y == to.y)
            return 10;
        return 16;
    }
};


std::vector<std::string> scene =
{
    "                     ",
    "  ################   ",
    "  #              #G  ",
    "  #              #   ",
    "  #              #   ",
    "  # S            #   ",
    "  #              #   ",
    "  ############## #   ",
    "                 #   ",
    "                 #   ",
    "                 #   ",
    "                 #   ",
    "                 #   ",
    "  ################   ",
    "                     ",
    "                     ",
};

struct Enterable
{
    bool operator()(const Position& pos) const
    {
        if(pos.x < 0 || pos.x >= scene.size())
            return false;
        if(pos.y < 0 || pos.y >= scene[pos.x].size())
            return false;

        return scene[pos.x][pos.y] != '#';
    }

    std::vector<std::string> scene;
};



int main()
{
    Position start;
    Position goal;
    for(int i = 0; i < scene.size(); ++i)
    {
        for(int j = 0; j < scene[i].size(); ++j)
        {
            if(scene[i][j] == 'S')
            {
                start.x = i;
                start.y = j;
            }
            else if(scene[i][j] == 'G')
            {
                goal.x = i;
                goal.y = j;
            }
        }
    }
//    Nighbors n;
//    Enterable e;
//    StepCost s;
//    Heruistic h;
//    e.scene = scene;
//
//    std::list<Position> path = findPath(n, e, s, h, start, goal);

    AStar<Position, Nighbors, Enterable, StepCost, Heruistic> astar;
    astar.enterable().scene = scene;
    std::list<Position> path = astar.findPath(start, goal);

    auto resultScene = scene;
    for(auto pos : path)
        resultScene[pos.x][pos.y] = '*';
    for(const auto& line : resultScene)
        cout << line << endl;
    cout << endl;

#ifdef COMPONET_DEBUG

    std::vector<std::vector<int32_t>> costScene(scene.size());
    for(auto& line : costScene)
        line.resize(scene[1].size());

    for(auto item : astar.m_searchedArea)
        costScene[item.first.x][item.first.y] = item.second.cost + Heruistic()(item.first, goal);

    for(int i = 0; i < costScene.size(); ++i)
    {
        for(int j = 0; j < costScene[i].size(); ++j)
            cout << setw(5) << costScene[i][j];
        cout << endl;
    }
#endif

    return 0;
}

