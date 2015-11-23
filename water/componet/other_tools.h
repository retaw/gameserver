#ifndef WATER_COMPONET_OTHER_TOOLS_H
#define WATER_COMPONET_OTHER_TOOLS_H

#include  <type_traits>

namespace water{
namespace componet{

//枚举转整数
template <typename E>
typename std::enable_if<
                        std::is_enum<E>::value, 
                        typename std::underlying_type<E>::type
                        >::type
e2i(E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

//随机访问容器的尾删除
template <typename RandomContiner>
void tailEraseInRandomAccessContiner(RandomContiner* continer, typename RandomContiner::size_type index)
{
    continer->at(index) = continer->back();
    continer->pop_back();
}

}};

#endif
