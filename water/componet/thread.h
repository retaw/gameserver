/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-11-28 10:58 +0800
 *
 * Description:  线程对象
 */

#ifndef WATER_COMPONET_THREAD_H
#define WATER_COMPONET_THREAD_H

namespace water{
namespace componet{

template <typename ThreadFunRet, typename... ThreadFunArgs>
class Thread;

template <typename ThreadFunRet, typename... ThreadFunArgs>
class Thread<ThreadFunRet (ThreadFunArgs...)>
{
public:
    Thread();
    Thread(const std::functional<ThreadFunRet (ThreadFunArgs...)>&& fun);

private:
};

}}


#endif
