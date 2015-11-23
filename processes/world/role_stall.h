/*
 *
 *
 * 摆摊系统
 *
 *
 */

#ifndef PROCESS_WORLD_ROLE_STALL_H
#define PROCESS_WORLD_ROLE_STALL_H

#include "water/common/roledef.h"
#include "water/componet/datetime.h"
#include "water/componet/coord.h"
#include <deque>

#include "protocol/rawmsg/public/stall.h"

namespace world{

using namespace water::componet;
class Role;
class RoleStall
{
public:
    explicit RoleStall(Role& me);
    ~RoleStall() = default;

public:
    //摆摊
    void openStall(const PublicRaw::ReqOpenStall* rev);

    //收摊
    void closeStall();

    //摊位购买物品
    void buyStallObj(RoleId sellerId, uint8_t index);

    //查看他人摊位
    void watchOthersStall(RoleId targetId);

    //加载出售日志
    void loadLog(const std::string& log);

    //保存出售日志
    void saveLog();

    //刷新自己摊位
    void refreshRoleStall();

    //返回观察他人摊位结果
    void retWatchStallObj(Role& watcher);

    //发送摆摊出售日志给玩家
    void retStallLog();

    //出售物品
    bool sellObj(uint8_t index, Role& buyer);

private:
    //向DB请求出售日志
    void reqDBStallLog() const;

    //计算摊位点
    bool calcStallPos();

    //摊位点
    Coord2D stallPos() const;


private:
    Role&                   m_owner;
    std::string             m_stallName; //摊位名称
    std::vector<PublicRaw::StallObj> m_stallObjs; //摆摊出售的物品
    std::deque<PublicRaw::StallSellLog> m_stallSellLogs; //摊位出售记录(最多36条,超出的删掉队列末尾)

    bool                    m_loadLogFlag;
    Coord2D                 m_stallPos;
};

}

#endif

