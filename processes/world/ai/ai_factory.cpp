#include "ai_factory.h"

#include "ai_action.h"
#include "ai_condition.h"

namespace world{
namespace ai{

water::componet::Reflector<std::string, AIAction> actionFactory;
#define regAction(actionName) actionFactory.reg<actionName>(#actionName);

void registerAction()
{
    regAction(IdelMove);
    regAction(MobAction);
    regAction(Tower);
}

/*************************************************************************/
/***************************** AI触发条件 ********************************/
/*************************************************************************/
water::componet::Reflector<std::string, AICondition> conditionFactory;
#define regCondition(conditionName) conditionFactory.reg<conditionName>(#conditionName);

// 注册AICondition厂
void registerCondition()
{
    regCondition(TimerFilter);
    regCondition(Probability);
}

}}

