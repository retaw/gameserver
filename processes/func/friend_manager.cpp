#include "friend_manager.h"
#include "func.h"

#include "water/componet/logger.h"
#include "water/componet/random.h"
#include "water/componet/serialize.h"
#include "water/componet/string_kit.h"
#include "water/componet/xmlparse.h"

#include "protocol/rawmsg/public/friend.h"
#include "protocol/rawmsg/public/friend.codedef.public.h"

#include "protocol/rawmsg/private/friend.h"
#include "protocol/rawmsg/private/friend.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "friend_table_structure.h"
#include "role_table_structure.h"
#include "all_role_info_manager.h"

namespace func{

FriendManager& FriendManager::me()
{
    static FriendManager me;
    return me;
}

void FriendManager::loadConfig(const std::string& configDir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = configDir + "/relation.xml";
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + "parse root node failed");
    m_enemyNumLimit = root.getChildNodeText<uint16_t>("enemyNumLimit");
    m_friendNumLimit = root.getChildNodeText<uint32_t>("friendNumLimit");
    m_friendStartLevel = root.getChildNodeText<uint32_t>("friendStartLevel");
}

//for reg
void FriendManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(RequestFriendToS, std::bind(&FriendManager::clientmsg_RequestFriendToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RetRequestFriendToS, std::bind(&FriendManager::clientmsg_RetRequestFriendToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(EraseFriendToS, std::bind(&FriendManager::clientmsg_EraseFriendToS, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(InsertBlackList, std::bind(&FriendManager::clientmsg_InsertBlackList, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(EraseBlackList, std::bind(&FriendManager::clientmsg_EraseBlackList, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestAllRelationerInfo, std::bind(&FriendManager::clientmsg_RequestAllRelationerInfo, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(EraseEnemy, std::bind(&FriendManager::clientmsg_EraseEnemy, this, _1, _2, _3));
    REG_RAWMSG_PUBLIC(RequestRecommendFriend, std::bind(&FriendManager::clientmsg_RequestRecommendFriend, this, _1, _2, _3));


    REG_RAWMSG_PRIVATE(InsertEnemy, std::bind(&FriendManager::servermsg_InsertEnemy, this, _1, _2, _3));
}

void FriendManager::clientmsg_RequestFriendToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::RequestFriendToS);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("添加好友请求, 消息错误, msgSize = {}", msgSize);
        return ;
    }
    auto rev = reinterpret_cast<const PublicRaw::RequestFriendToS*>(msgData);
    sizeCount += sizeof(PublicRaw::RequestFriendToS::RequestFriend) * rev->size;
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("添加好友请求, 消息错误, msgSize = {}", msgSize);
        return ;
    }

    LOG_DEBUG("添加好友请求, size = {}, msgSize = {}", rev->size, msgSize);
    auto role = RoleManager::me().getById(roleId);
    for(uint16_t i = 0; i < rev->size; i++)
    {
        LOG_DEBUG("添加好友请求, 收到好友请求, roleId = {}, friendId = {}",
                  roleId, rev->data[i].friendId);
        Role::Ptr fdRole = RoleManager::me().getById(rev->data[i].friendId);
        if(fdRole == nullptr)
        {
            LOG_DEBUG("添加好友请求, 被添加方不在线, 不允许添加, roleId = {}, friendId = {}", 
                      roleId, rev->data[i].friendId);
            role->sendSysChat("添加的好友不在线");
            continue;
        }
        if(canAddFriend(role, fdRole))
        {
            sendRequestToFriend(role, fdRole);
            auto ret = m_RequestRecord[roleId].insert(rev->data[i].friendId);
            if(ret.second == false)
            {
                LOG_DEBUG("添加好友请求, 重复请求, roleId = {}, friendId = {}", 
                          roleId, rev->data[i].friendId);
            }
        }
    }
}


void FriendManager::clientmsg_RetRequestFriendToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::RetRequestFriendToS);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("好友添加回复, 消息错误");
        return ;
    }
    auto rev = reinterpret_cast<const PublicRaw::RetRequestFriendToS*>(msgData);
    sizeCount += sizeof(PublicRaw::RetRequestFriendToS::RetRequestFriend) * rev->size;
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("好友添加回复, 消息错误");
        return ;
    }

    LOG_DEBUG("好友添加回复, size = {}, retStatus = {}, msgSize = {}", 
              rev->size, rev->retStatus, msgSize);
    if(rev->size == 0)
        return;
    Role::Ptr fdRole = RoleManager::me().getById(roleId);
    for(uint16_t i = 0; i < rev->size; i++)
    {
        if(rev->data[i].roleId == roleId)
            continue;
        Role::Ptr role = RoleManager::me().getById(rev->data[i].roleId);
        auto it = m_RequestRecord[(rev->data[i].roleId)].find(roleId);
        if(it != m_RequestRecord[(rev->data[i].roleId)].end())
        {
            LOG_DEBUG("好友添加回复, 收到好友的回复, 将请求从管理列表删除, 并开始添, 加roleId = {}, friendId = {}",
                      rev->data[i].roleId, roleId);
            m_RequestRecord[(rev->data[i].roleId)].erase(it);
        }
        else
        {
            LOG_DEBUG("好友添加回复, 收到未请求的好友请求回复, 疑似挂, roleId ={},friendId = {}", rev->data[i].roleId, roleId);
            continue;
        }
        if(rev->retStatus == RequestFdRely::refuse)
        {
            LOG_DEBUG("好友添加回复, 被动方拒绝, roleId = {}, friendId = {}", 
                      rev->data[i].roleId, roleId);
            continue;
        }
        LOG_DEBUG("好友添加回复, 被动方同意, roleId = {}, friendId = {}",
                  rev->data[i].roleId, roleId);
        if(role->friendNum() > (m_friendNumLimit + role->level() - 1))
        {   
            fdRole->sendSysChat("{}的好友已经达到上限,无法添加为好友", role->name());
            continue; //被动方达到上限,不能再继续添加
        }  
        if(fdRole->friendNum() > (m_friendNumLimit + fdRole->level() - 1))
        {
            fdRole->sendSysChat("你的好友已经达到上限, 无法继续添加好友");
            return;
        }
        if(canAddFriend(fdRole, role))
            insertFriend(role, fdRole);//roleId是被动方,rev->data[i].roleId是主动方
    }
}

void FriendManager::clientmsg_EraseFriendToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::EraseFriendToS);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("好友删除, 消息不正确, msgSize = {}", msgSize);
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::EraseFriendToS*>(msgData);
    sizeCount += sizeof(PublicRaw::EraseFriendToS::EraseFriend) * rev->size;
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("好友删除, 消息不正确, msgSize = {}", msgSize);
        return;
    }
    LOG_DEBUG("好友删除, size = {}", rev->size);
    for(uint16_t i = 0; i < rev->size; i++)
    {
        LOG_DEBUG("好友删除, 收到好友删除请求, roleId = {}, friendId = {}",
                  roleId, rev->data[i].roleId);
        eraseFriend(roleId, rev->data[i].roleId);//理论上一定是好友，不用判断, 并且不管在不在都是删除成功
    }
}

void FriendManager::clientmsg_InsertBlackList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::InsertBlackList);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("添加黑名单, 消息错误");
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::InsertBlackList*>(msgData);
    LOG_DEBUG("添加黑名单, roleId = {}, blackId = {}",
              roleId, rev->roleId);
    if(rev->roleId == roleId)
        return;
    if(isExistFriend(roleId, rev->roleId))
        eraseFriend(roleId, rev->roleId);
    insertBlackList(roleId, rev->roleId);
}

void FriendManager::clientmsg_EraseBlackList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    uint16_t sizeCount = sizeof(PublicRaw::EraseBlackList);
    if(msgSize < sizeCount)
    {
        LOG_DEBUG("删除黑名单, 消息错误");
        return;
    }
    auto rev = reinterpret_cast<const PublicRaw::EraseBlackList*>(msgData);
    LOG_DEBUG("删除黑名单, roleId = {}, blackId= {}", 
              roleId, rev->roleId);
    eraseBlackList(roleId, rev->roleId);
}

void FriendManager::clientmsg_RequestAllRelationerInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    //auto rev = reinterpret_cast<const PublicRaw::RequestAllRelationerInfo*>(msgData);
    if(msgSize != 0)
    {
        LOG_DEBUG("好友面板展开, 消息错误");
        return;
    }
    LOG_DEBUG("好友面板展开, 请求所有有关角色信息, roleId={}, msgSize = {}", roleId, msgSize);
    sendAllRelationerInfoToMe(roleId);
}


void FriendManager::clientmsg_EraseEnemy(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    auto rev = reinterpret_cast<const PublicRaw::EraseEnemy*>(msgData);
    if(msgSize != sizeof(PublicRaw::EraseEnemy))
    {
        LOG_DEBUG("删除仇人, 消息错误");
        return;
    }
    LOG_DEBUG("删除仇人, 收到删除请求, roleId = {}, enemyId = {}",
              roleId, rev->enemyId);
    eraseEnemy(roleId, rev->enemyId);
}

void FriendManager::clientmsg_RequestRecommendFriend(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    LOG_DEBUG("好友推荐, 收到推荐请求, roleId = {}", roleId);
    std::vector<uint8_t> buf;
    buf.reserve(560);
    buf.resize(sizeof(PublicRaw::RetRecommendFriend));
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    fillRecommendFriend(role->level(), buf, roleId);
    auto msg = reinterpret_cast<PublicRaw::RetRecommendFriend*>(buf.data());
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetRecommendFriend), buf.data(), buf.size());
    LOG_DEBUG("好友推荐, 返回好友推荐列表给客户端, roleId = {}, friendSize = {}",
              roleId, msg->friendSize);
}

void FriendManager::servermsg_InsertEnemy(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::InsertEnemy*>(msgData);
    LOG_DEBUG("添加仇人, 收到world传来的添加仇人消息, roleId = {},enemyId = {}",
              rev->roleId, rev->enemyId);
    insertEnemy(rev->roleId, rev->enemyId, rev->beKilledType);
}

//private
void FriendManager::sendRequestToFriend(Role::Ptr role, Role::Ptr fdRole)
{
    if(role == nullptr || fdRole == nullptr)
        return ;
    const uint32_t bufSize = sizeof(PublicRaw::RequestFriendToC);
    PublicRaw::RequestFriendToC buf;
    buf.roleId = role->id();
    buf.level = role->level();
    std::memset(buf.name, 0, NAME_BUFF_SZIE);
    role->name().copy(buf.name, NAME_BUFF_SZIE);
    LOG_DEBUG("向客户端发送好友请求信息");
    role->sendSysChat("好友申请已发送");
    fdRole->sendToMe(RAWMSG_CODE_PUBLIC(RequestFriendToC), (uint8_t*)&buf, bufSize);
}

void FriendManager::fillRecommendFriend(uint32_t level, std::vector<uint8_t>& buf, RoleId roleId)
{
    std::vector<Role::Ptr> roleVec = RoleManager::me().getSimilarMylevelAndUpOnelevel(level, 0, roleId);   //第二个参数是满足高于这个等级
    uint16_t count = 0;
    auto msg = reinterpret_cast<PublicRaw::RetRecommendFriend*>(buf.data());
    if(roleVec.size() < (RcommonFdNum + 1))
    {
        msg->friendSize = roleVec.size();
        for(auto& it : roleVec)
        {
            buf.resize(buf.size() + sizeof(PublicRaw::RetRecommendFriend::Friend));
            auto msg = reinterpret_cast<PublicRaw::RetRecommendFriend*>(buf.data());
            msg->data[count].friendId = it->id();
            msg->data[count].level = it->level();
            std::memset(msg->data[count].name, 0, NAME_BUFF_SZIE);
            it->name().copy(msg->data[count].name, NAME_BUFF_SZIE);
            LOG_DEBUG("好友推荐, 第{}个推荐好友:roleId= {}, name = {}, level = {}",
                      count, msg->data[count].friendId, msg->data[count].name, msg->data[count].level);
            count++;
        }
    }
    else
    {
        msg->friendSize = RcommonFdNum;
        water::componet::Random<uint16_t> rand(0, roleVec.size());
        std::unordered_set<uint16_t> numSet;
        while(numSet.size() < RcommonFdNum)
        {
            numSet.insert(rand.get());
        }
        for(auto& num : numSet)
        {
            buf.resize(buf.size() + sizeof(PublicRaw::RetRecommendFriend::Friend));
            auto msg = reinterpret_cast<PublicRaw::RetRecommendFriend*>(buf.data());
            msg->data[count].friendId = roleVec[num]->id();
            msg->data[count].level = roleVec[num]->level();
            std::memset(msg->data[count].name, 0, NAME_BUFF_SZIE);
            roleVec[num]->name().copy(msg->data[count].name, NAME_BUFF_SZIE);
            LOG_DEBUG("好友推荐, 第{}个推荐好友:roleId= {}, name = {}, level = {}",
                      count, msg->data[count].friendId, msg->data[count].name, msg->data[count].level);
            count++;
        }
    }
    
}

void FriendManager::getSmallAndBigRoleId(uint64_t& small, uint64_t& big, RoleId roleId1, RoleId roleId2)
{
    small = roleId1;
    big = roleId2;
    if(roleId1 > roleId2)
    {   
        small = roleId2;
        big = roleId1;
    }   
}

//业务
bool FriendManager::canAddFriend(Role::Ptr role, Role::Ptr fdRole)
{
    if(role == nullptr)
        return false;
    if(fdRole == nullptr)
    {
        role->sendSysChat("对方已经下线, 无法添加好友");
        return false;
    }
    
    if(role->id() == fdRole->id())
        return false;

    if(isExistBlackList(role->id(), fdRole->id()))
    {   
        eraseBlackList(role->id(), fdRole->id());//在自己的黑名单，删除黑名单
    }
    if(isExistBlackList(fdRole->id(), role->id()))//在对方黑名单中
    {   
        role->sendSysChat("你在对方的黑名单中, 无法添加好友");
        return false;
    }
    if(isExistFriend(role->id(), fdRole->id()))
    {
        role->sendSysChat("{}已经是你的好友,不要重复添加", fdRole->name());
        return false;
    }
    if(role->friendNum() >= (m_friendNumLimit + role->level() - 1))
    {   
        role->sendSysChat("你的好友已经达到上限,升级可以增加好友上限");
        return false;
    }  
    return true;
}

void FriendManager::insertFriend(Role::Ptr role, Role::Ptr fdRole)
{
    if(role == nullptr || fdRole == nullptr)
        return;

    //先插入缓存，如果数据库不成功再删除，不先入库
    insertFriendToMap(role, fdRole); 
    std::unordered_set<RoleId> friendSet = role->getFriendSet();
    std::unordered_set<RoleId> fdFriendSet = role->getFriendSet();
    water::componet::Serialize<std::string> ss;
    water::componet::Serialize<std::string> fdSs;
    ss.reset();
    fdSs.reset();
    ss << friendSet;
    ss << fdFriendSet;
    if(!updateFriendToDB(role->id(), ss.buffer(), fdRole->id(), fdSs.buffer()))
    {
        eraseFriendFromMap(role, fdRole);
        return;
    }

    //通知客户端添加好友
    {
        //主动端
        PublicRaw::RetRequestFriendToC sendToC;
        sendToC.friendId = fdRole->id();
        fillRoleInfo(fdRole->id(), sendToC.name, sendToC.level);
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetRequestFriendToC), (uint8_t*)&sendToC, sizeof(PublicRaw::RetRequestFriendToC));
        role->sendSysChat("添加好友{}成功", fdRole->name());
        LOG_DEBUG("向主动客户端发送好友添加通知, roleId = {}, friendId = {}",
                  role->id(), fdRole->id());
    }
    {
        //被动端
        PublicRaw::RetRequestFriendToC sendToC;
        sendToC.friendId = role->id();
        fillRoleInfo(role->id(), sendToC.name, sendToC.level);
        fdRole->sendToMe(RAWMSG_CODE_PUBLIC(RetRequestFriendToC), (uint8_t*)&sendToC, sizeof(PublicRaw::RetRequestFriendToC));
        LOG_DEBUG("向被动客户端发送好友添加通知, roleId = {}, friendId = {}",
                  fdRole->id(), role->id());
    }
}

void FriendManager::eraseFriend(const RoleId roleId, const RoleId friendId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    //可能为nullptr
    auto fdRole = RoleManager::me().getById(friendId);

    //删除缓存
    eraseFriendFromMap(role, fdRole);
    //只能确保得到主动者的缓存
    std::unordered_set<RoleId> friendSet = role->getFriendSet();
    water::componet::Serialize<std::string> ss;
    ss.reset();
    ss << friendSet;
    if(!eraseFriendFromDB(roleId, ss.buffer(), friendId))
    {
        insertFriendToMap(role, fdRole);
        return;
    }

    //返回删除好友信息
    PublicRaw::RetEraseFriendToC sendToC;
    sendToC.roleId = friendId;
    if(role != nullptr)
    {
        LOG_DEBUG("向主动客户端发送好友删除通知, roleId = {}, friendId = {}", 
                  roleId, friendId);
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetEraseFriendToC), (uint8_t*)&sendToC, sizeof(PublicRaw::RetEraseFriendToC));
        char fdName[NAME_BUFF_SZIE];
        uint32_t fdLevel;
        fillRoleInfo(friendId, fdName, fdLevel);
        role->sendSysChat("已将{}从好友列表移除", fdName);
    }

    //似乎多余，客户端暂时需要发送
    sendToC.roleId = roleId;
    if(fdRole != nullptr)
    {
        LOG_DEBUG("向被动客户端发送好友删除通知, roleId = {}, friendId = {}", 
                  friendId, roleId);
        fdRole->sendToMe(RAWMSG_CODE_PUBLIC(RetEraseFriendToC), (uint8_t*)&sendToC, sizeof(PublicRaw::RetEraseFriendToC));
    }
}

void FriendManager::sendAllRelationerInfoToMe(const RoleId roleId)
{
   auto role = RoleManager::me().getById(roleId); 
   if(role == nullptr)
       return;
   std::vector<uint8_t> buf;
   buf.reserve(1024);
   buf.resize(sizeof(PublicRaw::RetAllRelationerInfo));
   auto msg = reinterpret_cast<PublicRaw::RetAllRelationerInfo*>(buf.data());
   msg->friendSize = role->friendNum();
   msg->blackSize = m_blackListMap[roleId].size();
   msg->enemySize = role->enemyNum();
   uint16_t i =0;
   for(auto& relationer : role->getFriendSet())
   {
       buf.resize(buf.size() + sizeof(PublicRaw::RetAllRelationerInfo::RelationerInfo));
       msg = reinterpret_cast<PublicRaw::RetAllRelationerInfo*>(buf.data());
       msg->data[i].roleId = relationer;
       fillRoleInfo(relationer, msg->data[i].name, msg->data[i].level);
       if(RoleManager::me().getById(relationer))
           msg->data[i].isOnln = IsOnln::yes;
       LOG_DEBUG("好友面板数据请求, 好友变长数据第{}个:roleId= {}, name = {}, level = {}",
                  i, msg->data[i].roleId, msg->data[i].name, msg->data[i].level);
       i++;
   }
   auto blackList = m_blackListMap[roleId];
   for(auto& relationer : blackList)
   {
       buf.resize(buf.size() + sizeof(PublicRaw::RetAllRelationerInfo::RelationerInfo));
       msg = reinterpret_cast<PublicRaw::RetAllRelationerInfo*>(buf.data());
       msg->data[i].roleId = relationer;
       fillRoleInfo(relationer, msg->data[i].name, msg->data[i].level);
       if(RoleManager::me().getById(relationer))
           msg->data[i].isOnln = IsOnln::yes;
       LOG_DEBUG("好友面板数据请求, 黑名单变长数据第{}个:roleId= {}, name = {}, level = {}",
                  i, msg->data[i].roleId, msg->data[i].name, msg->data[i].level);
       i++;
   }
   for(auto& relationer : role->getEnemys())
   {
       buf.resize(buf.size() + sizeof(PublicRaw::RetAllRelationerInfo::RelationerInfo));
       msg = reinterpret_cast<PublicRaw::RetAllRelationerInfo*>(buf.data());
       msg->data[i].roleId = relationer;
       fillRoleInfo(relationer, msg->data[i].name, msg->data[i].level);
       if(RoleManager::me().getById(relationer))
           msg->data[i].isOnln = IsOnln::yes;
       LOG_DEBUG("好友面板数据请求, 好友变长数据第{}个:roleId= {}, name = {}, level = {}",
                  i, msg->data[i].roleId, msg->data[i].name, msg->data[i].level);
       i++;
   }
   role->sendToMe(RAWMSG_CODE_PUBLIC(RetAllRelationerInfo), buf.data(), buf.size());
   LOG_DEBUG("好友面板数据请求, 发送给客户端, roleId = {}, friendSize = {}, blackSize = {}, enemysize = {}",
             roleId, msg->friendSize, msg->blackSize, msg->enemySize);
}

void FriendManager::insertEnemy(const RoleId roleId, const RoleId enemyId, const BeKilledType type)
{
    bool isexist = false;
    if(isExistEnemy(roleId, enemyId))
    {
        eraseEnemy(roleId, enemyId, false);
        isexist = true;
    }
    auto role = RoleManager::me().getById(roleId);
    auto enemyRole = RoleManager::me().getById(enemyId);
    //role与enemy都应该在线
    if((role == nullptr) || (enemyRole == nullptr))
        return;

    insertEnemyToMap(role, enemyId);
    //这里先插入缓存，如果数据库不成功再删除缓存
    std::list<RoleId> enemyList = role->getEnemyList();
    water::componet::Serialize<std::string> ss;
    ss.reset();
    ss << enemyList;
    if(!insertEnemyToDB(roleId, ss.buffer()))
    {
        eraseEnemyIdFromMap(role, enemyId);
        LOG_DEBUG("添加仇人, 失败, roleId = {}, enemyId = {}",
                  roleId, enemyId);
        return;
    }

    PublicRaw::AddEnemy sendToC;
    sendToC.enemyId = enemyId;
    sendToC.level = enemyRole->level();
    std::memset(sendToC.name, 0, NAME_BUFF_SZIE);
    enemyRole->name().copy(sendToC.name, NAME_BUFF_SZIE);
    sendToC.beKilledType = type;
    role->sendToMe(RAWMSG_CODE_PUBLIC(AddEnemy), (uint8_t*)&sendToC, sizeof(PublicRaw::AddEnemy));
    if(!isexist)
        role->sendSysChat("{}被加入你的仇人列表", enemyRole->name());
    else
        role->sendSysChat("{}再次将你击杀", enemyRole->name());

    LOG_DEBUG("仇人添加, 向客户端发送添加仇人消息, roleId = {}, enemyId = {}, name = {}, level = {}",
              roleId, sendToC.enemyId, sendToC.name, sendToC.level);

}

void FriendManager::eraseEnemy(const RoleId roleId, const RoleId enemyId, bool sendSysChat)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    eraseEnemyIdFromMap(role, enemyId);

    std::list<RoleId> enemyList = role->getEnemyList();
    water::componet::Serialize<std::string> ss;
    ss.reset();
    ss << enemyList;

    if(!eraseEnemyFromDB(roleId, ss.buffer(), enemyList.empty()))
    {
        insertEnemyToMap(role, enemyId);
        LOG_DEBUG("删除仇人, 失败, roleId = {}, enemyId = {}",
                  roleId, enemyId);
        return;
    }
    //发给客户端删除信息
    PublicRaw::RetEraseEnemy sendToC;
    sendToC.enemyId = enemyId;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetEraseEnemy), (uint8_t*)(&sendToC), sizeof(PublicRaw::RetEraseEnemy));
    char enemyName[NAME_BUFF_SZIE];
    uint32_t enemyLevel;
    fillRoleInfo(enemyId, enemyName, enemyLevel);
    if(sendSysChat)
        role->sendSysChat("已将{}从仇人列表移除", enemyName);
}

void FriendManager::insertBlackList(const RoleId roleId, const RoleId blackId)
{
    insertBlackListToMap(roleId, blackId);
    LOG_DEBUG("黑名单添加入缓存, 黑名单添加, roleId = {}, blackId = {}",
              roleId, blackId);

    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    PublicRaw::RetInsertBlackList sendToC;
    sendToC.blackId = blackId;
    fillRoleInfo(blackId, sendToC.name, sendToC.level);
    if(role != nullptr)
    {
        role->sendToMe(RAWMSG_CODE_PUBLIC(RetInsertBlackList), (uint8_t*)&sendToC, sizeof(PublicRaw::RetInsertBlackList));
        LOG_DEBUG("向主动客户端发送名单添加通知, roleId = {}, friendId = {}, name = {}, level = {}",
                  roleId, blackId, sendToC.name, sendToC.level);
        role->sendSysChat("添加黑名单成功");
    }
}

void FriendManager::eraseBlackList(const RoleId roleId, const RoleId blackId)
{
    eraseBlackListFromMap(roleId, blackId);
    LOG_DEBUG("黑名单删除入库, 黑名单删除, roleId = {}, blackId = {}",
               roleId, blackId);

    auto role = RoleManager::me().getById(roleId);
    auto fdRole = RoleManager::me().getById(blackId);
    PublicRaw::RetEraseBlackList sendToC;
    sendToC.blackId = blackId;
    role->sendToMe(RAWMSG_CODE_PUBLIC(RetEraseBlackList), (uint8_t*)&sendToC, sizeof(PublicRaw::RetInsertBlackList));
    LOG_DEBUG("向客户端发送黑名单删除通知, roleId = {}, blackId = {}",
              roleId, blackId);
    if(role != nullptr)
    {
        char name[NAME_BUFF_SZIE];
        uint32_t level;
        fillRoleInfo(blackId, name, level);
        role->sendSysChat("已将{}从黑名单列表移除", name);
    }
}

void FriendManager::tellFriendOnln(const RoleId roleId) const
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    auto friends = role->getFriendSet();
    for(auto fd : friends)
    {
        auto fdRole = RoleManager::me().getById(fd);
        if(fdRole == nullptr)
            continue;
        fdRole->sendSysChat("你的好友{}上线", 
                            role->name());
    }
}

void FriendManager::tellFriendOffln(const RoleId roleId) const
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    auto friends = role->getFriendSet();
    for(auto fd : friends)
    {
        auto fdRole = RoleManager::me().getById(fd);
        if(fdRole == nullptr)
            continue;
        fdRole->sendSysChat("你的好友{}下线", 
                            role->name());
    }
}

void FriendManager::clearOfflnRequetRecord(const RoleId roleId)
{
    //m_RequestRecord.erase(roleId);
}

void FriendManager::fillRoleRelationer(const RoleId roleId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return;
    std::list<RoleId> enemyList;
    std::unordered_set<RoleId> friendSet;
    try
    {
        std::vector<RowOfFriend> friendVec;
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select friendStr from friends where roleId = " << roleId;
        query.storein(friendVec);

        std::vector<RowOfEnemy> enemyVec;
        query.reset();
        query << "select enemyId from enemy where roleId = " << roleId;
        query.storein(enemyVec);

        if(friendVec.size() != 0)
        {
            std::string ss = friendVec[0].friendStr;
            water::componet::Deserialize<std::string> ds(&ss);
            ds >> friendSet;
        }
        if(enemyVec.size() != 0)
        {

            std::string ss = enemyVec[0].enemyId;
            water::componet::Deserialize<std::string> ds(&ss);
            ds >> enemyList;
        }
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("角色上线, 社会关系初始化, DB error{}", er.what());
        return;
    }

    //初始化及同步到world
    if(friendSet.size() != 0)
    {
        role->setFriendSet(friendSet);
    }
    if(enemyList.size() != 0)//同步world
    {
        role->setEnemyList(enemyList);
        role->synWorldEnemy();
    }
    auto black = m_blackListMap.find(role->id());
    if(black != m_blackListMap.end())
    {
        sysnBlackToWorld(role);
    }
}

bool FriendManager::updateFriendToDB(RoleId roleId, const std::string* friendStr, RoleId friendId, const std::string* fdFriend)
{
    try 
    {   
        //两次操作同一张表，这里不做事务，不需要太严谨
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfFriend row(roleId, *friendStr);
        RowOfFriend fdRow(friendId, *fdFriend);
        query.replace(row);
        query.execute();
        query.reset();
        query.replace(fdRow);
        query.execute();
        return true;
    }   
    catch(const mysqlpp::Exception& er) 
    {   
        LOG_ERROR("添加好友, DB error:{}", er.what());
        return false;
    }   
}

bool FriendManager::eraseFriendFromDB(RoleId roleId, const std::string* friendStr, RoleId friendId)
{
    try
    {
        //先查找出被删除者的好友信息，删除对应好友
        std::vector<RowOfFriend> fdFriendVec;
        std::unordered_set<RoleId> fdFriendSet;
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        query << "select friendId from friends where roleId = " << friendId;
        query.storein(fdFriendVec);
        if(fdFriendVec.size() != 0)
        {
            std::string ss = fdFriendVec[0].friendStr;
            water::componet::Deserialize<std::string> ds(&ss);
            ds >> fdFriendSet;
            fdFriendSet.erase(roleId);
        }
        //更新
        water::componet::Serialize<std::string> ss;
        ss.reset();
        ss << fdFriendSet;
        RowOfFriend row(roleId, *friendStr);
        RowOfFriend fdRow(friendId, *ss.buffer());
        query.reset();
        query.replace(row);
        query.execute();
        query.reset();
        query.replace(fdRow);
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("删除好友, DB error:{}", er.what());
        return false;
    }
}

bool FriendManager::insertEnemyToDB(RoleId roleId, const std::string* enemy)
{
    //replace,有就更新，没有就插入
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        RowOfEnemy enemyRow(roleId, *enemy);
        query.replace(enemyRow);
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("仇人增加, 数据库更新失败, DB error:{}", er.what());
        return false;
    }
}

bool FriendManager::eraseEnemyFromDB(RoleId roleId, const std::string* enemy, bool isEmpty)
{
    try
    {
        mysqlpp::Query query = dbadaptcher::MysqlConnectionPool::me().getconn()->query();
        if(!isEmpty)
        {
            query << "update enemy set enemyId = " << mysqlpp::quote << *enemy
            << " where roleId = " << roleId;
        }
        else
        {
            query << "delete from enemy where roleId = " << roleId;
        }
        query.execute();
        return true;
    }
    catch(const mysqlpp::Exception& er)
    {
        LOG_ERROR("仇人删除, 数据库更新失败, DB error:{}", er.what());
        return false;
    }
}

//缓存
bool FriendManager::isExistFriend(RoleId roleId, RoleId friendId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return false;
     if(role->isExistFriend(friendId))
         return true;
     return false;
}

bool FriendManager::insertFriendToMap(Role::Ptr role, Role::Ptr fdRole)
{
    if(role != nullptr)
        role->insertFriend(fdRole->id());
    if(fdRole != nullptr)
        fdRole->insertFriend(role->id());
    return true;
}

bool FriendManager::eraseFriendFromMap(Role::Ptr role, Role::Ptr fdRole) 
{
    if(role != nullptr)
        role->eraseFriend(fdRole->id());
    if(fdRole != nullptr)
        fdRole->eraseFriend(role->id());
    return true;
}

bool FriendManager::isExistEnemy(RoleId roleId, RoleId enemyId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return false;
    if(role->isExistEnemy(enemyId))
        return true;
    return false;
}

bool FriendManager::insertEnemyToMap(Role::Ptr role, RoleId enemyId)
{
    role->insertEnemy(enemyId, m_enemyNumLimit);
    return true;
}

bool FriendManager::eraseEnemyIdFromMap(Role::Ptr role, RoleId enemyId)
{
    if(role == nullptr)
        return false;
    role->eraseEnemy(enemyId);
    return true;
}

bool FriendManager::isExistBlackList(const RoleId roleId, const RoleId blackId)
{
    auto roleRet = m_blackListMap.find(roleId);
    if(roleRet == m_blackListMap.end())
        return false;
    auto blackRet = roleRet->second.find(blackId);
    if(blackRet == roleRet->second.end())
        return false;
    return true;
}

bool FriendManager::insertBlackListToMap(const RoleId roleId, const RoleId blackId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return false;
    m_blackListMap[roleId].insert(blackId);
    sysnBlackToWorld(role);
    return true;
}

void FriendManager::sysnBlackToWorld(Role::Ptr role)
{
    water::componet::Serialize<std::string> ss;
    ss.reset();
    ss << m_blackListMap[role->id()];
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PrivateRaw::UpdateWorldBlack) + ss.tellp());
    auto* msg = reinterpret_cast<PrivateRaw::UpdateWorldBlack*>(buf.data());
    msg->roleId = role->id();
    msg->size = ss.tellp();
    std::memcpy(msg->buf, ss.buffer()->data(), ss.tellp());
    Func::me().sendToPrivate(role->worldId(), RAWMSG_CODE_PRIVATE(UpdateWorldBlack), buf.data(), buf.size());

}

bool FriendManager::eraseBlackListFromMap(const RoleId roleId, const RoleId blackId)
{
    auto role = RoleManager::me().getById(roleId);
    if(role == nullptr)
        return false;
    auto it = m_blackListMap.find(roleId);
    if(it == m_blackListMap.end())
        return true;
    it->second.erase(blackId);
    sysnBlackToWorld(role);
    return true;
}

void FriendManager::fillRoleInfo(RoleId roleId, char* name, uint32_t& level)
{
    auto roleInfo = RoleInfoManager::me().getRoleInfoById(roleId);
    std::memset(name, 0, NAME_BUFF_SZIE);
    roleInfo->name.copy(name, NAME_BUFF_SZIE);
    level = roleInfo->level;
}


}
