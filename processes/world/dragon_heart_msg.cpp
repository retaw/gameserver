#include "dragon_heart_msg.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/dragon_heart.h"
#include "protocol/rawmsg/public/dragon_heart.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{


DragonHeartMsg& DragonHeartMsg::me()
{
    static DragonHeartMsg me;
    return me;
}

void DragonHeartMsg::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RequestDragonSkills, std::bind(&DragonHeartMsg::clientmsg_RequestDragonSkills, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestUpgradeDragonSkill, std::bind(&DragonHeartMsg::clientmsg_RequestUpgradeDragonSkill, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestAddEnerge, std::bind(&DragonHeartMsg::clientmsg_RequestAddEnerge, this, _1, _2, _3));
}


void DragonHeartMsg::clientmsg_RequestDragonSkills(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    role->m_dragonHeart.retDragonSkillList();
}

void DragonHeartMsg::clientmsg_RequestUpgradeDragonSkill(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestUpgradeDragonSkill*>(msgData);
    role->m_dragonHeart.upgradeDragonSkill(rev->id);
}

void DragonHeartMsg::clientmsg_RequestAddEnerge(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestAddEnerge*>(msgData);
    role->requestAddEnerge(rev->autoyb);
}

}
