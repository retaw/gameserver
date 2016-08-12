/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 10:52 +0800
 *
 * Modified: 2015-04-14 10:52 +0800
 *
 * Description: 可快速遍历的unordered_map, 
 *              按值存储, 删除操作有对象复制的开销
 *              迭代器在变动性操作后不保证有效
 *          ！！！！！！ 这个实现具有重大bug，没有考虑stlunordered_map会自动rehash的问题，会造成内存错误而宕机。
 *                       已经发现很久了，由于换了工作，一直没有来得及修复，特此注明
 */

#ifndef WATER_QUICK_SEARCH_VECTOR_HPP
#define WATER_QUICK_SEARCH_VECTOR_HPP

#include "exception.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace water{
namespace componet{

DEFINE_EXCEPTION(InvalidIter, ExceptionBase)


template<typename Key, typename Value>
class FastTravelUnorderedMap
{
public:
    using iterator       = typename std::vector<std::pair<Key, Value>>::iterator;
    using const_iterator = typename std::vector<std::pair<Key, Value>>::const_iterator;

    iterator begin()
    {
        return m_vec.begin();
    }

    iterator end()
    {
        return m_vec.end();
    }

    const_iterator begin() const
    {
        return m_vec.begin();
    }

    const_iterator end() const
    {
        return m_vec.end();
    }

    uint32_t size() const
    {
        return m_vec.size();
    }

    bool empty() const
    {
        return m_vec.empty();
    }

    bool insert(const std::pair<Key, Value>& item)
    {
        const auto insertRet = m_map.insert(std::make_pair(item.first, m_vec.size()));
        if(!insertRet.second)
            return false;

        m_vec.emplace_back(item);
        insertRet.first->second = m_vec.size() - 1;

        return true;
    }

    iterator find(Key key)
    {
        auto mapIter = m_map.find(key);
        if(mapIter == m_map.end())
            return m_vec.end();

        return m_vec.begin() + mapIter->second;
    }

    const_iterator find(Key key) const
    {
        auto mapIter = m_map.find(key);
        if(mapIter == m_map.end())
            return m_vec.end();

        return m_vec.begin() + mapIter->second;
    }

    iterator erase(iterator iter)
    {
        auto mapIt = m_map.find(iter->first);
        //只要iter合法, 这里mapIt一定合法, so, 这个异常不要抓, 就让宕掉好了
        if(mapIt == m_map.end())
            EXCEPTION(InvalidIter, "FastTravelUnorderedMap 出现map和vec不一致"); 

        const auto index = mapIt->second;
        erase(iter->first);
        return m_vec.begin() + index; //原来的位置, 即为逻辑上的"下一个"
    }

    void erase(Key key)
    {
        auto it = m_map.find(key);
        if(it == m_map.end())
            return;

        if(m_vec.size() == 1)
        {
            m_vec.clear();
            m_map.clear();
            return;
        }

        //当要删的结点不在vec的尾部, 用当前的尾部覆盖要删的节点
        if(it->second + 1 != m_vec.size())
        {
            m_vec.at(it->second) = m_vec.back();
            //更新map中的index
            m_map[m_vec.at(it->second).first] = it->second;
        }
        //删掉vec的尾部
        m_vec.pop_back();

        //删掉老的索引
        m_map.erase(it);
    }

    std::vector<Value> getValues() const
    {
        std::vector<Value> ret;
        ret.reserve(size());
        for(auto it = begin(); it != end(); ++it)
            ret.push_back(it->second);

        return ret;
    }

private:
    std::vector<std::pair<Key, Value>> m_vec;//{{kye, value},...}
    std::unordered_map<Key, typename std::vector<Value>::size_type> m_map;  //{key, indexOfVec} 这里存的是下标, 所以这个封装可以直接支持default copy
};


}}
#endif
