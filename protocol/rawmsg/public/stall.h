#ifndef PROTOCOL_RAWMSG_PUBLIC_STALL_HPP
#define PROTOCOL_RAWMSG_PUBLIC_STALL_HPP

#include "water/common/roledef.h"

#pragma pack(1)

namespace PublicRaw{

//摆摊物品数据结构      
struct StallObj         
{   
    uint8_t index = 0;  //摊位物品索引(从0开始)
    uint16_t cell = 0;
    uint32_t id = 0;
    uint16_t num = 0;       
    MoneyType moneyType = MoneyType::none;
    uint64_t price = 0;     
};  

//摆摊销售日志记录数据结构
struct StallSellLog     
{                       
    std::string buyer;  
    uint32_t id;        
    uint16_t num;       
    MoneyType moneyType;
    uint64_t money;     
    uint32_t selltime;  
};                      

//c -> s
//请求出售日志记录
struct ReqStallSellLog
{
};

//s -> c
//发送出售日志记录
struct RetStallSellLog
{
    ArraySize size = 0;
    struct StallLog     
    {                       
        char buyer[MAX_NAME_SZIE+1];  
        uint32_t id;        
        uint16_t num;       
        MoneyType moneyType;
        uint64_t money;     
        uint32_t selltime;  
    } log[0];                      
};

//s -> c
//刷新摊位出售物品
struct RefreshStallObj
{
    char stallName[64]; //摊位名称
    ArraySize size = 0;
    StallObj obj[0];
};

//c -> s
//请求摆摊
struct ReqOpenStall
{
    char stallName[64];
    ArraySize size = 0;
    struct StallCell
    {
        uint16_t cell;      //物品所在的背包格子
        MoneyType moneyType;//出售的价格货币类型
        uint64_t price;     //价格
    } info[0];
};

//c -> s
//请求收摊
struct ReqCloseStall
{
};

//c -> s
//请求购买卖主摊位上的某个物品
struct ReqBuyStallObj
{
    RoleId seller;
    uint8_t index;  //索引(服务器下发的数据)
};

//c -> s
//请求查看他人的摊位
struct ReqWatchOthersStall
{
    RoleId targetRoleId;
};

//s -> c
//返回观察结果
struct RetWatchStallObj
{
    char stallName[64]; //摊位名称
    ArraySize size = 0;
    StallObj obj[0];
};

}

#pragma pack()

#endif
