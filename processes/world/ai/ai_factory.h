/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-19 17:03 +0800
 *
 * Modified: 2015-05-19 17:03 +0800
 *
 * Description:  
 */

#ifndef WEBGAME_AI_FACTORY_HPP
#define WEBGAME_AI_FACTORY_HPP

#include "water/componet/reflector.h"

namespace world{
namespace ai{

class AIAction;
class AICondition;

extern water::componet::Reflector<std::string, AIAction>    actionFactory;
extern water::componet::Reflector<std::string, AICondition> conditionFactory;

void registerAction();
void registerCondition();

}}

#endif
