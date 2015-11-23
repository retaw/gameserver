#include "field_boss_manager.h"
#include "npc.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/field_boss.codedef.public.h"
#include "protocol/rawmsg/public/field_boss.h"

#include "protocol/rawmsg/private/field_boss.codedef.private.h"
#include "protocol/rawmsg/private/field_boss.h"

#include "roles_and_scenes.h"
#include "water/componet/string_kit.h"

namespace world{

FieldBossManager& FieldBossManager::me()
{
    static FieldBossManager me;
    return me;
}

void FieldBossManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PUBLIC(TransFerToFieldBoss, std::bind(&FieldBossManager::clientmsg_TransFerToFieldBoss, this, _1, _2, _3));
}

void FieldBossManager::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/field_boss.xml";
    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");


    std::vector<std::string> transferVec;
    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        FieldBossTpl::Ptr bossPtr = FieldBossTpl::create();
        bossPtr->bossId = itemNode.getChildNodeText<uint32_t>("bossId");
        bossPtr->npcId = itemNode.getChildNodeText<uint32_t>("npcId");
        bossPtr->npcMapId = itemNode.getChildNodeText<uint32_t>("mapId");
        std::string str = itemNode.getChildNodeText<std::string>("transfer");
        transferVec.clear();
        transferVec =  water::componet::splitString(str, "-");
        if(transferVec.size() < 3)
        {
            LOG_ERROR("{}配置错误, transfer出错, bossId={}", cfgdir, bossPtr->bossId);
            continue;
        }
        bossPtr->transferMapId = water::componet::fromString<uint32_t>(transferVec[0]);
        bossPtr->posx = water::componet::fromString<uint16_t>(transferVec[1]);
        bossPtr->posy = water::componet::fromString<uint16_t>(transferVec[2]);
        m_fieldBoss.emplace(bossPtr->bossId, bossPtr);
        m_npcTplIdMapId2BossId.emplace(std::make_pair(bossPtr->npcId, bossPtr->npcMapId), bossPtr->bossId);
    }
}

void FieldBossManager::clientmsg_TransFerToFieldBoss(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
    if(msgSize != sizeof(PublicRaw::TransFerToFieldBoss))
        return;

    auto rev = reinterpret_cast<const PublicRaw::TransFerToFieldBoss*>(msgData);
    
    auto it = m_fieldBoss.find(rev->bossId);
    if(it == m_fieldBoss.end())
        return;
    auto bossTpr = it->second;
    //传送
    bool tansFerSuccess = RolesAndScenes::me().gotoOtherScene(roleId, bossTpr->transferMapId, Coord2D(bossTpr->posx, bossTpr->posy));
    if(tansFerSuccess)
    {
        //成功返回给客户端
        PublicRaw::TransFerToFieldBossSuccess send;
        auto role = RoleManager::me().getById(roleId);
        if(role == nullptr)
            return;
        role->sendToMe(RAWMSG_CODE_PUBLIC(TransFerToFieldBossSuccess), &send, 0);
    }
}

void FieldBossManager::toDie(const uint32_t npcTplId, const MapId mapId, const std::vector<uint32_t>& objIdVec, Role::Ptr role)
{
    auto it = m_npcTplIdMapId2BossId.find(std::make_pair(npcTplId, mapId));
    if(it == m_npcTplIdMapId2BossId.end())
    {
        LOG_ERROR("野外boss死亡, 在boss表中未找到该boss, npcId={}, mapId={}", npcTplId, mapId);
        return;
    }
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PrivateRaw::FieldBossToDie));
    auto msg = reinterpret_cast<PrivateRaw::FieldBossToDie*>(buf.data());
    msg->bossId = it->second;
    msg->killerId = role->id();
    msg->size = objIdVec.size();
    for(uint16_t i = 0; i < objIdVec.size(); i++)
    {
        buf.resize(buf.size() + sizeof(uint32_t));
        msg = reinterpret_cast<PrivateRaw::FieldBossToDie*>(buf.data());
        msg->objId[i] = objIdVec[i];
    }
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(FieldBossToDie), buf.data(), buf.size());
}

void FieldBossManager::refreshFieldBoss(uint32_t npcTplId, uint32_t mapId)
{
    auto it = m_npcTplIdMapId2BossId.find(std::make_pair(npcTplId, mapId));
    if(it == m_npcTplIdMapId2BossId.end())
    {
        LOG_ERROR("野外boss刷新, 在boss表中未找到该boss, npcId={}, mapId={}", npcTplId, mapId);
        return;
    }

    PrivateRaw::RefreshFieldBoss send;
    send.bossId = it->second;

    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(RefreshFieldBoss), &send, sizeof(send));
}

}
