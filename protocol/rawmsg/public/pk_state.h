#ifndef PROTOCOL_RAWMSG_PUBLIC_PK_STATE_H
#define PROTOCOL_RAWMSG_PUBLIC_PK_STATE_H

#include "water/common/roledef.h"

#pragma pack(1)


namespace PublicRaw{


//s -> c
//同步状态到9屏
struct ShowPKStatusTo9    
{                         
    PKId id         = 0;  
    uint8_t sceneItem   = 1;
    uint32_t status = 0;  
};                        

//s -> c
//取消某个pk状态
struct UnshowPKStatusTo9    
{                           
    PKId id         = 0;    
    uint8_t sceneItem   = 1;
    uint32_t status = 0;    
};                          

}

#pragma pack()

#endif
