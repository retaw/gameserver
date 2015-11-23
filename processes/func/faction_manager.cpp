#include "faction_manager.h"
#include "faction_table_structure.h"
#include "water/componet/logger.h"
#include "water/componet/string_kit.h"
#include "water/componet/xmlparse.h"
#include "water/componet/serialize.h"

#include "func.h"
#include "role_manager.h"
#include "channel.h"
#include "all_role_info_manager.h"

#include "protocol/rawmsg/rawmsg_manager.h"

#include "protocol/rawmsg/public/faction.codedef.public.h"

#include "protocol/rawmsg/private/faction.codedef.private.h"


namespace func{

FactionManager& FactionManager::me()
{
    static FactionManager me;
    return me;
}

bool FactionManager::loadConfig(const std::string& cfgDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgDir + "/faction.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(FactionIdNotExist, cfgFile + " parse root node failed");
    
    m_cfg.createFactionLevel = root.getChildNodeText<uint32_t>("createFactionLevel");
    m_cfg.applyLevel = root.getChildNodeText<uint32_t>("applyLevel");
//帮派创建消耗
    XmlParseNode costPropNode = root.getChild("createFactionCostProp");
    m_cfg.propId = costPropNode.getAttr<uint64_t>("id");
    m_cfg.propNum = costPropNode.getAttr<uint32_t>("num");
    XmlParseNode costMoneyNode = root.getChild("createFactionCostMoney");
    m_cfg.moneyId = (MoneyType)costMoneyNode.getAttr<uint8_t>("id");
    m_cfg.moneyNum = costMoneyNode.getAttr<uint32_t>("num");
    m_cfg.reuseFactionDuration = root.getChildNodeText<uint32_t>("reuseFactionDuration");
    
    XmlParseNode levelNode = root.getChild("level");
    uint16_t max = levelNode.getAttr<uint16_t>("max");
    //levelItem节点
    for(XmlParseNode levelItemNode = levelNode.getChild("levelItem"); levelItemNode; ++levelItemNode)
    {
        LevelItem item;
        item.memberNum = levelItemNode.getAttr<uint16_t>("memberNum");
        item.viceLeaderNum = levelItemNode.getAttr<uint16_t>("viceLeaderNum");
        item.exp = levelItemNode.getAttr<uint64_t>("exp");
        m_cfg.levelItemVec.push_back(item);
    }
    LOG_DEBUG("帮派共{}级", m_cfg.levelItemVec.size());
    if(max != m_cfg.levelItemVec.size())
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");
    return true;
}

Faction::Ptr FactionManager::getById(FactionId factionId) const
{
    auto it = m_factions.find(factionId);
    if(it == m_factions.end())
        return nullptr;
    return it->second;
}

void FactionManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(ReqFactionHall, std::bind(&FactionManager::clientmsg_ReqFactionHall, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqFactionList, std::bind(&FactionManager::clientmsg_ReqFactionList, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(CreateFaction, std::bind(&FactionManager::clientmsg_CreateFaction, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ApplyJoinFactionToS, std::bind(&FactionManager::clientmsg_ApplyJoinFactionToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ApplyList, std::bind(&FactionManager::clientmsg_ApplyList, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(DealApplyRecord, std::bind(&FactionManager::clientmsg_DealApplyRecord, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(AppointLeader, std::bind(&FactionManager::clientmsg_AppointLeader, this, _1, _2, _3));

    REG_RAWMSG_PUBLIC(InviteJoinFactionToS, std::bind(&FactionManager::clientmsg_InviteJoinFactionToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RetInviteJoinFactionToS, std::bind(&FactionManager::clientmsg_RetInviteJoinFactionToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(KickOutFromFaction, std::bind(&FactionManager::clientmsg_KickOutFromFaction, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(LeaveFaction, std::bind(&FactionManager::clientmsg_LeaveFaction, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(SaveNotice, std::bind(&FactionManager::clientmsg_SaveNotice, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(CancelApplyRecord, std::bind(&FactionManager::clientmsg_CancelApplyRecord, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqFactionLog, std::bind(&FactionManager::clientmsg_ReqFactionLog, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(FactionLevelUp, std::bind(&FactionManager::clientmsg_FactionLevelUp, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(ReqFactionMembers, std::bind(&FactionManager::clientmsg_ReqFactionMembers, this, _1, _2, _3));

    REG_RAWMSG_PRIVATE(ObjectDonate, std::bind(&FactionManager::servermsg_ObjectDonate, this, _1, _2));
    REG_RAWMSG_PRIVATE(CreateFaction, std::bind(&FactionManager::servermsg_CreateFaction, this, _1, _2));
    REG_RAWMSG_PRIVATE(SynBanggong, std::bind(&FactionManager::servermsg_SynBanggong, this, _1, _2));
}

void FactionManager::clientmsg_ReqFactionHall(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("帮派大厅, 消息错误, msgSize={}", msgSize);
        return;
    }
    LOG_DEBUG("收到派大厅请求");

    factionPanel(roleId);
}

void FactionManager::clientmsg_ReqFactionList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("帮派列表, 消息错误, msgSize={}", msgSize);
        return;
    }
    LOG_DEBUG("收到派列表请求");

    factionPanel(roleId);
}

void FactionManager::clientmsg_CreateFaction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::CreateFaction);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("创建帮派, 消息错误, msgsize={}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::CreateFaction*>(msgData);
    LOG_DEBUG("收到创建帮派消息, name={}", rev->name);
    
    canCreateFaction(rev->name, roleId);
}

void FactionManager::clientmsg_ApplyJoinFactionToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::ApplyJoinFactionToS);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("申请入帮, 消息错误, msgsize={}, msgSize={}", msgSize);
        return;
    }

    auto rev = reinterpret_cast<const PublicRaw::ApplyJoinFactionToS*>(msgData);
    std::string factionIdStr;
    factionIdStr.append(rev->factionId, rev->size);
    FactionId factionId = water::componet::fromString<FactionId>(factionIdStr);
    applyJoinFactionToS(factionId, roleId);
}

void FactionManager::clientmsg_ApplyList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("请求申请列表, 消息错误, msgsize={}", msgSize);
        return;
    }
    
    LOG_DEBUG("收到请求帮派申请列表请求");
    applyList(roleId);
}

void FactionManager::clientmsg_DealApplyRecord(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::DealApplyRecord);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("处理帮派申请结果, 消息错误, msgsize={}", msgSize);
        return;
    }
    
    auto rev = reinterpret_cast<const PublicRaw::DealApplyRecord*>(msgData);

    LOG_DEBUG("收到处理帮派申请结果, roleId={}, accept={}", rev->roleId, rev->accept);

    dealApplyRecord(roleId, rev->roleId, rev->accept);
}

void FactionManager::clientmsg_CancelApplyRecord(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::CancelApplyRecord);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("取消帮派申请, 消息错误, msgSize={}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::CancelApplyRecord*>(msgData);
    cancelApplyRecord(roleId, rev->factionId);
}

void FactionManager::clientmsg_AppointLeader(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::AppointLeader);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("任命副帮主, 消息错误, msgsize={}", msgSize);
        return;
    }

    auto rev = reinterpret_cast<const PublicRaw::AppointLeader*>(msgData);

    appointLeader(roleId, rev->roleId, rev->position);
}

void FactionManager::clientmsg_InviteJoinFactionToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::InviteJoinFactionToS);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("邀请入帮, 消息错误, msgSize={}", msgSize);
        return;
    }
    LOG_DEBUG("邀请入帮, 收到邀请消息");
    auto rev = reinterpret_cast<const PublicRaw::InviteJoinFactionToS*>(msgData);

    inviteJoinFactionToS(roleId, rev->roleId);
}

void FactionManager::clientmsg_RetInviteJoinFactionToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::RetInviteJoinFactionToS);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("邀请入帮回复, 消息错误, msgSize={}", msgSize);
        return;
    }
    auto rev =  reinterpret_cast<const PublicRaw::RetInviteJoinFactionToS*>(msgData);

    LOG_DEBUG("收到邀请入帮回复消息, accept={}, size={}", rev->accept, msgSize);
    retInviteJoinFactionToS(roleId, rev);
}

void FactionManager::clientmsg_KickOutFromFaction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::KickOutFromFaction);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("踢出帮派, 消息错误, msgSize={}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::KickOutFromFaction*>(msgData);

    LOG_DEBUG("收到踢出帮派消息, leaderId={}, roleId={}", roleId, rev->roleId);
    kickOutFromFaction(roleId, rev->roleId);
}

void FactionManager::clientmsg_LeaveFaction(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("离开帮派, 消息错误, msgSize={}", msgSize);
        return;
    }

    LOG_DEBUG("收到离开帮派消息");

    leaveFaction(roleId);
}

void FactionManager::clientmsg_SaveNotice(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::SaveNotice);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("保存公告, 消息错误");
        return;
    }

    LOG_DEBUG("收到保存公告消息");

    auto rev = reinterpret_cast<const PublicRaw::SaveNotice*>(msgData);

    saveNotice(roleId, rev);
}

void FactionManager::clientmsg_ReqFactionLog(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("帮派日志请求, 消息错误");
        return;
    }
    LOG_DEBUG("帮派日志, 收到请求");
    retFactionLog(roleId);
}

void FactionManager::clientmsg_FactionLevelUp(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("帮派升级, 消息错误");
        return;
    }
    LOG_DEBUG("帮派升级, 收到请求");
    factionLevelUp(roleId);
}

void FactionManager::clientmsg_ReqFactionMembers(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != 0)
    {
        LOG_DEBUG("帮派成员列表请求, 消息错误");
        return;
    }
    LOG_DEBUG("帮派成员列表请求, 收到请求");
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->factionId() == 0)
        return;
    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派成员列表请求, 角色的帮派不存在, roleId={}, factionId={}",
                  roleId, role->factionId());
        return;
    }
    std::vector<uint8_t*> buf;
    buf.reserve(2000);
    facPair->second->fillFactionMembers(buf);
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionMembers), buf.data(), buf.size());
}

void FactionManager::servermsg_ObjectDonate(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::ObjectDonate*>(msgData);
    auto role = RoleManager::me().getById(rev->roleId);
    if(role == nullptr)
        return;
    if(role->factionId() == 0)
    {
        role->sendSysChat("你还没有帮派, 不能捐献");
        return;
    }
    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派捐献, 角色的帮派不存在, roleId={}, factionId={}",
                  rev->roleId, role->factionId());
        return;
    }

    uint64_t exp = facPair->second->exp() + rev->exp;
    uint64_t resource = facPair->second->resource() + rev->resource;
    uint64_t banggong = m_factionInfoOfRole[rev->roleId].banggong + rev->banggong;

    //数据库插入（faction和role）......
    if(!donateToDB(exp, resource, banggong, rev->roleId, role->factionId()))
    {
        LOG_ERROR("帮派捐献, 失败, exp={}, resource={}, banggong={}, roleId={}, factionId={}",
                  rev->exp, rev->resource, rev->banggong, rev->roleId, role->factionId());
        return;
    }
    //faction内存
    facPair->second->donate(exp, resource);
    //role内存
    m_factionInfoOfRole[rev->roleId].banggong = banggong; 

    auto shouldBeLevel = factionLevelShouldBe(facPair->second->exp(), facPair->second->level());
    if(shouldBeLevel == 0)
        return;
    if(facPair->second->level() < shouldBeLevel)
    {
        LOG_DEBUG("帮派捐献, 帮派可升级, 给相关人发送升级提示");
        auto leader = RoleManager::me().getById(facPair->second->leader());
        if(leader != nullptr)
            leader->sendSysChat("您的帮派已可以升级", "");
        std::unordered_set<RoleId> viceLeaders = facPair->second->viceLeader();
        for(auto& it : viceLeaders)
        {
            auto viceLeader = RoleManager::me().getById(it);
            if(viceLeader == nullptr)
                continue;
            viceLeader->sendSysChat("您的帮派已可以升级", "");
        }
    }
}

void FactionManager::servermsg_CreateFaction(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::CreateFaction*>(msgData);
    LOG_DEBUG("收到world创建帮派消息, roleId={}, factionName={}",
              rev->roleId, rev->factionName);
    createFaction(rev->factionName, rev->roleId);
}

void FactionManager::servermsg_SynBanggong(const uint8_t* msgData, uint32_t msgSize)
{
    auto rev = reinterpret_cast<const PrivateRaw::SynBanggong*>(msgData);
    LOG_DEBUG("收到world帮贡同步消息, banggong={}", rev->banggong);

    auto info = m_factionInfoOfRole.find(rev->roleId);
    if(info == m_factionInfoOfRole.end())
        return;
    if(!changeBanggongToDB(rev->banggong, rev->roleId))
    {
        LOG_TRACE("帮贡入库失败, 帮贡={}", rev->banggong);
        return;
    }
    info->second.banggong = rev->banggong;

}

void FactionManager::loadFaction()
{
    std::vector<RowOfFaction> ret;
    //初始化时异常由主线程捕获并退出
    mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
    query << "select * from faction";
    query.storein(ret);
    m_lastId = 0;
    //反序列化得到的所有变长数据存入各自的vector中
    std::string ss;
    std::unordered_set<RoleId> viceLeaderSet;
    for(auto it = ret.begin(); it != ret.end(); it++)
    {
        //反序列化副帮主
        ss.clear();
        viceLeaderSet.clear();
        ss = it->viceLeaders;
        water::componet::Deserialize<std::string> ds(&ss);
        ds >> viceLeaderSet;
        //初始化最大的factionId
        uint32_t lastId = getIdCounter(it->factionId);
        if(m_lastId < lastId)
            m_lastId = lastId;
        auto fac = Faction::create(it->factionId,
                                   it->name,
                                   it->level,
                                   it->exp,
                                   it->resource,
                                   it->leader,
                                   it->warriorLeader,
                                   it->magicianLeader,
                                   it->taoistLeader,
                                   it->notice);
        //插入副帮主
        if(!viceLeaderSet.empty())
            fac->insertViceLeader(viceLeaderSet);
        if(fac == nullptr)
        {
            continue;//应该抛出，不过应该不会有这种情况
        }
        //缓存
        m_factions.insert({it->factionId, fac});
        m_factionNames.insert(it->name);
    }
    LOG_DEBUG("帮派加载完毕, 加载个数:{}", ret.size());
}

void FactionManager::loadRoleInFaction()
{
    std::vector<RowOfRoleInfoInFaction> ret;
    //初始化时异常由主线程捕获并退出
    mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
    query << "SELECT roleInFaction.id,factionId,name,level,job,banggong,offlnTime FROM roleInFaction LEFT JOIN roleRarelyUp ON roleInFaction.id = roleRarelyUp.id LEFT JOIN roleOfflnUp ON roleInFaction.id = roleOfflnUp.id LEFT JOIN roleOftenUp On roleInFaction.id = roleOftenUp.id";
    query.storein(ret);
    FactionInfoOfRole info;
    for(auto it = ret.begin(); it != ret.end(); it++)
    {
        //m_factionInfoOfRole,帮派每个角色的缓存信息
        info.roleId = it->id;
        info.factionId = it->factionId;
        info.name = it->name;
        info.level = it->level;
        info.job = Job(it->job);
        info.banggong = it->banggong;
        info.offlnTime = it->offlnTime;
        m_factionInfoOfRole.insert({it->id, info});
        //每个角色添加进自己的帮派
        auto facPair = m_factions.find(it->factionId);
        if(facPair == m_factions.end())
        {
            //抛异常
            EXCEPTION(FactionIdNotExist, "角色的帮派Id对应的帮会在数据库中不存在");
        }
        facPair->second->insertMember(it->id);
    }
}

FactionId FactionManager::getFactionId(RoleId roleId)
{
    auto it = m_factionInfoOfRole.find(roleId);
    if(it == m_factionInfoOfRole.end())
        return 0;
    return it->second.factionId;
}

std::string FactionManager::getFactionName(FactionId factionId)
{
    auto it = m_factions.find(factionId);
    if(it == m_factions.end())
        return "";
    return it->second->name();
}

FactionInfoOfRole FactionManager::getFactionRoleInfo(RoleId roleId)
{
    FactionInfoOfRole ret;
    auto it = m_factionInfoOfRole.find(roleId);
    if(it == m_factionInfoOfRole.end())
    {
        ret.roleId = 0;
        return ret;
    }
    ret = it->second;
    return ret;
}

void FactionManager::roleOnlineSet(Role::Ptr role)
{
    if(role == nullptr)
        return;
    //看是否有帮派
    auto infoPair = m_factionInfoOfRole.find(role->id());
    if(infoPair == m_factionInfoOfRole.end())
    {
        return;
    }
    //上线修改缓存的下线时间为0
    FactionInfoOfRole& info = infoPair->second;
    info.offlnTime = 0; //代表在线

    //修改所在帮派的在线人数
    auto facPair = m_factions.find(info.factionId);
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派内角色上线设置, 缓存错误, 角色存在帮派信息, 不存在对应帮派, roleId={}", role->id());
        return;
    }
    auto position = facPair->second->getPositionByRoleId(role->id());
    role->setFactionPositionNotSysnc(position);//设置role的职务但不同步
    role->setFactionIdNotSysnc(info.factionId);//设置factionId不需要同步了
    role->synBanggong(infoPair->second.banggong);//同步帮贡到world
    //role->sysnFactionLevel(facPair->second->level());
    //facPair->second->addOnlineNum();
}

void FactionManager::roleOfflineSet(Role::Ptr role)
{
    if(role == nullptr)
        return;
    if(role->factionId() == 0)
        return;
    auto infoPair = m_factionInfoOfRole.find(role->id());
    if(infoPair == m_factionInfoOfRole.end())
    {
        LOG_ERROR("帮派内角色下线设置, 缓存错误, 角色自己的factionId在faction缓存中不存在, roleId={}, role->factionId={}", 
                  role->id(), role->factionId());
        return;
    }

    //下线修改缓存的下线时间为当前时间
    FactionInfoOfRole& info = infoPair->second;
    info.offlnTime = time(NULL);
/*
    //修改所在帮派的在线人数
    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派内角色下线设置, 缓存错误, 角色存在帮派信息, 不存在对应帮派, roleId={}, factionId={}",
                  role->id(), role->factionId());
        return;
    }

    facPair->second->subOnlineNum();
*/
}

void FactionManager::setRoleInfLevel(RoleId roleId, uint32_t level)
{
    auto infoPair = m_factionInfoOfRole.find(roleId);
    if(infoPair == m_factionInfoOfRole.end())
        return;

    infoPair->second.level = level;
}

uint32_t FactionManager::getFactionLevel(FactionId factionId) const
{
    auto it = m_factions.find(factionId);
    if(it == m_factions.end())
        return 0;
    return it->second->level();
}

std::string FactionManager::getFactionLeaderName(FactionId factionId) const
{
    RoleId roleId = getFactionLeaderRoleId(factionId);
    auto it = m_factionInfoOfRole.find(roleId);
    if(it == m_factionInfoOfRole.end())
        return "";

    return it->second.name;
}

RoleId FactionManager::getFactionLeaderRoleId(FactionId factionId) const
{
    auto it = m_factions.find(factionId);
    if(it == m_factions.end())
        return 0;
    return it->second->leader();
}

std::unordered_set<RoleId> FactionManager::getFactionMembers(RoleId roleId) const
{
    std::unordered_set<RoleId> ret;
    ret.clear();
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return ret;

    if(role->factionId() == 0)
        return ret;

    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
        return ret;
    return facPair->second->allMembers();
}

//业务
void FactionManager::factionPanel(const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    //有帮派
    if(role->factionId() != 0)
    {
        auto facPair = m_factions.find(role->factionId());
        if(facPair == m_factions.end())
        {
            LOG_DEBUG("帮派大厅请求, 帮派不存在, factionId={}", role->factionId());
            return;
        }

        auto fac = facPair->second;

        std::vector<uint8_t*> buf;
        buf.reserve(1024);
        auto factionRoleInfo = getFactionRoleInfo(roleId);
        //第三个参数是当前帮派等级对应的配置
        if((fac->level() - 1) < 0 || (fac->level() - 1) > m_cfg.levelItemVec.size())
        {
            LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
            return;
        }
        fac->fillFactionHall(buf, m_cfg.levelItemVec[fac->level() - 1], factionRoleInfo);
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionHall), buf.data(), buf.size());
        LOG_DEBUG("帮派大厅发送给客户端, roleId={}, factionId={}", roleId, role->factionId());
    }
    else
    {
        std::vector<uint8_t*> buf;
        buf.resize(sizeof(PublicRaw::RetFactionList));
        auto msg = reinterpret_cast<PublicRaw::RetFactionList*>(buf.data());
        msg->size = m_factions.size();
        uint16_t i = 0;
        for(auto it = m_factions.begin(); it != m_factions.end(); it++)
        {
            buf.resize(buf.size() + sizeof(PublicRaw::FactionList));
            msg = reinterpret_cast<PublicRaw::RetFactionList*>(buf.data());
            msg->data[i].factionId = it->second->factionId();
            std::memset(msg->data[i].name, 0, NAME_BUFF_SZIE);
            it->second->name().copy(msg->data[i].name, NAME_BUFF_SZIE);
            msg->data[i].level = it->second->level();
            auto factionRoleInfo = getFactionRoleInfo(it->second->leader());
            std::memset(msg->data[i].leaderName, 0, NAME_BUFF_SZIE);
            factionRoleInfo.name.copy(msg->data[i].leaderName, NAME_BUFF_SZIE);
            msg->data[i].memberNum = it->second->memberSize();
            if((it->second->level() - 1) < 0 || (it->second->level() - 1) >= m_cfg.levelItemVec.size())
            {
                LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
                return;
            }
            msg->data[i].maxMemberNum = m_cfg.levelItemVec[it->second->level() - 1].memberNum;
            msg->data[i].beApplied = it->second->isAppliedByRoleId(roleId);

            i++;
        }
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionList), buf.data(), buf.size());
        LOG_DEBUG("帮派列表发送给客户端, roleId={}, size={}", roleId, msg->size);
    }

}

void FactionManager::insertMember(const FactionId factionId, RoleId roleId)
{
    if(!insertMemberToDB(factionId, roleId))
    {
        LOG_ERROR("帮派添加成员, 插入数据失败, factionId={}, roleId={}",
                  factionId, roleId);
        return;
    }
    //m_factions
    auto fac = m_factions.find(factionId);
    if(fac == m_factions.end())
    {
        LOG_ERROR("帮派添加成员, 要加入的是不存在的帮会, factionId={}, roleId={}",
                  factionId, roleId);
        return;
    }
    //m_factionInfoOfRole
    FactionInfoOfRole infoOfRole;
    infoOfRole.roleId = roleId;
    infoOfRole.factionId = factionId;
    fillRoleInfo(roleId, infoOfRole.name, infoOfRole.level, infoOfRole.job);
    infoOfRole.banggong = 0;
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        infoOfRole.offlnTime = time(NULL);
    else
        infoOfRole.offlnTime = 0;
    m_factionInfoOfRole.insert({roleId, infoOfRole});

    fac->second->insertMember(roleId);
}

//判断帮派是否可创建（func上判断），如果可创建，告诉world消耗金币和道具，worl返回成功后再真正创建
void FactionManager::canCreateFaction(const std::string name, const RoleId leader)
{
    auto role = RoleManager::me().getById(leader);
    if(role == nullptr)
        return;

    //是否已经有帮派
    if(role->factionId() != 0)
    {
        LOG_DEBUG("创建帮派, 创建者已经有帮会");
        return;
    }

    //验证级别
    if(m_cfg.createFactionLevel > role->level())
    {
        LOG_DEBUG("创建帮派, 创建者级别不合法");
        return;
    }

    //帮派名字是否存在
    if(existFactionName(name))
    {
        PublicRaw::RetCreateFaction send;
        send.ret = CreateFactionRet::existName;
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetCreateFaction), (uint8_t*)&send, sizeof(send));
        LOG_DEBUG("创建帮派, 返回给客户端, 名字重复");
        return;
    }
    //可以创建，告诉world判断消耗是否足够并消耗后返回结果
    PrivateRaw::CreateFactionCost send;
    send.roleId = leader;
    std::memset(send.factionName, 0, NAME_BUFF_SZIE);
    name.copy(send.factionName, NAME_BUFF_SZIE);
    send.propId = m_cfg.propId;
    send.propNum = m_cfg.propNum;
    send.moneyId = m_cfg.moneyId;
    send.moneyNum = m_cfg.moneyNum;
    Func::me().sendToPrivate(role->worldId(), RAWMSG_CODE_PRIVATE(CreateFactionCost), (uint8_t*)&send, sizeof(send));
}

//只创建，已经确保满足func上的创建条件
void FactionManager::createFaction(const std::string name, const RoleId leader)
{
    FactionId factionId = createFactionId();
    auto role = RoleManager::me().getById(leader);
    if(role == nullptr)
        return;

    if(!createFactionToDB(factionId, name, leader))
    {
        LOG_DEBUG("创建帮派, 插入数据库失败, factionId={}, name={}, leader={}",
                  factionId, name, leader);
        return;
    }
    //m_factionInfoOfRole
    FactionInfoOfRole infoOfRole;
    infoOfRole.roleId = leader;
    infoOfRole.factionId = factionId;
    infoOfRole.offlnTime = 0;
    infoOfRole.name = role->name();
    infoOfRole.level = role->level();
    infoOfRole.job = role->job();
    infoOfRole.banggong = 0;
    m_factionInfoOfRole.insert({leader, infoOfRole});
    //m_factions
    auto fac = Faction::create(factionId, name);
    m_factions.insert({factionId, fac});
    fac->setLeader(leader);//不会给world发送职位变动
    fac->insertMember(leader, FactionPosition::leader);//会给world发送变动
    //m_factionNames
    m_factionNames.insert(name);

    PublicRaw::RetCreateFaction send;
    send.ret = CreateFactionRet::success;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetCreateFaction), (uint8_t*)&send, sizeof(send));
    role->sendSysChat("帮派创建成功");
    std::string text = "恭喜玩家" + role->name() + "成功创建" + fac->name();
    Channel::me().sendSysNotifyToGlobal(ChannelType::screen_middle, text);
    std::string textToAll = "#F[4$" + role->name() + "$" + fac->name() + "$" + water::componet::toString<FactionId>(fac->factionId()) + "]";
    Channel::me().sendFactionMsgToGlobal(0, textToAll);
    LOG_DEBUG("创建帮派成功, 返回给创建者客户端");
}

void FactionManager::eraseMember(Faction::Ptr fac, const RoleId roleId)
{
    auto factionId = fac->factionId();
    if(!eraseMemberFromDB(roleId))
    {
        LOG_ERROR("帮派删除成员, 删除数据失败, factionId={}, roleId={}",
                  factionId, roleId);
        return;
    }
    fac->eraseMember(roleId);
    //m_factionInfoOfRole
    m_factionInfoOfRole.erase(roleId);

    //设置离开帮派的时间
    setLeaveFactionTime(roleId);
}

void FactionManager::applyJoinFactionToS(const FactionId factionId, const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    //是否已经有帮派
    if(role->factionId() != 0)
    {
        if(role->factionId() == factionId)
        {
            role->sendSysChat("你已经加入该帮派, 不要重复添加");
        }
        else
            role->sendSysChat("你已经有帮派, 不能加入其它帮派");
        return;
    }
    //是否达到申请级别
    if(role->level() < m_cfg.applyLevel)
    {
        role->sendSysChat("达到{}级的玩家才可以申请加入帮派", m_cfg.applyLevel);
        return;
    }
    //离开帮派时间是否满足
    if(!canUseFaction(roleId))
    {
        role->sendSysChat("你退出帮派后{}小时内不能再加入帮派", m_cfg.reuseFactionDuration);
        return;
    }

    auto facPair = m_factions.find(factionId);
    if(facPair == m_factions.end())
    {
        role->sendSysChat("你申请加入的帮派已经解散");
        return;
    }
    auto fac = facPair->second;
    
    //申请者申请记录是否达到10
    auto& applyRecord = m_applyRecord[roleId];
    if(applyRecord.size() > 9)
    {
        role->sendSysChat("最多可同时申请10个帮派","");
        return;
    }
/*
    //申请的帮派是否满员
    auto maxMemberNum = m_cfg.levelItemVec[fac->level() - 1].memberNum;
    if(fac->memberSize() == maxMemberNum || fac->memberSize() > maxMemberNum)
    {
        role->sendSysChat("帮派人数已满无法申请", "");
        return;
    }
*/
    //加入角色申请列表,不管之前有没有，执行后就一定有
    applyRecord.insert(factionId);
    //加入帮派申请列表,之前有的就更新
    fac->insertApplyRecord(roleId, role->name());
    //成功加入申请列表,给申请者反馈
    PublicRaw::RetApplyJoinFactionToC send;
    send.factionId = factionId;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetApplyJoinFactionToC), (uint8_t*)&send, sizeof(send));
    LOG_DEBUG("申请加入帮派, 给客户端发送刚加入申请列表的帮派id, roleId={}, factionId={}",
              roleId, factionId);
    //通知客户端申请已发送
    role->sendSysChat("帮派申请已发送");

    //给领导发送申请列表size
    PublicRaw::NewApplyJoinFaction sendToLeader;
    sendToLeader.size = fac->applySize();
    RoleId leaderId = fac->leader();
    auto viceLeaderSet = fac->viceLeader();
    for(auto& it : viceLeaderSet)
    {
        auto viceLeader = RoleManager::me().getById(it);
        if(viceLeader != nullptr)
            viceLeader->sendToMe(RAWMSG_CODE_PUBLIC(NewApplyJoinFaction), (uint8_t*)&sendToLeader, sizeof(send)); 
    }
    auto leader = RoleManager::me().getById(leaderId);
    if(leader != nullptr)
        leader->sendToMe(RAWMSG_CODE_PUBLIC(NewApplyJoinFaction), (uint8_t*)&sendToLeader, sizeof(send)); 
    LOG_DEBUG("请加入帮派, 给领导客户端发送申请列表大小, size={}", sendToLeader.size);
}

void FactionManager::applyList(const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    if(role->factionId() == 0)
        return;
    auto facPair = m_factions.find(role->factionId());
    auto fac = facPair->second;
    if(fac == nullptr)
    {
        LOG_ERROR("请求帮派申请列表, 出现了不存在的帮派id, roleId={}, factionId={}", 
                  roleId, role->factionId());
        return;
    }

    if(!fac->canDealApply(roleId))
    {
        LOG_DEBUG("请求帮派申请列表, 请求者没有权利");
        return;
    }

    std::vector<uint8_t*> buf;
    buf.reserve(512);
    fac->fillApplyRcord(buf);
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetApplyList), buf.data(), buf.size());
    LOG_DEBUG("帮派列表申请, 给客户端发送帮派列表");
}

void FactionManager::dealApplyRecord(const RoleId dealerId, const RoleId roleId, const bool accept)
{
    auto dealer = RoleManager::me().getById(dealerId);
    if(dealer == nullptr)
        return;
    if(!canUseFaction(roleId))
    {
        dealer->sendSysChat("对方{}小时内无法加入帮派", m_cfg.reuseFactionDuration);
        return;
    }
    //找到帮派
    auto facPair = m_factions.find(dealer->factionId());
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派申请处理, 处理者所属帮派不存在, roleId={}, factionId={}",
                  roleId, dealer->factionId());
        return;
    }
    //申请的帮派是否满员
    auto fac = facPair->second;
    if((fac->level() - 1) < 0 || (fac->level() - 1) >= m_cfg.levelItemVec.size())
    {
        LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
        return;
    }
    auto maxMemberNum = m_cfg.levelItemVec[fac->level() - 1].memberNum;
    if(fac->memberSize() == maxMemberNum || fac->memberSize() > maxMemberNum)
    {
        dealer->sendSysChat("帮派人数已满无法再加入成员", "");
        return;
    }

    //不管同意或者拒绝，都给处理人发送反馈,告诉处理者删除本条申请
    PublicRaw::RetDealApplyRecord send;
    send.roleId = roleId;
    dealer->sendToMe(RAWMSG_CODE_PUBLIC(RetDealApplyRecord), (uint8_t*)&send, sizeof(PublicRaw::RetDealApplyRecord));
    LOG_DEBUG("帮派申请处理, 给处理者发送反馈, roleId={}, dealerId={}",
              roleId, dealerId);

    //处理者是否有权处理
    if(!fac->canDealApply(dealerId))
    {
        LOG_DEBUG("帮派申请处理, 请求者没有权利, roleId={}, factionId={}",
                  dealerId, dealer->factionId());
        return;
    }

    //删除个人的申请记录
    auto roleRcordPair = m_applyRecord.find(roleId);
    if(roleRcordPair == m_applyRecord.end())
    {
        LOG_DEBUG("帮派申请处理, 处理的角色自己的列表中没有申请加入任何帮派, roleId={}",
                  roleId);
        return;
    }
    auto roleRcordPairValuePair = roleRcordPair->second.find(dealer->factionId());
    if(roleRcordPairValuePair == roleRcordPair->second.end())
    {
        LOG_DEBUG("帮派申请处理, 处理的角色自己的列表中没有对应帮派的申请, roleId={}, factionId={}",
                  roleId, dealer->factionId());
        return;
    }
    roleRcordPair->second.erase(dealer->factionId());
    if(roleRcordPair->second.empty())
        m_applyRecord.erase(roleId);

    //删除帮派的申请记录
    fac->eraseApplyRecord(roleId);
    //处理是否加入帮派
    auto role = RoleManager::me().getById(roleId);
    if(!accept)
    {
        LOG_DEBUG("帮派申请处理, 处理者拒绝了申请, roleId={}, factionId={}",
                  roleId, dealer->factionId());
        //告诉申请人被拒绝
        if(role != nullptr)
        {
            role->sendSysChat("{}派拒绝了你的入派申请", fac->name());
            PublicRaw::RefuseApplyInFaction rsend;
            rsend.factionId = fac->factionId();
            role->sendToMe(RAWMSG_CODE_PUBLIC(RefuseApplyInFaction), (uint8_t*)&rsend, sizeof(rsend));
        }
    }
    else
    {
        //申请者是否已经有帮派
        auto ret = m_factionInfoOfRole.find(roleId);
        if(ret != m_factionInfoOfRole.end())
        {
            dealer->sendSysChat("你批准的申请者已经拥有帮派");
            return;
        }

        insertMember(dealer->factionId(), roleId);
        //告诉申请人成功加入,并发送帮派大厅
        if(role != nullptr)
        {
            role->sendSysChat("{}派接受了你的入派申请", fac->name());
            std::vector<uint8_t*> buf;
            buf.reserve(1024);
            auto factionRoleInfo = getFactionRoleInfo(roleId);
            if((fac->level() - 1) < 0 || (fac->level() - 1) >= m_cfg.levelItemVec.size())
            {
                LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
                return;
            }
            fac->fillFactionHall(buf, m_cfg.levelItemVec[fac->level() - 1], factionRoleInfo);
            role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionHall), buf.data(), buf.size());
            LOG_DEBUG("帮派申请处理, 帮派大厅发送给客户端, roleId={}, factionId={}", roleId, role->factionId());
        }

        //给所有帮派成员发送添加成员消息
        PublicRaw::AddFactionMem send;
        send.member.roleId = roleId;
        send.member.position = FactionPosition::ordinary;
        auto roleInfo = getFactionRoleInfo(roleId);
        send.member.level = roleInfo.level;
        std::memset(send.member.name, 0, NAME_BUFF_SZIE);
        roleInfo.name.copy(send.member.name, NAME_BUFF_SZIE);
        send.member.job = roleInfo.job;
        send.member.banggong = roleInfo.banggong;
        send.member.offlnTime = roleInfo.offlnTime;
        facPair->second->sendAddMemToall(send);
        LOG_DEBUG("帮派申请处理, 处理者接受了申请, 给所有帮派成员发送添加通知, roleId={}, factionId={}", 
                 roleId, dealer->factionId());    
        //帮派聊天添加新人提示
        Channel::me().sendFactionMsgToGang(dealerId, "欢迎" + roleInfo.name + "加入帮派");

    }
}

void FactionManager::cancelApplyRecord(const RoleId roleId, const FactionId factionId)
{
    //不管是否有记录，都给客户端返回，因为如果没有记录的话，可能是已经被拒绝，所以取消只要接到消息就看做成功
    PublicRaw::RetCancelApplyRecord send;
    send.factionId = factionId;
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetCancelApplyRecord), (uint8_t*)&send, sizeof(PublicRaw::RetCancelApplyRecord));
    LOG_DEBUG("取消申请, 给客户端发送取消回复, roleId={}", roleId);
    auto roleRecordPair = m_applyRecord.find(roleId);
    if(roleRecordPair == m_applyRecord.end())
    {
        LOG_DEBUG("取消申请, 记录不在角色申请记录中, roleId={}, factionId={}",
                  roleId, factionId);
        return;
    }
    auto facPair = m_factions.find(factionId);
    if(facPair == m_factions.end())
    {
        LOG_DEBUG("取消申请, 记录不在帮派申请记录内，但是在role自己的记录内, 缓存逻辑错误, roleId={}, factionId={}",
                  roleId, factionId);
        return;
    }
    facPair->second->eraseApplyRecord(roleId);
    
    auto roleApplyPair = m_applyRecord.find(roleId);
    if(roleApplyPair != m_applyRecord.end())
    {
        roleApplyPair->second.erase(factionId);
        if(roleApplyPair->second.empty())
            m_applyRecord.erase(roleId);
    }
    role->sendSysChat("帮派申请已取消", "");

}

//所有职务变换都走这里
void FactionManager::appointLeader(const RoleId leaderId, const RoleId roleId, FactionPosition position)
{
    /******判断一些条件******/
    auto leader = RoleManager::me().getById(leaderId);
    if(leader == nullptr)
        return;
    //找到对应帮派
    auto facPair = m_factions.find(leader->factionId());
    if(facPair == m_factions.end())
    {
        LOG_DEBUG("帮派任命, 任命者的帮派不存在, factionId={}, roleId={}",
                  leader->factionId(), leaderId);
        return;
    }
    auto fac = facPair->second;
    //是否是队长,或者任命人与被任命人相同（卸任）
    if(!fac->isLeader(leaderId) && (leaderId != roleId))
        return;
    //得到原职务与被任命职务及职务名字
    std::string PositionName = getNameByPosition(position);
    auto originalPosition = fac->getPositionByRoleId(roleId);
    std::string originalPositionName = getNameByPosition(originalPosition);
    //不是帮内成员
    if(originalPosition == FactionPosition::none)
    {
        LOG_DEBUG("帮派任命, 该角色不在帮派内, roleId={}", roleId);
        return;
    }
    //对方已经是要设置的职务
    if(position == originalPosition)
    {
        leader->sendSysChat("对方已是{}", getNameByPosition(position));
        return;
    }
    if((fac->level() - 1) < 0 || (fac->level() - 1) >= m_cfg.levelItemVec.size())
    {
        LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
        return;
    }
    //要任命的职务是否已经达到上限,是帮主的话返回true
    if(!fac->canAddPosition(position, m_cfg.levelItemVec[fac->level() - 1]))
    {
        LOG_DEBUG("帮派任命, 任命职务满员, position={}, factionId={}", 
                  position, leader->factionId());
        leader->sendSysChat("要任命的职务已经满员, 需撤销后再任命");
        return;
    }
    //被任命者
    auto beAppointer = RoleManager::me().getById(roleId);

    /******任命帮主即转让帮主,较特殊,单独处理后返回******/
    if(position == FactionPosition::leader)
    {
        if(!appointPositionToDB(leader->factionId(), FactionPosition::leader, roleId))
        {
            LOG_ERROR("帮主转移, 失败, leaderId={}, roleId={}", leaderId, roleId);
            return;
        }
        leader->setFactionPosition(FactionPosition::ordinary);//清空原帮主觉色的职位
        fac->setPosition(roleId, FactionPosition::leader);//设置新帮主并设置了角色的职务
        //发送消息告诉客户端职务变动
        PublicRaw::RetAppointLeader send;
        send.roleId = leaderId;
        send.position = FactionPosition::ordinary;
        fac->sendAppointToAll(send);
        send.roleId = roleId;
        send.position = FactionPosition::leader;
        fac->sendAppointToAll(send);
        //频道提示
        leader->sendSysChat("帮主转移成功");
        if(beAppointer != nullptr)
            beAppointer->sendSysChat("你已成为帮主");

        LOG_DEBUG("帮主转移, 成功, 返回信息, leaderId={}, roleId={}", leaderId, roleId);
        return;
    }

    /******不是帮主，先取消原有职务******/
    if(originalPosition != FactionPosition::ordinary)
    {
       if(originalPosition == FactionPosition::viceLeader)
       {
           auto viceLeaderSet = fac->viceLeader();
           viceLeaderSet.erase(roleId);
           water::componet::Serialize<std::string> ss;
           ss.reset();
           ss << viceLeaderSet;
           if(!changeViceLeaderToDB(leader->factionId(), ss.buffer()))
           {
                LOG_ERROR("帮派任命, 取消原有副帮主职务失败, factionId={}, roleId={}",
                          leader->factionId(), roleId);
                return;
           }
           //fac取消职务
           fac->cancelSpecialLeader(roleId);
       }
       //首席取消
       else
       {
            if(!cancelPositionToDB(leader->factionId(), originalPosition, roleId))
            {
                LOG_ERROR("帮派任命, 取消原有首席职务失败, factionId={}, roleId={}",
                         leader->factionId(), roleId);
                return;
            }
            //fac取消职务
            fac->cancelSpecialLeader(roleId);
       }
    }

    /******任命副帮主******/
    if(position == FactionPosition::viceLeader)
    {
        //序列化
        auto viceLeaderSet = fac->viceLeader();
        viceLeaderSet.insert(roleId);
        water::componet::Serialize<std::string> ss;
        ss.reset();
        ss << viceLeaderSet;
        if(!changeViceLeaderToDB(leader->factionId(), ss.buffer()))
        {
            LOG_ERROR("帮派任命, 任命副帮主失败, factionId={}, roleId={}",
                      leader->factionId(), roleId);
            return;
        }
        fac->setPosition(roleId, position);
        LOG_DEBUG("帮派任命, 任命副帮主成功, factionId={}, roleId={}",
                  leader->factionId(), roleId);
    }

    /******任命首席******/
    if(position == FactionPosition::warriorLeader || position == FactionPosition::magicianLeader || position == FactionPosition::taoistLeader)
    {
        //如果任命的职务是首席，要根据职业确认是什么职业的首席
        auto roleInfo = getFactionRoleInfo(roleId);
        {
            if(roleInfo.job == Job::warrior)
                position = FactionPosition::warriorLeader;
            else if(roleInfo.job == Job::magician)
                position = FactionPosition::magicianLeader;
            else if(roleInfo.job == Job::taoist)
                position = FactionPosition::taoistLeader;
            else
                return;
        }
        if(!appointPositionToDB(leader->factionId(), position, roleId))
        {
            LOG_ERROR("帮派任命, 任命首席失败, factionId={}, roleId={}", 
                      leader->factionId(), roleId);
            return;
        }
        fac->setPosition(roleId, position);
        LOG_DEBUG("帮派任命, 任命首席成功, factionId={}, roleId={}",
                  leader->factionId(), roleId);
    }
    if(position == FactionPosition::ordinary)
    {
        //如果是任命普通成员也得在帮派任命一下，或者自己更新role的position
        if(beAppointer != nullptr)
            beAppointer->setFactionPosition(FactionPosition::ordinary);
    }
    

    /******统一的反馈******/
    if(position == FactionPosition::ordinary)
    {
        //取消职务
        if(leaderId != roleId)
        {
            leader->sendSysChat("成功取消{}", PositionName);
            if(beAppointer != nullptr)
                beAppointer->sendSysChat("你的{}职位被取消", originalPositionName);
        }
        //卸任职务
        else
        {
            if(beAppointer != nullptr)
                beAppointer->sendSysChat("职位已卸任,你已成为普通成员", PositionName);
        }
    }
    else//任命职位
    {
        leader->sendSysChat("成功任命{}", PositionName);
        if(beAppointer != nullptr)
            beAppointer->sendSysChat("你被任命为{}", PositionName);
    }
    //反馈给所有帮派成员，任命某某为某职位
    PublicRaw::RetAppointLeader send;
    send.roleId = roleId;
    send.position = position;
    fac->sendAppointToAll(send);
    LOG_DEBUG("帮派任命, 通知所有成员任命{}为{}", send.roleId, PositionName);
}

void FactionManager::inviteJoinFactionToS(const RoleId leaderId, const RoleId roleId)
{
    
    auto leader = RoleManager::me().getById(leaderId);
    if(leader == nullptr)
        return;

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
    {
        leader->sendSysChat("对方不在线, 不能邀请他加入本帮派", "");
        return;
    }

    //判断帮派Id
    if(leader->factionId() == 0)
        return;
    if(role->factionId() != 0)
    {
        leader->sendSysChat("{}已经有帮派, 不能对他发起邀请", role->name());
        return;
    }

    //找到帮派
    auto facPair = m_factions.find(leader->factionId());
    if(facPair == m_factions.end())
    {
        LOG_DEBUG("邀请入帮, 不存在的帮派, factionId={}", leader->factionId());
        return;
    }
    //判断是否满员
    auto fac = facPair->second;
    if((fac->level() - 1) < 0 || (fac->level() - 1) >= m_cfg.levelItemVec.size())
    {
        LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
        return;
    }
    auto maxMemberNum = m_cfg.levelItemVec[fac->level() - 1].memberNum;
    if(fac->memberSize() == maxMemberNum || fac->memberSize() > maxMemberNum)
    {
        LOG_DEBUG("邀请入帮, 帮派满员, size={}", fac->memberSize());
        leader->sendSysChat("你的帮派已经满员, 不能发起邀请");
        return;
    }

    //验证leader是否可以邀请他人
    if(!fac->canInvitejoin(leaderId))
    {
        //无权邀请，不给回馈，因为正常逻辑客户端不会发这种消息
        LOG_DEBUG("邀请入帮, 邀请者没有权利");
        return;
    }

    //服务器为每个角色添加被邀请记录，下线清空即可
    //role->addInvitedRecord(leaderId);

    //直接给被邀请人发送邀请
    PublicRaw::InviteJoinFactionToC send;
    send.factionId = leader->factionId();
    std::memset(send.factionName, 0, NAME_BUFF_SZIE);
    std::memset(send.name, 0, NAME_BUFF_SZIE);
    fac->name().copy(send.factionName, NAME_BUFF_SZIE);
    leader->name().copy(send.name, NAME_BUFF_SZIE);
    role->sendToMe(RAWMSG_CODE_PUBLIC(InviteJoinFactionToC), (uint8_t*)&send, sizeof(PublicRaw::InviteJoinFactionToC));
    LOG_DEBUG("帮派邀请, 给被邀请人发送邀请消息, factionId={}, factionName={}, name={}",
              send.factionId, send.factionName, send.name);
}

void FactionManager::retInviteJoinFactionToS(const RoleId roleId, const PublicRaw::RetInviteJoinFactionToS* rev)
{
    if(!rev->accept)
    {
        //被邀请人自己删除决绝的记录即可,服务器不维护
        LOG_DEBUG("帮派邀请, 被邀请人拒绝, roleId={}, factionId={}", roleId, rev->factionId);
        return;
    }
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    //级别是否满足
    if(role->level() < m_cfg.applyLevel)
    {
        role->sendSysChat("你需要达到{}级才可以加入帮派", m_cfg.applyLevel);
        return;
    }
    //离开帮派时间是否满足
    if(!canUseFaction(roleId))
    {
        role->sendSysChat("你退出帮派后{}小时内不能再加入帮派", m_cfg.reuseFactionDuration);
        return;
    }
    //判断是否已经存在帮派
    auto roleInfoPair = m_factionInfoOfRole.find(roleId);//此时没有
    if(roleInfoPair != m_factionInfoOfRole.end())
    {
        role->sendSysChat("帮派邀请, 被邀请人已经加入帮派");
        return;
    }
    auto facPair = m_factions.find(rev->factionId[0]);
    if(facPair == m_factions.end())
        return;
    //申请的帮派是否满员
    auto fac = facPair->second;
    if((fac->level() - 1) < 0 || (fac->level() - 1) >= m_cfg.levelItemVec.size())
    {
        LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
        return;
    }
    auto maxMemberNum = m_cfg.levelItemVec[fac->level() - 1].memberNum;
    if(fac->memberSize() == maxMemberNum || fac->memberSize() > maxMemberNum)
    {
        role->sendSysChat("帮派人数已满无法再加入成员", "");
        return;
    }

    insertMember(rev->factionId[0], roleId); 

    //给所有帮派成员发送添加成员消息
    PublicRaw::AddFactionMem send;
    send.member.roleId = roleId;
    send.member.position = FactionPosition::ordinary;
    send.member.level = role->level();


    roleInfoPair = m_factionInfoOfRole.find(roleId);//此时应该有
    if(roleInfoPair == m_factionInfoOfRole.end())
    {
        LOG_ERROR("派邀请处理, 逻辑错误, 加入帮派后帮派角色内存没有成员信息");
        return;
    }
    auto& roleInfo = roleInfoPair->second;
    std::memset(send.member.name, 0, NAME_BUFF_SZIE);
    roleInfo.name.copy(send.member.name, NAME_BUFF_SZIE);
    send.member.job = roleInfo.job;
    send.member.banggong = roleInfo.banggong;
    send.member.offlnTime = roleInfo.offlnTime;
    facPair->second->sendAddMemToall(send);
    role->sendSysChat("你成功加入{}派", facPair->second->name());
    LOG_DEBUG("帮派邀请处理, 处理者接受了邀请, 给所有帮派成员发送添加通知, roleId={}, factionId={}", 
              roleId, rev->factionId[0]);    
    //帮派聊天提示新人加入
    Channel::me().sendFactionMsgToGang(facPair->second->leader(), "欢迎" + roleInfo.name + "加入帮派");
    //给被邀请者发送帮派大厅
    std::vector<uint8_t*> buf;
    buf.reserve(1024);
    if((facPair->second->level() - 1) < 0 || (facPair->second->level() - 1) >= m_cfg.levelItemVec.size())
    {
        LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
        return;
    }
    facPair->second->fillFactionHall(buf, m_cfg.levelItemVec[facPair->second->level() - 1], roleInfo);
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionHall), buf.data(), buf.size());
    LOG_DEBUG("帮派邀请处理, 帮派大厅发送给客户端, roleId={}, factionId={}", roleId, role->factionId());

}

void FactionManager::kickOutFromFaction(const RoleId leaderId, const RoleId roleId)
{
    if(leaderId == roleId)
        return;

    auto leader = RoleManager::me().getById(leaderId);
    if(leader == nullptr)
        return;

    auto factionId = leader->factionId();
    auto facPair = m_factions.find(factionId);
    if(facPair == m_factions.end())
    {
        LOG_ERROR("踢出帮派, 角色帮派id在缓存中不存在, role->factionId={}",
                  leader->factionId());
        return;
    }
    //执行者是否有权限
    if(!facPair->second->canKickOut(leaderId))
    {
        LOG_DEBUG("踢出帮派, 执行者无权限");
        return; 
    }
    //被提出者是否是队长
    if(facPair->second->isLeader(roleId))
        return;

    //删除前先得到名字，用于返回消息用
    auto ret = m_factionInfoOfRole.find(roleId);
    std::string name = ret->second.name;

    //副帮主

    if(facPair->second->isViceLeader(roleId))
    {
        //序列成2进制
        water::componet::Serialize<std::string> ss;
        ss.reset();
        auto viceLeaderSet = facPair->second->viceLeader();
        viceLeaderSet.erase(roleId);
        std::vector<RoleId> viceLeaderVec;
        viceLeaderVec.insert(viceLeaderVec.begin(), viceLeaderSet.begin(), viceLeaderSet.end());
        ss << viceLeaderVec;
        eraseViceLeader(facPair->second, ss.buffer()->data(), roleId);
    }

    //其他领导的删除
    if(facPair->second->isOtherLeader(roleId))
        eraseEspecialMember(facPair->second, roleId);
    //如果是普通成员删除
    else
        eraseMember(facPair->second, roleId);

    //给执行者返回消息
    leader->sendSysChat("已将 {} 从本帮派踢出", name);

    PublicRaw::SubFactionMem sendToleader;
    sendToleader.roleId = roleId;
    //给所有成员发送删减成员
    facPair->second->sendSubMemToall(sendToleader);

    //给被删除的人发送消息
    auto role = RoleManager::me().getById(roleId);
    if(role != nullptr)
    {
        role->sendToMe(RAWMSG_CODE_PUBLIC(KickOutFromFaction), (uint8_t*)&sendToleader, sizeof(PublicRaw::KickOutFromFaction));
        role->sendSysChat("你被逐出帮派 {}", facPair->second->name());
    }
    LOG_DEBUG("踢出帮派, 给所有帮派成员发送删减消息, factionId={}, roleId={}",
              leader->factionId(), roleId);
}

void FactionManager::leaveFaction(const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_DEBUG("离开帮派, 内存逻辑错误, 角色对应的帮派id不存在, factionId={}", role->factionId());
        return;
    }
    auto fac = facPair->second;

    //如果是帮主
    if(fac->isLeader(roleId))
    {
        if(fac->memberSize() == 1)
        {
            LOG_DEBUG("离开帮派, 帮主离开, 解散帮派, roleId={}, factionId={}", roleId, role->factionId());
            breakFaction(fac);
            PublicRaw::SubFactionMem send;
            send.roleId = roleId;
            role->sendToMe(RAWMSG_CODE_PUBLIC(SubFactionMem), (uint8_t*)&send, sizeof(PublicRaw::SubFactionMem));
            role->sendSysChat("解散帮派成功");
            return;
        }
        else
        {
            role->sendSysChat("帮主不能离开帮派, 请将帮主转移后再离开");
            LOG_DEBUG("离开帮派, 帮主试图离开有成员的帮派, 不允许");
            return;
        }

    }
    //如果是副帮主
    if(fac->isViceLeader(roleId))
    {
        //序列成2进制
        water::componet::Serialize<std::string> ss;
        ss.reset();
        auto viceLeaderSet = fac->viceLeader();
        viceLeaderSet.erase(roleId);
        ss << viceLeaderSet;
        eraseViceLeader(fac, ss.buffer()->data(), roleId);
    }
    //如果是除了帮主和副帮主之外的领导
    if(fac->isOtherLeader(roleId))
    {
        eraseEspecialMember(fac, roleId);
    }
    else//普通成员
    {
        eraseMember(fac, roleId);
    }
    //给所有成员发送删减成员
    PublicRaw::SubFactionMem sendToleader;
    sendToleader.roleId = roleId;
    fac->sendSubMemToall(sendToleader);
}

void FactionManager::saveNotice(const RoleId roleId, const PublicRaw::SaveNotice* rev)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;

    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_DEBUG("保存帮派公告, 帮派Id不存在, roleId={}, factionId={}", 
                  roleId, role->factionId());
        return;
    }
    auto fac = facPair->second;
    if(!fac->canSaveNotice(roleId))
    {
        LOG_DEBUG("保存帮派公告, 存储者无权限, roleId={}, factionId={}",
                  roleId, role->factionId());
        return;
    }

    std::string notice = rev->notice;
    if(!saveNoticeToDB(role->factionId(), notice))
    {
        LOG_ERROR("保存帮派公告, 错误, roleId={}, factionId={}", 
                  roleId, role->factionId());
        return;
    }
    fac->saveNotice(notice);
    PublicRaw::RetSaveNotice send;
    send.success = true;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetSaveNotice), (uint8_t*)&send, sizeof(PublicRaw::RetSaveNotice));
    Channel::me().sendFactionMsgToGang(roleId, "帮派公告已修改");
}

void FactionManager::retFactionLog(const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
    {
        return;
    }
    if(role->factionId() == 0)
    {
        LOG_DEBUG("帮派日志请求, 请求者没有帮派");
        return;
    }

    std::vector<uint8_t*> buf;
    buf.reserve(20000);
    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派日志请求, 帮派不存在");
        return;
    }

    facPair->second->fillRetLog(buf);

    role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionLog), buf.data(), buf.size());
}

void FactionManager::factionLevelUp(const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
    {
        return;
    }
    if(role->factionId() == 0)
    {
        LOG_DEBUG("帮派升级请求, 请求者没有帮派");
        return;
    }
    
    auto facPair = m_factions.find(role->factionId());
    if(facPair == m_factions.end())
    {
        LOG_ERROR("帮派升级, 角色的帮派不存在, roleId={}, factionId={}",
                  roleId, role->factionId());
        return;
    }

    uint32_t shouldBeLevel = factionLevelShouldBe(facPair->second->exp(), facPair->second->level());
    if(shouldBeLevel == 0)
        return;
    if(shouldBeLevel == facPair->second->level())
    {
        //已经是最新级别
        LOG_DEBUG("帮派升级, 当前级别已经是升级后的级别, factionId={}, factionLeve={}",
                  role->factionId(), facPair->second->level());
        return;
    }
    else if(shouldBeLevel < facPair->second->level())
    {
        //不可能出现
        LOG_ERROR("帮派升级, 级别错乱, factionId={}, level={}, shouldBeLevel={}",
                  role->factionId(), facPair->second->level(), shouldBeLevel);
        return;
    }
    else
    {
        //应该升级
        if(!factionLevelUpToDB(role->factionId(), shouldBeLevel))
        {
            LOG_ERROR("帮派升级, 失败, factionId={}, level={}, shouldBeLevel={}",
                      role->factionId(), facPair->second->level(), shouldBeLevel);
            return;
        }
        LOG_DEBUG("帮派升级, 成功, factionId={}, level={}, shouldBeLevel={}",
                  role->factionId(), facPair->second->level(), shouldBeLevel);
        //内存.并发送给客户端消息
        facPair->second->setLevel(shouldBeLevel);

        std::vector<uint8_t*> buf;
        buf.reserve(1024);
        auto factionRoleInfo = getFactionRoleInfo(roleId);
        if((facPair->second->level() - 1) < 0 || (facPair->second->level() - 1) >= m_cfg.levelItemVec.size())
        {
            LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
            return;
        }
        //第三个参数是当前帮派等级对应的配置
        facPair->second->fillFactionHall(buf, m_cfg.levelItemVec[facPair->second->level() - 1], factionRoleInfo);
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetFactionHall), buf.data(), buf.size());
        LOG_DEBUG("帮派升级成功, 发送大厅发送给升级者客户端, roleId={}, factionId={}", roleId, role->factionId());
        facPair->second->sysnLevel();
    }
}

uint32_t FactionManager::factionLevelShouldBe(const uint64_t exp, uint32_t nowLevel)
{
    if(nowLevel >= m_cfg.levelItemVec.size())
    {
        //满级
        return nowLevel;
    }
    if(nowLevel < 0 || nowLevel > m_cfg.levelItemVec.size())
    {
        LOG_ERROR("帮派配置出错, levelItemVec.size={}", m_cfg.levelItemVec.size());
        return 0;
    }
    auto nextLevelExp = m_cfg.levelItemVec[nowLevel - 1].exp;//升到下一级需要达到的经验值
    if(nextLevelExp > exp)
    {
        //不可升级
        return nowLevel;
    }
    for(; nowLevel != (m_cfg.levelItemVec.size() + 1); nowLevel++)
    {
        auto nextLevelExp = m_cfg.levelItemVec[nowLevel - 1].exp;//升到下一级需要达到的经验值
        if(nextLevelExp == 0)
            return nowLevel; //代表满级
        if(nextLevelExp > exp)
        {
            //返回应该到达的级别
            break;
        }
    }
    return nowLevel;
}

void FactionManager::breakFaction(Faction::Ptr fac)
{
    if(fac == nullptr)
        return;
    if(!breakFactionFromDB(fac->leader(), fac->factionId()))
    {
        LOG_ERROR("帮派解除, 解除失败, factionId={}, leader={}",
                  fac->factionId(), fac->leader());
        return;
    }
    LOG_DEBUG("帮派解除, 解除成功, factionId={}, leader={}", fac->factionId(), fac->leader());
    auto role = RoleManager::me().getById(fac->leader());
    if(role == nullptr)
    {
        LOG_ERROR("帮派解除, 角色不存在");
        return;
    }
    //角色清除
    role->setFactionId(0);
    m_factionInfoOfRole.erase(fac->leader());
    //帮派名字清除
    m_factionNames.erase(fac->name());
    //帮派清除
    m_factions.erase(fac->factionId());

}

void FactionManager::eraseViceLeader(Faction::Ptr fac, const std::string& viceLeader, const RoleId roleId)
{
   if(fac == nullptr)
       return;
   if(!eraseViceLeaderFromDB(fac->factionId(), viceLeader, roleId))
   {
       LOG_ERROR("帮派副帮主删除, 删除失败, factionId={}, roleId={}", fac->factionId(), roleId);
       return;
   }
   LOG_DEBUG("帮派副帮主删除, 删除成功, factionId={}, roleId={}", fac->factionId(), roleId);
   fac->eraseMember(roleId);
   fac->eraseViceLeader(roleId);
   m_factionInfoOfRole.erase(roleId);
}

void FactionManager::eraseEspecialMember(Faction::Ptr fac, RoleId roleId)
{
    if(fac == nullptr)
        return;
    FactionPosition position = fac->getPositionByRoleId(roleId);
    if(!eraseEspecialMemberFromDB(fac->factionId(), position, roleId))
    {
       LOG_ERROR("帮派其他领导删除, 删除失败, factionId={}, roleId={}", fac->factionId(), roleId);
       return;
    }
    LOG_DEBUG("帮派其他领导删除, 删除成功, factionId={}, roleId={}", fac->factionId(), roleId);
    fac->eraseMember(roleId);
    m_factionInfoOfRole.erase(roleId);
}

/*复杂版
void FactionManager::breakFaction(FactionId factionId)
{
    auto fac = m_factions.find(factionId);
    if(fac == m_factions.end())
    {
        LOG_ERROR("帮派解除, 不存在的帮会, factionId={}", factionId);
        return;
    }
    auto memberIds = fac->second->getMembers();
    
    for(auto& memberId : memberIds)
    {
        if(memberId == fac->second->leader())   //先留住会长，最后再处理
            continue;

        //遍历删除所有会员,如果出现异常就返回，保证数据一致性
        if(!eraseMemberFromDB(factionId, memberId))
        {
            LOG_ERROR("帮派删除成员, 删除数据失败, factionId={}, roleId={}",
                      factionId, memberId);
            return;
        }

        auto fac = m_factions.find(factionId);  //m_factions
        if(fac == m_factions.end())
        {
            LOG_ERROR("帮派删除成员, 要删除的是不存在的帮会, factionId={}, roleId={}",
                      factionId, memberId);
            return;
        }
        fac->second->eraseMember(factionId);

        m_factionInfoOfRole.erase(memberId);    //m_factionInfoOfRole

        //处理会长，直接删除即可
        eraseMember(factionId, fac->second->leader());
    }
}
*/

bool FactionManager::existFactionName(const std::string& name)
{
    auto it = m_factionNames.find(name);
    if(it == m_factionNames.end())
        return false;
    return true;
}

std::string FactionManager::getNameByPosition(const FactionPosition position)
{
    switch(position)
    {
        case FactionPosition::viceLeader:
            return "副帮主";
        case FactionPosition::warriorLeader:
            return "战士首席";
        case FactionPosition::magicianLeader:
            return "法师首席";
        case FactionPosition::taoistLeader:
            return "道士首席";
        case FactionPosition::leader:
            return "帮主";
        case FactionPosition::ordinary:
            return "普通帮派成员";
        case FactionPosition::none:
            return "未知";
    }
    return "未知";
}

std::string FactionManager::toPositionName(const FactionPosition position)
{
    switch(position)
    {
        case FactionPosition::viceLeader:
            return "viceLeaders";
        case FactionPosition::warriorLeader:
            return "warriorLeader";
        case FactionPosition::magicianLeader:
            return "magicianLeader";
        case FactionPosition::taoistLeader:
            return "taoistLeader";
        case FactionPosition::leader:
            return "leader";
        case FactionPosition::ordinary:
            return "ordinary";
        case FactionPosition::none:
            return "";
    }
    return "";
}
//DB
bool FactionManager::insertMemberToDB(const FactionId factionId, const RoleId roleId)
{
    try
    {
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfRoleInFaction rowOfRole(roleId, factionId, 0);
        queryF.insert(rowOfRole);
        queryF.execute();
        LOG_DEBUG("帮派添加成员, 添加成功, factionId={}, roleId={}",
                  factionId, roleId);
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_DEBUG("帮派添加成员, 添加失败, factionId={}, roleId={}",
                  factionId, roleId);
        return false;
    }
}

bool FactionManager::eraseMemberFromDB(const RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "delete from roleInFaction where id = " << roleId;
        query.execute();
        LOG_DEBUG("帮派删除成员, 删除成功, roleId={}",
                  roleId);
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派删除成员, 删除失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::breakFactionFromDB(const RoleId roleId, const FactionId factionId)
{
    try
    {
        //用于删除faction和roleInFaction
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryS = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        queryF << "delete from faction where factionId = " << factionId;
        queryS << " delete from roleInFaction where id = " << roleId;
        mysqlpp::Transaction trans(
                                   *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                   mysqlpp::Transaction::serializable,
                                   mysqlpp::Transaction::session);
        queryS.execute();
        queryF.execute();
        trans.commit();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("解散帮派, 删除数据库失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::eraseViceLeaderFromDB(const FactionId factionId, const std::string& viceLeader, const RoleId roleId)
{
    try
    {
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryS = dbadaptcher::MysqlConnectionPool::me().getconn()->query();

        queryS << "update faction set viceLeaders = " << mysqlpp::quote << viceLeader
        <<" where factionId = " << factionId;
        queryF << "delete from roleInFaction where id = " << roleId;
        mysqlpp::Transaction trans(
                                   *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                   mysqlpp::Transaction::serializable,
                                   mysqlpp::Transaction::session);
        queryS.execute();
        queryF.execute();
        trans.commit();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派副帮主删除, 删除失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::eraseEspecialMemberFromDB(const FactionId factionId, const FactionPosition position, const RoleId roleId)
{
    try
    {
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryS = dbadaptcher::MysqlConnectionPool::me().getconn()->query();

        queryF << "delete from roleInFaction where id = " << roleId;
        queryS << "update faction set " << toPositionName(position)
        << " = 0 where factionId = " << factionId;
        //事务
        mysqlpp::Transaction trans(
                                   *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                   mysqlpp::Transaction::serializable,
                                   mysqlpp::Transaction::session);
        queryS.execute();
        queryF.execute();
        trans.commit();

        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派删除成员, 删除失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::donateToDB(const uint64_t exp, const uint64_t resource, const uint64_t banggong, const RoleId roleId, const FactionId factionId)
{
    try
    {
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryS = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        
        queryF << "update faction set exp = " << exp 
        << ", resource = " << resource
        << " where factionId = " << factionId;

        queryS << "update roleInFaction set banggong = " << banggong
        << " where id = " << roleId;
        
        mysqlpp::Transaction trans(
                                   *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                   mysqlpp::Transaction::serializable,
                                   mysqlpp::Transaction::session);

        queryF.execute();
        queryS.execute();
        trans.commit();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派贡献, 入库失败, DB error:{}", er.what());
        return false;
    }

}

bool FactionManager::changeBanggongToDB(const uint64_t banggong, const RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update roleInFaction set banggong = " << banggong
        << " where id = " << roleId;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮贡修改, 入库失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::createFactionToDB(const FactionId factionId, const std::string& name, const RoleId leader)
{
    try
    {
        //用于插入faction和roleInFaction
        mysqlpp::Query queryF = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        mysqlpp::Query queryS = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfRoleInFaction rowOfRole(leader, factionId, 0);
        RowOfFaction rowOfFaction(factionId, name, 1, 0, 0, leader, "", 0, 0, 0, "这个帮主很懒,什么都没留下");

        queryF.insert(rowOfRole);
        queryS.insert(rowOfFaction);

        //事务
        mysqlpp::Transaction trans(
                                   *(dbadaptcher::MysqlConnectionPool::me().getconn()),
                                   mysqlpp::Transaction::serializable,
                                   mysqlpp::Transaction::session);
        queryS.execute();
        queryF.execute();
        trans.commit();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("创建帮派, 插入数据库失败, DB error:{}",
                  er.what());
        return false;
    }
}

bool FactionManager::saveNoticeToDB(const FactionId factionId, const std::string& notice)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update faction set notice = " << mysqlpp::quote << notice
        << " where factionId = " << factionId;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("保存帮派公告, 失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::factionLevelUpToDB(const FactionId factionId, const uint32_t level)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update faction set level = " << level
        << " where factionId = " << factionId;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派升级, 失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::changeViceLeaderToDB(const FactionId factionId, const std::string* viceLeader)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update faction set viceLeaders = " << mysqlpp::quote 
        << *viceLeader << " where factionId = " << factionId;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派副帮主任命, 插入数据库失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::appointPositionToDB(const FactionId factionId, const FactionPosition position, const RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update faction set " << toPositionName(position)
        << " = " << roleId 
        << " where factionId = " << factionId;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派职务任命, 插入数据库失败, DB error:{}", er.what());
        return false;
    }
}

bool FactionManager::cancelPositionToDB(const FactionId factionId, const FactionPosition position, const RoleId roleId)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "update faction set " << toPositionName(position)
        << " = 0 "
        << " where factionId = " << factionId;
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("帮派职务取消, 插入数据库失败, DB error:{}", er.what());
        return false;
    }
}


FactionId FactionManager::createFactionId()
{
    FactionId id = ++m_lastId;
    id |= (FactionId(Func::me().platform()) << 48u);
    id |= (FactionId(Func::me().zoneId()) << 32u);
    return id;
}

uint32_t FactionManager::getIdCounter(const FactionId id)
{
    return uint32_t(id);
}

void FactionManager::fillRoleInfo(RoleId roleId, std::string& name, uint32_t& level, Job& job)
{
    auto roleInfo = RoleInfoManager::me().getRoleInfoById(roleId);
    name = roleInfo->name;
    level = roleInfo->level;
    job  = roleInfo->job;
}

bool FactionManager::canUseFaction(RoleId roleId)
{
    auto it = m_leaveFactionTime.find(roleId);
    if(it == m_leaveFactionTime.end())
        return true;

    auto now = std::time(NULL);
    if(now - it->second < m_cfg.reuseFactionDuration * 60 * 60)
    {
        return false;
    }
    m_leaveFactionTime.erase(roleId);
    return true;
}

void FactionManager::setLeaveFactionTime(RoleId roleId)
{
    m_leaveFactionTime[roleId] = std::time(NULL);
}

}
