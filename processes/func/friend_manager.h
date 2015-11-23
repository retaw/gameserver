
#include "role_manager.h"

#include "water/common/roledef.h"
#include "water/common/frienddef.h"

#include <unordered_set>
#include <set>
#ifndef PROCESSES_FUNC_FRIENDS_MANAGER_H
#define PROCESSES_FUNC_FRIENDS_MANAGER_H

namespace func{
class FriendManager
{
const uint32_t RcommonFdNum = 10;//推荐好友数量

    struct RequestRecord
    {
        bool operator < (const RequestRecord& other) const
        {
            if(roleId == other.roleId)
                return this->friendId < other.friendId;
            return this->roleId < other.roleId;
        }
        RoleId roleId;
        RoleId friendId;
    };
public:
    ~FriendManager() = default;
    void regMsgHandler();
    static FriendManager& me();
    void loadConfig(const std::string& configDir);
public:
    //是否在某人的黑名单内
    bool isExistBlackList(const RoleId roleId, const RoleId blackId);
    //初始化上线角色的好友、仇人
    void fillRoleRelationer(const RoleId roleId);
    //好友上线通知
    void tellFriendOnln(const RoleId roleId) const;
    //好友下线通知
    void tellFriendOffln(const RoleId roleId) const;
    //清空好友请求记录（待处理）
    void clearOfflnRequetRecord(const RoleId roleId);
private:
    void clientmsg_RequestFriendToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RetRequestFriendToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_EraseFriendToS(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_InsertBlackList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_EraseBlackList(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestAllRelationerInfo(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_EraseEnemy(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);
    void clientmsg_RequestRecommendFriend(const uint8_t* msgData, uint32_t msgSize, RoleId roleId);

    void servermsg_InsertEnemy(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);

private:
    FriendManager() = default;

    void fillRoleInfo(RoleId roleId, char* name, uint32_t& level);
    void fillRecommendFriend(uint32_t level, std::vector<uint8_t>& buf, RoleId roleId);
    void getSmallAndBigRoleId(uint64_t& small, uint64_t& big, RoleId roleId1, RoleId roleId2);

    //业务
    void sendRequestToFriend(Role::Ptr role, Role::Ptr fdRole);
    bool canAddFriend(Role::Ptr role, Role::Ptr fdRole);
    void insertFriend(Role::Ptr role, Role::Ptr fdRole);
    void eraseFriend(const RoleId roleId, const RoleId friendId);
    void sendAllRelationerInfoToMe(const RoleId roleId);
    void insertEnemy(const RoleId roleId, const RoleId enemyId, const BeKilledType type);
    void eraseEnemy(const RoleId roleId, const RoleId enemyId, bool sendSysChat = true);
    void insertBlackList(const RoleId roleId, const RoleId blackId);
    void eraseBlackList(const RoleId roleId, const RoleId blackId);

    //DB
    bool updateFriendToDB(RoleId roleId, const std::string*, RoleId friendId, const std::string* );
    bool eraseFriendFromDB(RoleId roleId, const std::string*, RoleId friendId);
    bool insertEnemyToDB(RoleId roleId, const std::string* enemy);
    bool eraseEnemyFromDB(RoleId roleId, const std::string* enemy, bool isEmpty);

    //缓存
    bool isExistFriend(RoleId roleId, RoleId friendId);
    bool insertFriendToMap(Role::Ptr role, Role::Ptr fdRole);
    bool eraseFriendFromMap(Role::Ptr role, Role::Ptr fdRole);

    bool isExistEnemy(RoleId roleId, RoleId enemyId);
    bool insertEnemyToMap(Role::Ptr role, RoleId enemyId);
    bool eraseEnemyIdFromMap(Role::Ptr role, RoleId enemyId);

    bool insertBlackListToMap(const RoleId roleId, const RoleId blackId);
    bool eraseBlackListFromMap(const RoleId roleId, const RoleId blackId);
    void sysnBlackToWorld(Role::Ptr role);

private:
    //好友请求记录
    std::map<RoleId, std::set<RoleId>> m_RequestRecord;
    //1级角色好友上限，每升一级加1
    uint32_t m_friendNumLimit = 20;
    //好友功能开启等级
    uint32_t m_friendStartLevel = 1;

    //黑名单缓存，不需要入库，服务器重启清除即可
    std::unordered_map<RoleId, std::unordered_set<RoleId>> m_blackListMap;
    //仇人列表保存最多个数
    uint16_t m_enemyNumLimit = 50;
};

}
#endif
