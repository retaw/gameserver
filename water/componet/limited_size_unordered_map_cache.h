/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-14 10:52 +0800
 *
 * Modified: 2015-08-25 11:26 +0800
 *
 * Description: 带容量上限的unordered_map, 用作限体积缓存
 *              按值存储, 删除操作会有对象复制的开销
 *              迭代器在变动性操作后依然保持有效
 */

#ifndef WATER_SIZE_LIMITED_UNORDERED_MAP_CACHE_HPP
#define WATER_SIZE_LIMITED_UNORDERED_MAP_CACHE_HPP

#include "exception.h"

#include <memory>
#include <unordered_map>
#include <list>
#include <vector>
#include <cstdint>

namespace water{
namespace componet{

DEFINE_EXCEPTION(InvalidCacheIter, ExceptionBase)


template<typename Key, typename Value, uint32_t maxSize>
class LimitedSizeUnorderedMapCache
{
public:
    using iterator       = typename std::list<std::pair<Key, Value>>::iterator;
    using const_iterator = typename std::list<std::pair<Key, Value>>::const_iterator;

    iterator begin()
    {
        return m_list.begin();
    }

    iterator end()
    {
        return m_list.end();
    }

    const_iterator begin() const
    {
        return m_list.begin();
    }

    const_iterator end() const
    {
        return m_list.end();
    }

    uint32_t size() const
    {
        //这里用m_map而不用m_list看起来风格有点怪, 原因:
        //m_list在gcc下过去一直是O(n), 
        //gcc Wiki的Cxx11AbiCompatibility上讲gcc4.7和4.7.1已经改为用一个成员存储size, 所以理论上是O(1)了才对
        //但是gcc bugzilla, bug49561 的状态显示计划要到5.0版才能修掉, 所以用map.size()得了
        return m_map.size(); 
    }

    bool empty() const
    {
        return m_list.empty();
    }

    bool insert(const std::pair<Key, Value>& item)
    {
        //新放入的, 放在list的最前面
        m_list.emplace_front(item);
        const auto insertRet = m_map.insert(std::make_pair(item.first, m_list.begin()));
        if(!insertRet.second)
        {
            m_list.pop_front();
            return false;
        }

        //检查size, 保证size <= maxSize
        if(m_map.size() > maxSize)
        {
            m_map.erase(m_list.back().first);
            m_list.pop_back();
        }

        return true;
    }

    iterator find(Key key)
    {
        auto mapIter = m_map.find(key);
        if(mapIter == m_map.end())
            return m_list.end();

        //把要返回的item提到list的最前面
        m_list.splice(m_list.begin(), m_list, mapIter->second);
        mapIter->second = m_list.begin();

        return mapIter->second;
    }

    const_iterator find(Key key) const
    {
        auto mapIter = m_map.find(key);
        if(mapIter == m_map.end())
            return m_list.end();

        //把要返回的item提到list的最前面
        m_list.splice(m_list.begin(), m_list, mapIter->second);
        mapIter->second = m_list.begin();

        return mapIter->second;
    }

    iterator erase(iterator iter)
    {
        m_map.erase(iter->first);
        return m_list.erase(iter);
    }

    void erase(Key key)
    {
        auto mapIter = m_map.find(key);
        if(mapIter == m_map.end())
            return;

        erase(mapIter->second);
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
    mutable std::list<std::pair<Key, Value>> m_list;//{{kye, value},...}
    std::unordered_map<Key, iterator> m_map;  //{key, indexOfVec} 这里存的是下标, 所以这个封装可以直接支持default copy
};


}}
#endif
