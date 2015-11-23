/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-25 20:54 +0800
 *
 * Modified: 2015-05-27 16:24 +0800
 *
 * Description: 通用的 A*寻路,  
 *              坐标表示方式, 邻接坐标计算, 阻挡检测, 单步代价, 启发函数 全部允许调用者定制
 *              所以此算法独立于 地图的具体实现以及路径代价判断规则
 *              可适用于各种2d, 3d寻路
 *
 *              可参考example中的测试用例
 */

#ifndef WATER_COMPONET_ASTAR_H
#define WATER_COMPONET_ASTAR_H

#include <queue>
#include <list>
#include <unordered_map>

namespace water{
namespace componet{


template <
         typename Position,  //坐标位置的类型, 仅要求该类型可以放入std::unordered_map
         typename Nighbors,   //计算pos的邻居  Container<Position> nighbors(const Position& pos)
         typename Enterable,  //检查pos的阻挡  bool enterable(const Position& pos)
         typename StepCost,   //from到to的权重 int32_t stepCos(const Position& from, const Position& to)
         typename Heruistic   //距离的启发函数 int32_t heruistic(const Position& pos, const Position& goal)
         >
class AStar
{
    class PriorityQueue
    {
        using Item = std::pair<int32_t, Position>;
    public:
        void push(int32_t cost, const Position& pos)
        {
            m_stdPriorityQueue.emplace(cost, pos);
        }

        Position pop()
        {
            Position ret = m_stdPriorityQueue.top().second;
            m_stdPriorityQueue.pop();
            return ret;
        }

        void clear()
        {
            std::priority_queue<Item, std::vector<Item>, std::greater<Item>> emptyQueue;
            m_stdPriorityQueue.swap(emptyQueue);
        }

        bool empty() const
        {
            return m_stdPriorityQueue.empty();
        }

    private:
        std::priority_queue<Item, std::vector<Item>, std::greater<Item>> m_stdPriorityQueue; //小顶堆
    };

    enum { INVALID_COST = -1 };

    struct PosAttr
    {
        Position prevPos;
        int32_t cost = INVALID_COST;
    };

public:
    AStar() = default;
    AStar(const Nighbors& nighbors,
          const Enterable& enterable,
          const StepCost& stepCost,
          const Heruistic& heruistic)
    : m_nighbors(nighbors), m_enterable(enterable), m_stepCost(stepCost), m_heruistic(heruistic)
    {
    }

    ~AStar() = default;

    std::list<Position> findPath(const Position& start, const Position& goal)
    {
        m_frontier.clear();
        m_searchedArea.clear();

        m_frontier.push(0, start);
        m_searchedArea[start].cost = 0;

        //离终点最近的可达点, 即路径的末尾
        std::pair<int32_t, Position> nearestToGoal(m_heruistic(start, goal), start);

        while(!m_frontier.empty())
        {
            const Position curPos = m_frontier.pop();
            if(curPos == goal) //已达终点, 寻路成功
            {
                nearestToGoal.second = goal;
                break;
            }

            PosAttr& curAttr = m_searchedArea[curPos];

            for(const auto& nextPos : m_nighbors(curPos))
            {
                if(!m_enterable(nextPos))
                    continue;

                const int32_t nextCost = curAttr.cost + m_stepCost(curPos, nextPos);

                PosAttr& nextAttr = m_searchedArea[nextPos];

                //在格子第一次被检查的情况 或
                //新的可能经过此格子的路径, 开销低于上次的检查结果的情况下
                //把格子当前的开销和前驱点存起来,  
                //并把格子计入当前的边界
                if(nextAttr.cost == INVALID_COST ||
                   nextCost < curAttr.cost)
                {
                    const int32_t nextHeruistic = m_heruistic(nextPos, goal);
                    //存下新的路径代价和前驱
                    nextAttr.prevPos = curPos;
                    nextAttr.cost    = nextCost;

                    //放入边界列表
                    m_frontier.push(nextCost + nextHeruistic, nextPos);

                    //可能的距终点最近点
                    if(nextHeruistic < nearestToGoal.first)
                    {
                        nearestToGoal.first  = nextHeruistic;
                        nearestToGoal.second = nextPos;
                    }
                }

            }
        }

        std::list<Position> path;
        for(Position pos = nearestToGoal.second; pos != start; pos = m_searchedArea[pos].prevPos)
            path.push_front(pos);
        return path;
    }

    Nighbors& nighbors()
    {
        return m_nighbors;
    }

    Enterable& enterable()
    {
        return m_enterable;
    }

    StepCost& stepCost()
    {
        return m_stepCost;
    }

    Heruistic& heruistic()
    {
        return m_heruistic;
    }


private:
#ifdef COMPONET_DEBUG
public:
#endif
    Nighbors  m_nighbors;
    Enterable m_enterable;
    StepCost  m_stepCost;
    Heruistic m_heruistic;

    std::unordered_map<Position, PosAttr> m_searchedArea; //已搜索过的区域
    PriorityQueue m_frontier;                             //搜索过的区域的边界
};

template<typename Position, typename Nighbors, typename Enterable, typename StepCost, typename Heruistic>
inline std::list<Position> findPath(const Nighbors& nighbors,
                                    const Enterable& enterable,
                                    const StepCost& stepCost,
                                    const Heruistic& heruistic, 
                                    const Position& start, const Position& goal)
{
    AStar<Position, Nighbors, Enterable, StepCost, Heruistic> astar(nighbors,
                                                                    enterable,
                                                                    stepCost,
                                                                    heruistic);
    return astar.findPath(start, goal);
}

}}

#endif

