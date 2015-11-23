#include "horse_msg.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/horse.h"
#include "protocol/rawmsg/public/horse.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{


HorseMsg& HorseMsg::me()
{
    static HorseMsg me;
    return me;
}

void HorseMsg::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RequestRaiseInfo, std::bind(&HorseMsg::clientmsg_RequestRaiseInfo, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestRide, std::bind(&HorseMsg::clientmsg_RequestRide, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestRaise, std::bind(&HorseMsg::clientmsg_RequestRaise, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestActiveSkins, std::bind(&HorseMsg::clientmsg_RequestActiveSkins, this, _1, _2, _3));
}


void HorseMsg::clientmsg_RequestRaiseInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    role->m_horse.retRaiseInfo();
}

void HorseMsg::clientmsg_RequestRide(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestRide*>(msgData);
    if(1 == rev->state)
        role->m_horse.onRide();
    else
        role->m_horse.offRide();
}

void HorseMsg::clientmsg_RequestRaise(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestRaise*>(msgData);
    role->m_horse.raiseHorse(rev->objId, rev->autoyb);
}

void HorseMsg::clientmsg_RequestActiveSkins(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;
    role->m_horse.retActiveSkins();
}

void HorseMsg::clientmsg_RequestHuanHuaSkin(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::RequestHuanHuaSkin*>(msgData);
    role->m_horse.changeSkin(rev->skin);
}

}
