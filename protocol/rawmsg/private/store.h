#ifndef RAWMSG_PRIVATE_STORE_HPP
#define RAWMSG_PRIVATE_STORE_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PrivateRaw{

// world -> func
// 检查限量道具上限
struct RequestCheckObjRecord
{
    RoleId roleId   = 0;
    uint32_t shopId;
    uint8_t tabId;
    uint8_t type;
    uint32_t objId;
    uint16_t num;
};


// func -> world
// 返回上限检查结果
struct RetCheckObjRecord
{
    uint8_t retcode = 0;    //0:成功 1:失败
    RoleId  roleId  = 0;
    uint32_t shopId;
    uint8_t tabId;
    uint32_t objId;
    uint16_t num;
};


// world -> func
// 更新出售数量
struct AddObjSellNum
{
    RoleId roleId;
    uint32_t shopId;
    uint8_t tabId;
    uint8_t type;
    uint32_t objId;
    uint16_t num;
};

/*帮派商店*/
//func->world
//转发RequestFactionShop
struct RequestFactionShop
{
    RoleId roleId;
    uint32_t factionLevel;
};

//func->world
//转发RefreshFactionShop
struct RefreshFactionShop
{
    RoleId roleId;
    uint32_t factionLevel;
};

}

#pragma pack()

#endif
