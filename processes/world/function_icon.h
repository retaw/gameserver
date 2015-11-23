/*
 *
 * 功能模块图标状态管理
 *
 */

#ifndef PROCESS_WORLD_FUNCTION_ICON_H
#define PROCESS_WORLD_FUNCTION_ICON_H

#include "water/common/funcdef.h"
#include <memory>

namespace world{

class Role;
class FunctionIcon
{
private:
    FunctionIcon();

public:
    ~FunctionIcon() = default;

    static FunctionIcon& me();

public:
    //返回所有已有图标的状态
    void retAllFunctionIconState(std::shared_ptr<Role>);
    //刷新单个图标状态
    void refreshFunctionIconState(std::shared_ptr<Role>, FunctionItem item, IconState state);
};

}

#endif
