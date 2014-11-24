/*
 * Author: LiZhaojia - dantezhu@vip.qq.com
 *
 * Last modified: 2014-07-24 17:29 +0800
 *
 * Description: 通用反射机制，可以通过name来索引不同的类型，并在运行期动态的通过name来构建对应类型的对象
 */

#ifndef WATER_BASE_REFLECTOR_H
#define WATER_BASE_REFLECTOR_H

#include <map>
#include <utility>

namespace water{
namespace componet{


//反射构建器，args   
template <typename Name, typename BaseType, typename... ConstructorArgs>
class Reflector
{
private:
    //抽象构建器， 实际构建器的基类
    class AbstructCreater
    {
    public:
        virtual ~AbstructCreater(){};
        virtual BaseType* create(ConstructorArgs&&... args) const = 0;
    };

    //实际对象构建器
    template <typename RealType>
    class Creater : public AbstructCreater
    {
        BaseType* create(ConstructorArgs&&... args) const override
        {
            return new(std::nothrow) RealType(std::forward<ConstructorArgs>(args)...); 
        }

        uint32_t sizeofObj() const
        {
            return sizeof(RealType);
        }
    };

public:
    //注册一种类型
    template <typename RealType>
    bool reg(const Name& name)
    {
        if(createrMap.find(name) != createrMap.end())//name 已被注册过， 不能重复注册
            return false;

        AbstructCreater* creater = new Creater<RealType>();
        if(creater == nullptr)
            return false;

        return createrMap.insert(std::make_pair(name, creater)).second;
    }

    //取消一种类型的注册
    void unreg(const Name& name)
    {
        createrMap.erase(name);
    }

    //创建一个指定类型的对象
    BaseType* produce(const Name& name, ConstructorArgs&&... args) const
    {
        typename std::map<Name, AbstructCreater*>::const_iterator it = createrMap.find(name);
        if(it == createrMap.end() || nullptr == it->second)
            return nullptr;

        return it->second->create(std::forward<ConstructorArgs>(args)...);
    }

    //得到指定类型的sizeof
    uint32_t sizeofType(const Name& name) const
    {
        typename std::map<Name, AbstructCreater*>::const_iterator it = createrMap.find(name);
        if(it == createrMap.end() || nullptr == it->second)
            return 0;

        return it->second->sizeofObj();
    }

private:
    std::map<Name, AbstructCreater*> createrMap;
};

}}

#endif


