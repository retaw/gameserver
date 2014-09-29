#ifndef WATER_BASE_SCOPE_GUARD_HPP
#define WATER_BASE_SCOPE_GUARD_HPP

#include <functional>

namespace water{
namespace componet{

class ScopeGuard
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

#define CODE_CON(code1, code2) code1##code2
#define MACRO_CON(code1, code2) CODE_CON(code1, code2)


#define ON_EXIT_SCOPE(callback) ScopeGuard MACRO_CON(onExit, __LINE__)(callback)
#define ON_EXIT_SCOPE_DO(statements) ScopeGuard MACRO_CON(onExit, __LINE__)([&]{statements;})

}}

#endif //#ifndef WATER_COMMON_SCOPE_GUARD_HPP
