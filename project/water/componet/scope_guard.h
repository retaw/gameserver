/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-09 16:35 +0800
 *
 * Description:  在代码块退出时自动执行一些代码的机制，可理解为java的finally block
 */

#ifndef WATER_COMPONET_SCOPE_GUARD_HPP
#define WATER_COMPONET_SCOPE_GUARD_HPP

#include <functional>

namespace water{
namespace componet{

class ScopeGuard final
{
public:
    explicit ScopeGuard(std::function<void()> onExitScope_);
    ~ScopeGuard();
    void dismiss();

private:
    std::function<void()> onExitScope;
    bool dismissed;

private: // noncopyable
    ScopeGuard(ScopeGuard const&) = delete;
    ScopeGuard& operator = (ScopeGuard const&) = delete;
};

}}

#define CODE_CON(code1, code2) code1##code2
#define MACRO_CON(code1, code2) CODE_CON(code1, code2)


#define ON_EXIT_SCOPE(callback) water::componet::ScopeGuard MACRO_CON(onExit, __LINE__)(callback)
#define ON_EXIT_SCOPE_DO(statements) water::componet::ScopeGuard MACRO_CON(onExit, __LINE__)([&]{statements;})

#endif 
