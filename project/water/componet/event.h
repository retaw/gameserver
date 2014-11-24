/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-29 20:31 +0800
 *
 * Description: 
 */

#ifndef WATER_EVENT_HPP
#define WATER_EVENT_HPP

#include <functional>
#include <memory>
#include <unordered_map>
//#include <map>
#include <vector>
#include <cstdint>

namespace water{
namespace componet{


template<typename T>
class Event;


template<typename... ParamType>
class Event<void (ParamType...)>
{
public:
    typedef std::function<void (ParamType...)> Handler;
    typedef uint32_t RegID;

    enum {INVALID_REGID = 0};

    RegID reg(Handler cb)
    {
        const RegID ID = lastRegID + 1;

        const auto it = regIDs.insert(regIDs.end(), std::make_pair(ID, callbackList.size()));
        if(it == regIDs.end())
            return INVALID_REGID;

        callbackList.emplace_back(ID, cb);
        it->second = callbackList.size() - 1;

        lastRegID = ID;
        return ID;
    }

    void unreg(RegID regID)
    {
        if(callbackList.size() == 1)
        {
            callbackList.clear();
            regIDs.clear();
            return;
        }

        auto it = regIDs.find(regID);
        if(it == regIDs.end())
            return;

        //尾删除
        callbackList.at(it->second) = callbackList.back();
        callbackList.pop_back();

        //更新索引
        regIDs[callbackList.at(it->second).first] = it->second;

        //删掉老的索引
        regIDs.erase(it);
    }

//    void raise(ParamType... args) noexcept
    void operator()(ParamType... args) noexcept
    {
        for(auto& cb : callbackList)
        {
            try
            {
                //cb.second(std::forward<ParamType>(args)...);
                cb.second(args...);
            }
            catch(...)
            {
            }
        }
    }

private:
    std::vector<std::pair<RegID, Handler>> callbackList;
    std::unordered_map<RegID, typename std::vector<Handler>::size_type> regIDs;
    RegID lastRegID = INVALID_REGID;
};

/*
template <typename T>
class Delegate;

template <typename Ret, typename... Param>
class Delegate<Ret (Param...)>
{
    typedef std::function<Ret(Param...)> Callable;
public:
    class Hash
    {
    public:
        size_t operator()(const Delegate& d) const
        {
            return std::hash<const void*>()(d.m_cb.get());
        }
    };

public:
    Delegate(const Delegate&) = default;
    Delegate& operator=(const Delegate&) = default;

    Delegate(const Callable& cb)
    :m_cb(std::make_shared<Callable>(cb))
    {
    }

    Delegate& operator=(Callable cb)
    {
        m_cb = cb;
        return *this;
    }

    Ret operator() (Param... args)
    {
        (*m_cb)(std::forward<Param>(args)...);
    }

    bool operator < (const Delegate& other) const
    {
        return m_cb < other.m_cb;
    }

    bool operator > (const Delegate& other) const
    {
        return m_cb > other.m_cb;
    }

    bool operator == (const Delegate& other) const
    {
        return m_cb == other.m_cb;
    }

private:
    std::shared_ptr<Callable> m_cb;
};

template<typename T>
class Event;


template<typename... ParamType>
class Event<void (ParamType...)>
{
public:
    typedef Delegate<void (ParamType...)> Handler;
    typedef uint32_t RegID;

    enum {INVALID_REGID = 0};

    void operator+=(Handler cb)
    {
        auto it = regIDs.insert(std::make_pair(cb, callbackList.size())).first;
        callbackList.emplace_back(cb);
        it->second = callbackList.size() - 1;
    }

    void operator-=(Handler cb)
    {
        if(callbackList.size() == 1)
        {
            callbackList.clear();
            regIDs.clear();
            return;
        }

        auto it = regIDs.find(cb);
        if(it == regIDs.end())
            return;

        //尾删除
        callbackList.at(it->second) = callbackList.back();
        callbackList.pop_back();

        //更新索引
        regIDs[callbackList.at(it->second)] = it->second;

        //删掉老的索引
        regIDs.erase(it);
    }

    void operator()(ParamType... args) noexcept
    {
        for(auto& cb : callbackList)
        {
            try
            {
                //cb.second(std::forward<ParamType>(args)...);
                cb(args...);
            }
            catch(...)
            {
            }
        }
    }

private:
    std::vector<Handler> callbackList;
    std::unordered_map<Handler, typename std::vector<Handler>::size_type, typename Handler::Hash> regIDs;
    RegID lastRegID = INVALID_REGID;
};
*/

}}
#endif
