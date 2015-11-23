/*
 * Author: LiZhaojia
 *
 * Created: 2015-10-28 17:08 +0800
 *
 * Modified: 2015-10-28 17:08 +0800
 *
 * Description:  测试消息, debug相关的内容也放在这里
 */


#ifndef PROTOCOL_RAWMSG_PUBLIC_TEST_MSG_HPP
#define PROTOCOL_RAWMSG_PUBLIC_TEST_MSG_HPP

#include "water/componet/coord.h"

#pragma pack(1)


namespace PublicRaw{

struct HighlightMapBlock
{
    uint32_t binSize() const
    {
        return sizeof(*this) + sizeof(list[0]) * size;
    }

    ArraySize size = 0;
    struct 
    {
        Coord1D x;
        Coord1D y;
        uint32_t duration = 0; //持续时间, 0表示不限时
        uint8_t state = 1;     //0 关闭高亮; 1 高亮 
    } list[0];
};


};



#pragma pack()

#endif
