#include "buff_scene.h"

#include "role_manager.h"

#include "protocol/rawmsg/public/buff_scene.h"
#include "protocol/rawmsg/public/buff_scene.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{


using namespace std::placeholders;

BuffScene& BuffScene::me()
{
    static BuffScene me;
    return me;
}



void BuffScene::regMsgHandler()
{
    REG_RAWMSG_PUBLIC(RoleRequestSelectedBuff, std::bind(&BuffScene::clientmsg_RoleRequestSelectedBuff, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RoleRequestSelectedBuffTips, std::bind(&BuffScene::clientmsg_RoleRequestSelectedBuffTips, this, _1, _2, _3));
}

void BuffScene::clientmsg_RoleRequestSelectedBuff(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto rev = reinterpret_cast<const PublicRaw::RoleRequestSelectedBuff*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;

    role->reqSelectedBuff(rev->id, rev->sceneItem);
}

void BuffScene::clientmsg_RoleRequestSelectedBuffTips(const uint8_t* msgData, uint32_t msgSize, RoleId rid)
{
    auto rev = reinterpret_cast<const PublicRaw::RoleRequestSelectedBuffTips*>(msgData);
    auto role = RoleManager::me().getById(rid);
    if(role == nullptr)
        return;

    role->reqSelectedBuffTips(rev->id, rev->sceneItem, rev->buffId);
}


}
