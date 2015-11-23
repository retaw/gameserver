#ifndef PROTOCOL_PROTOBUF_PUBLIC_CODE_CLIENT_HPP
#define PROTOCOL_PROTOBUF_PUBLIC_CODE_CLIENT_HPP

#include <stdint.h>
#include "client.pb.h"

namespace PublicProto
{
    extern const uint32_t codeAccountAndTokenToServer;
    extern const AccountAndTokenToServer tempAccountAndTokenToServer;
    extern const uint32_t codeS_ClientLoginReady;
    extern const S_ClientLoginReady tempS_ClientLoginReady;
    extern const uint32_t codeC_SetUpName;
    extern const C_SetUpName tempC_SetUpName;
    extern const uint32_t codeC_MoveTo;
    extern const C_MoveTo tempC_MoveTo;
    extern const uint32_t codeS_ItemRoleInfo;
    extern const S_ItemRoleInfo tempS_ItemRoleInfo;
    extern const uint32_t codeS_ItemFoodInfo;
    extern const S_ItemFoodInfo tempS_ItemFoodInfo;
    extern const uint32_t codeS_ItemThronInfo;
    extern const S_ItemThronInfo tempS_ItemThronInfo;
    extern const uint32_t codeS_ItemBulletInfo;
    extern const S_ItemBulletInfo tempS_ItemBulletInfo;
    extern const uint32_t codeS_ItemCollapsar;
    extern const S_ItemCollapsar tempS_ItemCollapsar;
    extern const uint32_t codeS_SyncSceneInfo;
    extern const S_SyncSceneInfo tempS_SyncSceneInfo;
    extern const uint32_t codeSceneItemAttr;
    extern const SceneItemAttr tempSceneItemAttr;
    extern const uint32_t codeHighSpeedSceneItemAttr;
    extern const HighSpeedSceneItemAttr tempHighSpeedSceneItemAttr;
    extern const uint32_t codeDeleteItem;
    extern const DeleteItem tempDeleteItem;
    extern const uint32_t codeS_SyncLogicCenter;
    extern const S_SyncLogicCenter tempS_SyncLogicCenter;
    extern const uint32_t codeS_GameOver;
    extern const S_GameOver tempS_GameOver;
    extern const uint32_t codeC_Fission;
    extern const C_Fission tempC_Fission;
    extern const uint32_t codeC_Expal;
    extern const C_Expal tempC_Expal;
    extern const uint32_t codeS_UserLogout;
    extern const S_UserLogout tempS_UserLogout;
    extern const uint32_t codeS_SceneItemAttrAfterImpactToCollapsar;
    extern const S_SceneItemAttrAfterImpactToCollapsar tempS_SceneItemAttrAfterImpactToCollapsar;
    extern const uint32_t codeS_UpdatePoint;
    extern const S_UpdatePoint tempS_UpdatePoint;
}

#endif