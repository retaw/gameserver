#include "pet_msg.h"
#include "role_manager.h"
#include "pet_manager.h"

#include "protocol/rawmsg/public/pet_scene.h"
#include "protocol/rawmsg/public/pet_scene.codedef.public.h"
#include "protocol/rawmsg/rawmsg_manager.h"

namespace world{


PetMsg& PetMsg::me()
{
    static PetMsg me;
    return me;
}

void PetMsg::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(PetRequestMoveToPos, std::bind(&PetMsg::clientmsg_PetRequestMoveToPos, this, _1, _2, _3));
}


void PetMsg::clientmsg_PetRequestMoveToPos(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

    auto rev = reinterpret_cast<const PublicRaw::PetRequestMoveToPos*>(msgData);
    PK::Ptr petpk = PK::getPkptr(rev->id, SceneItemType::pet);
    if(nullptr == petpk || petpk->isDead())
        return;

    Role::Ptr owner = getRole(petpk);
    if(nullptr == owner)
        return;
    if(owner != role)
        return;

    Pet::Ptr pet = std::static_pointer_cast<Pet>(petpk);
    pet->changePos(Coord2D(rev->posx, rev->posy), static_cast<componet::Direction>(rev->dir), rev->type);
}

void PetMsg::clientmsg_PetAIMode(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(nullptr == role)
        return;

//    auto rev = reinterpret_cast<const PublicRaw::PetAIMode*>(msgData);
}

}
