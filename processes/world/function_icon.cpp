#include "function_icon.h"
#include "role.h"
#include "world_boss.h"

#include "protocol/rawmsg/public/function.h"
#include "protocol/rawmsg/public/function.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{

FunctionIcon::FunctionIcon()
{
}

FunctionIcon& FunctionIcon::me()
{
    static FunctionIcon me;
    return me;
}

void FunctionIcon::retAllFunctionIconState(Role::Ptr role)
{
    if(nullptr == role)
        return;
    std::vector<uint8_t> buf;
    buf.reserve(64);
    buf.resize(sizeof(PublicRaw::RetAllFunctionIconState));

    //世界boss
    buf.resize(buf.size() + sizeof(PublicRaw::RetAllFunctionIconState::FunctionIconInfo));
    auto msg = reinterpret_cast<PublicRaw::RetAllFunctionIconState*>(buf.data());
    msg->data[msg->size].item = FunctionItem::world_boss;
    msg->data[msg->size].state = WorldBoss::me().iconState(role);
    ++msg->size;

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetAllFunctionIconState), buf.data(), buf.size());
}

void FunctionIcon::refreshFunctionIconState(Role::Ptr role, FunctionItem item, IconState state)
{
    if(nullptr == role)
        return;
    PublicRaw::RefreshFunctionIconState ret;
    ret.item = item;
    ret.state = state;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RefreshFunctionIconState), &ret, sizeof(ret));
}

}

