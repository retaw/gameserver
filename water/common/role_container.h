/*
 * Author: LiZhaojia
 *
 * Created: 2015-03-25 14:34 +0800
 *
 * Modified: 2015-03-25 14:34 +0800
 *
 * Description: 角色容器
 */

#ifndef COMMON_ROLE_CONTAINER_H
#define COMMON_ROLE_CONTAINER_H

#include "roledef.h"


#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

template <typename RoleHandle, typename IdType = RoleId>
class RoleContainer
{
    using Index          = typename std::vector<RoleHandle>::size_type;
public:
    using iterator       = typename std::vector<RoleHandle>::iterator;
    using const_iterator = typename std::vector<RoleHandle>::const_iterator;

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

    RoleContainer() = default;
    ~RoleContainer() = default;

    uint32_t size() const
    {
        return m_vec.size();
    }

    bool insert(RoleHandle role)
    {
        const Index newIndex = m_vec.size();

        auto retById = m_byId.insert({role->id(), newIndex});
        if(retById.second == false)
            return false;

        auto retByName = m_byName.insert({role->name(), newIndex});
        if(retByName.second == false)
        {
            m_byId.erase(retById.first);
            return false;
        }

        //因为可以接受key重复, 所以multimap::insert 不会失败
        m_byAccount.insert({role->account(), newIndex});
        m_vec.push_back(role);
        return true;
    }

    RoleHandle getById(IdType id) const
    {
        auto it = m_byId.find(id);
        if(it == m_byId.end())
            return nullptr;

        return m_vec.at(it->second);
    }

    RoleHandle getByName(const std::string& name) const
    {
        auto it = m_byName.find(name);
        if(it == m_byName.end())
            return nullptr;

        return m_vec.at(it->second);
    }

    std::vector<RoleHandle> getByAccount(const std::string& account) const
    {
        std::vector<RoleHandle> ret;
        auto iterRange = m_byAccount.equal_range(account);
        for(auto it = iterRange.first; it != iterRange.second; ++it)
            ret.push_back(m_vec.at(it->second));

        return ret;
    }

    void erase(RoleHandle handle)
    {
        if(m_vec.size() == 0)
            return;

        if(m_vec.size() == 1)
        {
            m_byId.clear();
            m_byName.clear();
            m_byAccount.clear();
            m_vec.clear();
            return;
        }

        auto idIt = m_byId.find(handle->id());
        if(idIt == m_byId.end())
            return;

        Index index = idIt->second;

        //尾删除
        if(index + 1 != m_vec.size())
        {
            m_vec[index] = m_vec.back();
            Index oldBackIndex = m_vec.size();
            RoleHandle oldBackRole = m_vec[index];

            //更新3个map中的index
            m_byId[oldBackRole->id()] = index;
            m_byName[oldBackRole->name()] = index;
            auto iterRange = m_byAccount.equal_range(oldBackRole->account());
            for(auto it = iterRange.first; it != iterRange.second; ++it)
            {
                if(it->second == oldBackIndex)
                {
                    it->second = index;
                    break;
                }
            }
        }
        m_vec.pop_back();

        //将3个map中相关的数据删除
        m_byId.erase(handle->id());
        m_byName.erase(handle->name());
        auto iterRange = m_byAccount.equal_range(handle->account());
        for(auto it = iterRange.first; it != iterRange.second; ++it)
        {
            if(it->second == index)
            {
                m_byAccount.erase(it);
                break;
            }
        }
    }

    void eraseById(IdType id)
    {
        auto itById = m_byId.find(id);
        if(itById == m_byId.end())
            return;

        erase(m_vec[itById->second]);
    }

    void eraseByName(const std::string& name)
    {
        auto itByName = m_byName.find(name);
        if(itByName == m_byName.end())
            return;

        erase(m_vec[itByName->second]);
    }

private:
    std::vector<RoleHandle> m_vec;
    std::unordered_map<IdType, Index> m_byId;
    std::unordered_map<std::string, Index> m_byName;
    std::unordered_multimap<std::string, Index> m_byAccount;
};

#endif
