
#include "scope_guard.h"


ScopeGuard::ScopeGuard(std::function<void()> onExitScope_)
: onExitScope(onExitScope_), dismissed(false)
{   
}   

ScopeGuard::~ScopeGuard()
{   
    if(!dismissed)
    {   
        onExitScope();
    }   
}   

void ScopeGuard::dismiss()
{   
    dismissed = true;
}   


/*************************test**********************/
//g++ --std=c++11 -DUNIT_TEST scope_guard.cpp
#ifdef UNIT_TEST


#include <iostream>
using namespace std;
int main()
{
    int i = 550;
    ON_EXIT_SCOPE_DO(cout << i << endl;);
    {
        ON_EXIT_SCOPE_DO(cout << i << endl;);
    }
    i = 560;
    return 0;
}

#endif
