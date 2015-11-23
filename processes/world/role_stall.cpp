#include "role_stall.h"
#include "role_manager.h"
#include "massive_config.h"
#include "scene.h"

#include "water/componet/serialize.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/stall_log.h"
#include "protocol/rawmsg/private/stall_log.codedef.private.h"

#include "protocol/rawmsg/public/stall.codedef.public.h"

namespace world{


RoleStall::RoleStall(Role& me)
:m_owner(me)
,m_loadLogFlag(false)
,m_stallPos(0,0)
{
}


void RoleStall::openStall(const PublicRaw::ReqOpenStall* rev)
{
    m_stallObjs.clear();
    if(m_owner.sceneId() != Massive::me().m_stallCfg.mapId
       || !m_owner.inArea(AreaType::security))
    {
        m_owner.sendSysChat("主城安全区域才能摆摊");
        return;
    }

    if(m_owner.level() < Massive::me().m_stallCfg.stallLevel)
    {
        m_owner.sendSysChat("60级才能摆摊");
        return;
    }

    Package::Ptr packagePtr = m_owner.m_packageSet.getPackageByPackageType(PackageType::role);
    if(nullptr == packagePtr)
        return;
    for(ArraySize i = 0; i < rev->size; ++i)
    {
        if(nullptr == packagePtr->getObjByCell(rev->info[i].cell))
        {
            LOG_DEBUG("摆摊, role:{} 请求摆摊发现背包中没有该道具, cell:{}", m_owner.name(), rev->info[i].cell);
            return;
        }
        if(Bind::yes == packagePtr->getBindByCell(rev->info[i].cell))
        {
            LOG_DEBUG("摆摊, role:{} 有绑定物品, cell:{}", m_owner.name(), rev->info[i].cell);
            return;
        }

        PublicRaw::StallObj stallObj;
        stallObj.cell = rev->info[i].cell;
        stallObj.id = packagePtr->getTplIdByCell(rev->info[i].cell);
        stallObj.num = packagePtr->getObjNumByCell(rev->info[i].cell);
        stallObj.moneyType = rev->info[i].moneyType;
        stallObj.price = rev->info[i].price;
        stallObj.index = i;

        m_stallObjs.push_back(stallObj);
    }

    if(m_stallObjs.empty())
    {
        LOG_DEBUG("摆摊, 物品列表为空, role:{}", m_owner.name());
        return;
    }

    if(!calcStallPos())
    {
        m_owner.sendSysChat("该位置不能摆摊");
        return;
    }

    for(const auto& it : m_stallObjs)
        packagePtr->fixCell(it.cell);
    m_stallName = rev->stallName;
    m_owner.setStall();
    refreshRoleStall();

    reqDBStallLog();
    m_owner.m_heroManager.requestRecallHero();
}

void RoleStall::closeStall()
{
    if(!m_owner.clearStall())
        return;
    m_stallName.clear();
    m_stallObjs.clear();
    m_owner.m_packageSet.execPackageCell(PackageType::role, 
                                         [](CellInfo& cellInfo) { cellInfo.fixed = 0; return true; });
}

void RoleStall::buyStallObj(RoleId sellerId, uint8_t index)
{
    Role::Ptr seller = RoleManager::me().getById(sellerId);
    if(nullptr == seller || !seller->isStall())
    {
        m_owner.sendSysChat("玩家已收摊");
        return;
    }

    seller->m_roleStall.sellObj(index, m_owner);
}

void RoleStall::watchOthersStall(RoleId targetId)
{
    Role::Ptr target = RoleManager::me().getById(targetId);
    if(nullptr == target)
        return;

    if(!target->isStall())
        return;

    target->m_roleStall.retWatchStallObj(m_owner);
}

void RoleStall::loadLog(const std::string& log)
{
    if(m_loadLogFlag)
        return;
    m_loadLogFlag = true;
    m_stallSellLogs.clear();
    Deserialize<std::string> dss(&log);
    uint32_t size = 0;
    dss >> size;
    for(uint32_t i = 0; i < size; ++i)
    {
        PublicRaw::StallSellLog log;
        dss >> log.buyer;
        dss >> log.id;
        dss >> log.num;
        dss >> log.moneyType;
        dss >> log.money;
        dss >> log.selltime;

        m_stallSellLogs.push_back(log);
    }
}

void RoleStall::saveLog()
{
    if(m_stallSellLogs.size() > 36)
        m_stallSellLogs.pop_back();
    Serialize<std::string> iss;
    iss.reset();
    iss << (uint32_t)m_stallSellLogs.size();
    for(const auto& it : m_stallSellLogs)
    {
        iss << it.buyer;
        iss << it.id;
        iss << it.num;
        iss << it.moneyType;
        iss << it.money;
        iss << it.selltime;
    }

    std::vector<uint8_t> buf;
    buf.reserve(2048);
    buf.resize(sizeof(PrivateRaw::SaveStallLog) + iss.tellp());
    auto msg = reinterpret_cast<PrivateRaw::SaveStallLog*>(buf.data());
    msg->roleId = m_owner.id();
    msg->size = iss.tellp();
    memcpy(msg->logs, iss.buffer()->data(), iss.tellp());

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(SaveStallLog), buf.data(), buf.size());
}

void RoleStall::refreshRoleStall()
{
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PublicRaw::RefreshStallObj));
    auto msg = reinterpret_cast<PublicRaw::RefreshStallObj*>(buf.data());
    m_stallName.copy(msg->stallName, sizeof(msg->stallName)-1);
    for(const auto& it : m_stallObjs)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::StallObj));
        auto msg = reinterpret_cast<PublicRaw::RefreshStallObj*>(buf.data());
        msg->obj[msg->size++] = it;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshStallObj), buf.data(), buf.size());
}

void RoleStall::retWatchStallObj(Role& watcher)
{
    std::vector<uint8_t> buf;
    buf.reserve(256);
    buf.resize(sizeof(PublicRaw::RetWatchStallObj));
    auto msg = reinterpret_cast<PublicRaw::RetWatchStallObj*>(buf.data());
    m_stallName.copy(msg->stallName, sizeof(msg->stallName)-1);
    for(const auto& it : m_stallObjs)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::StallObj));
        auto msg = reinterpret_cast<PublicRaw::RetWatchStallObj*>(buf.data());
        msg->obj[msg->size++] = it;
    }

    watcher.sendToMe(RAWMSG_CODE_PUBLIC(RetWatchStallObj), buf.data(), buf.size());
}

void RoleStall::retStallLog()
{
    if(!m_loadLogFlag)
    {
        reqDBStallLog();
        return;
    }

    std::vector<uint8_t> buf;
    buf.reserve(2048);
    buf.resize(sizeof(PublicRaw::RetStallSellLog));
    for(const auto& it : m_stallSellLogs)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetStallSellLog::StallLog));
        auto msg = reinterpret_cast<PublicRaw::RetStallSellLog*>(buf.data());
        it.buyer.copy(msg->log[msg->size].buyer, MAX_NAME_SZIE);
        msg->log[msg->size].id = it.id;
        msg->log[msg->size].num = it.num;
        msg->log[msg->size].moneyType = it.moneyType;
        msg->log[msg->size].money = it.money;
        msg->log[msg->size].selltime = it.selltime;
        ++msg->size;
    }

    m_owner.sendToMe(RAWMSG_CODE_PUBLIC(RetStallSellLog), buf.data(), buf.size());
}

bool RoleStall::sellObj(uint8_t index, Role& buyer)
{
    if(!m_loadLogFlag)
    {
        LOG_DEBUG("摆摊, role:{} 还没加载出售日志, 不能出售物品", m_owner.name());
        return false;
    }
    auto it = m_stallObjs.begin();
    for(; it != m_stallObjs.end(); ++it)
    {
        if(index == it->index)
            break;
    }
    if(it == m_stallObjs.end())
    {
        buyer.sendSysChat("物品已被买走");
        retWatchStallObj(buyer);
        return false;
    }

    const PublicRaw::StallObj stallObj = *it;
    if(nullptr == m_owner.getObjByCell(stallObj.cell, PackageType::role))
    {
        LOG_DEBUG("摆摊, 数据出现异常, 请求购买时已经没有道具, cell:{}", stallObj.cell);
        return false;
    }

    if(!buyer.checkPutObj(stallObj.id, stallObj.num, Bind::no, PackageType::role))
    {
        buyer.sendSysChat("背包空间不足, 不能购买");
        return false;
    }

    const uint64_t totalMoney = buyer.getMoney(stallObj.moneyType);
    if(totalMoney < stallObj.price)
    {
        buyer.sendSysChat("{}不足, 不能购买", buyer.getMoneyName(stallObj.moneyType));
        return false;
    }

    buyer.reduceMoney(stallObj.moneyType, stallObj.price, "摊位购买消耗");
    buyer.putObj(stallObj.id, stallObj.num, Bind::no, PackageType::role);

    //增加一条出售日志
    PublicRaw::StallSellLog log;
    log.buyer = buyer.name();
    log.id = stallObj.id;
    log.num = stallObj.num;
    log.moneyType = stallObj.moneyType;
    log.money = stallObj.price;
    log.selltime = toUnixTime(Clock::now());
    m_stallSellLogs.push_front(log);

    m_owner.m_packageSet.eraseFixedCellObj(stallObj.cell, PackageType::role, "摆摊出售删除");
    m_stallObjs.erase(it);
    //扣税
    const uint64_t tax = stallObj.price <= 10 ? 1 : stallObj.price / 10;
    m_owner.addMoney(stallObj.moneyType, SAFE_SUB(stallObj.price, tax), "摆摊出售获得");
    refreshRoleStall();
    retWatchStallObj(buyer);

    if(m_stallObjs.empty())
        closeStall();
    else
        refreshRoleStall();
    saveLog();

    return true;
}

void RoleStall::reqDBStallLog() const
{
    if(m_loadLogFlag)
        return;

    PrivateRaw::WorldReqStallLog req;
    req.roleId = m_owner.id();

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(WorldReqStallLog), &req, sizeof(req));
    return;
}

bool RoleStall::calcStallPos()
{
    Coord2D center(0,0);
    auto s = m_owner.scene();
    if(nullptr == s)
        return false;

    center = m_owner.pos().neighbor(Direction::down);
    center = center.neighbor(Direction::right);

    bool can = true;
    auto stallPosExec = [&, this](Coord2D pos) -> bool
    {
        Grid* grid = s->getGridByGridCoord(pos);
        if(nullptr == grid)
        {
            can = false;
            return true;
        }
        if(!grid->enterable(SceneItemType::role))
        {
            can = false;
            return true;
        }
        if(!grid->isArea(AreaType::security))
        {
            can = false;
            return true;
        }

        grid->execGrid([&, this](PK::Ptr pk) {
                       if(pk && pk->sceneItemType() == SceneItemType::role){
                       Role::Ptr role = std::static_pointer_cast<Role>(pk);
                       if(role->isStall())
                       can = false;
                       }
                       }
                      );
        return !can;
    };

    s->tryExecAreaByCircle(center, 1, stallPosExec);
    if(!can)
        return false;

    m_stallPos = center;
    return true;
}

Coord2D RoleStall::stallPos() const
{
    return m_stallPos;
}

}

