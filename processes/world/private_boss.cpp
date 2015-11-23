#include "private_boss.h"
#include "role_manager.h"
#include "roles_and_scenes.h"
#include "scene_manager.h"
#include "mail_manager.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/public/private_boss.codedef.public.h"
#include "protocol/rawmsg/public/private_boss.h"
#include "protocol/rawmsg/public/field_boss.codedef.public.h"
#include "protocol/rawmsg/public/field_boss.h"


namespace world{

PrivateBoss::PrivateBoss(Role& me)
: owner(me)
{
}

void PrivateBoss::retPrivateBoss()
{
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RetPrivateBoss));
    uint16_t i = 0;
    auto& bossTplMap = PrivateBossBase::me().m_privateBoss;
    auto& roleBossData = owner.m_roleSundry.privateBossMap;
    for(auto& it : bossTplMap)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RetPrivateBoss::PrivateBossData));
        auto msg = reinterpret_cast<PublicRaw::RetPrivateBoss*>(buf.data());
        msg->data[i].bossId = it.second->bossId;

        auto  bossDataPair = roleBossData.find(it.second->bossId);
        if(bossDataPair == roleBossData.end())
        {
            //说明bossdata不存在，即为初始化状态
            msg->data[i].canEnterTimes = getEnterTimes(it.second);
        }
        else
        {
            msg->data[i].canEnterTimes = getEnterTimes(it.second) - bossDataPair->second;
        }
        i++;

    }
    auto msg = reinterpret_cast<PublicRaw::RetPrivateBoss*>(buf.data());
    msg->mohun = owner.mohun();
    msg->size = i;

    owner.sendToMe(RAWMSG_CODE_PUBLIC(RetPrivateBoss), buf.data(), buf.size());
}

void PrivateBoss::afterEnterScene()
{

    auto scene = owner.scene();
    if(scene == nullptr)
        return;

    if(scene->mapTpl()->type != CopyMap::private_boss)
        return;

    //进场景刷新boss初始列表
    m_npcInMap = SceneManager::me().getNpcTplSize(scene->id());

    //刷新右侧面板.客户端要自己拉
    //sendRefreshPrivateBoss();

    //发送切换成功，告诉客户端关闭boss面板
    PublicRaw::TransFerToFieldBossSuccess send;
    owner.sendToMe(RAWMSG_CODE_PUBLIC(TransFerToFieldBossSuccess), &send, 0);

    //设置进入场景的时间
    owner.scene()->setRoleInTime(); 

    //给客户端发送倒计时时间,客户端要自己拉
    
}

void PrivateBoss::sendRemainSeconds()
{
    auto scene = owner.scene();
    if(scene == nullptr)
        return;

    using namespace std::chrono;
    //给客户端发送倒计时时间
    PublicRaw::PrivateBossRemainSeconds rsend;
    water::componet::TimePoint now = water::componet::Clock::now();
    uint32_t counter = duration_cast<seconds>(now - scene->createTime()).count();//不会出现<0的情况，不考虑
    auto bossTplPair = PrivateBossBase::me().m_privateBoss.find(owner.scene()->bossId());
    if(bossTplPair == PrivateBossBase::me().m_privateBoss.end())
    {
        LOG_ERROR("个人boss进入场景错误, 未找到boss配置, bossId={}", owner.scene()->bossId());
        return;
    }
    uint32_t bossDuration = bossTplPair->second->duration;
    if(counter >= bossDuration)//此时已经超时不应该进来
        rsend.seconds = 0;
    else
        rsend.seconds = bossDuration - counter;
    rsend.bossId = scene->bossId();
    owner.sendToMe(RAWMSG_CODE_PUBLIC(PrivateBossRemainSeconds), &rsend, sizeof(rsend));
    
    
}

/*
void PrivateBoss::refreshnpcInScene()
{
    //刷新m_npcInScene
    auto scene = owner.scene();
    if(scene == nullptr)
        return;

    if(scene->mapTpl()->type != CopyMap::private_boss)
        return;

    //得到当前场景剩余的各个npc类型的剩余数量
    m_npcInScene.clear();
    auto func = [this] (Npc::Ptr npc)
    {
        m_npcInScene[npc->tplId()]++;    
    };
    scene->execNpc(func);
}
*/
void PrivateBoss::sendRefreshPrivateBoss()
{
    //刷新m_npcInScene
    auto scene = owner.scene();
    if(scene == nullptr)
        return;

    if(scene->mapTpl()->type != CopyMap::private_boss)
        return;

    auto npcInScene = scene->m_npcInScene;
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::RefreshPrivateBoss));
    uint16_t i = 0;
    for(auto& it : m_npcInMap)
    {
        buf.resize(buf.size() + sizeof(PublicRaw::RefreshPrivateBoss::NpcData));
        auto msg = reinterpret_cast<PublicRaw::RefreshPrivateBoss*>(buf.data());
        msg->data[i].npcId = it.first;
        msg->data[i].killNum = npcInScene[it.first];
        msg->data[i].allNum = it.second;
        i++;
    }
    auto msg = reinterpret_cast<PublicRaw::RefreshPrivateBoss*>(buf.data());
    msg->size = i;

    owner.sendToMe(RAWMSG_CODE_PUBLIC(RefreshPrivateBoss), buf.data(), buf.size());
    
}

void PrivateBoss::npcDie(const NpcTplId npcTplId, const std::vector<uint32_t>& objIdVec)
{
    //scene上的npc击杀数量增加
    if(owner.scene() == nullptr)
        return;
    owner.scene()->m_npcInScene[npcTplId]++;
    //发送右侧面板展示挑战状态
    sendRefreshPrivateBoss();

    //找到对应的bossId
    auto it = PrivateBossBase::me().m_npcTplIdMapId2BossId.find(std::make_pair(npcTplId, owner.scene()->mapTpl()->id));
    if(it == PrivateBossBase::me().m_npcTplIdMapId2BossId.end())
    {
        LOG_ERROR("个人boss死亡物品掉落广播, 在boss表中未找到该boss, npcId={}, mapId={}", npcTplId, owner.scene()->mapTpl()->id);
        return;
    }

    //发送物品掉落广播
    std::vector<uint8_t> buf;
    buf.reserve(1024);
    buf.resize(sizeof(PublicRaw::PrivateBossBroadcastObj));
    auto msg = reinterpret_cast<PublicRaw::PrivateBossBroadcastObj*>(buf.data());
    msg->bossId = it->second;
    msg->roleId = owner.id();
    std::memset(msg->killerName, 0, NAME_BUFF_SZIE);
    owner.name().copy(msg->killerName, NAME_BUFF_SZIE);
    msg->size = objIdVec.size();
    for(uint16_t i = 0; i < objIdVec.size(); i++)
    {
        buf.resize(buf.size() + sizeof(uint32_t));
        msg = reinterpret_cast<PublicRaw::PrivateBossBroadcastObj*>(buf.data());
        msg->objId[i] = objIdVec[i];
    }
    owner.sendToMe(RAWMSG_CODE_PUBLIC(PrivateBossBroadcastObj), buf.data(), buf.size());
}

void PrivateBoss::leave()
{
    if(owner.scene() == nullptr)
        return;
    if(owner.scene()->mapTpl()->type != CopyMap::private_boss)
        return;
    //发奖
    uint32_t bossId = owner.scene()->bossId();
    getReward(bossId);

    //得到原场景id
    auto sceneId = owner.scene()->id();
    //回到原坐标
    bool tansFerSuccess = RolesAndScenes::me().gotoOtherScene(owner.id(), owner.preSceneId(), owner.prePos());
    if(!tansFerSuccess)
    {
        LOG_ERROR("从副本个人boss回到原场景失败");
        return;
    }

    //销毁动态场景
    SceneManager::me().destroyDynamicScene(sceneId);
    
}

void PrivateBoss::getReward(uint32_t bossId)
{
    //线判断是否完成任务
    auto npcInScene = owner.scene()->m_npcInScene;
    for(auto& it : m_npcInMap)
    {
        if(it.second != npcInScene[it.first])
            return;
    }
    //发奖只有离开的时候发，此时一定还在动态场景上
    auto scene = owner.scene();
    if(scene == nullptr)
        return;

    //得到boss配置
    auto bossTplMap = PrivateBossBase::me().m_privateBoss;
    auto bossTplPair = bossTplMap.find(bossId);
    if(bossTplPair == bossTplMap.end())
    {
        return;
    }
    auto bossTpl = bossTplPair->second;

    for(auto& reward : bossTpl->reward)
    {
        if(owner.checkPutObj(reward.objTplId, reward.objnum, reward.bind, PackageType::role))
        {
            owner.putObj(reward.objTplId, reward.objnum, reward.bind, PackageType::role);
            owner.sendSysChat("个人boss通关奖励已发放");
        }
        else//发邮件
        {
            std::vector<ObjItem> objVec;
            ObjItem temp;
            temp.tplId = reward.objTplId;
            temp.num = reward.objnum;
            temp.bind = reward.bind;
            objVec.push_back(std::move(temp));
            std::string text = "您在个人boss中顺利通关, 获得通关奖励";
            MailManager::me().send(owner.id(), "个人boss通关奖励", text, objVec);
            owner.sendSysChat("您的背包空间不足, 个人boss通关奖励通过邮件发放");
        }
        
    }
}

void PrivateBoss::transFerToPrivateBoss(uint32_t bossId)
{
    auto bossTplMap = PrivateBossBase::me().m_privateBoss;
    auto bossTplPair = bossTplMap.find(bossId);
    if(bossTplPair == bossTplMap.end())
    {
        return;
    }
    auto bossTpl = bossTplPair->second;
    //判断今日是否还有挑战次数
    auto roleBossData = owner.m_roleSundry.privateBossMap;
    if(getEnterTimes(bossTpl) <= roleBossData[bossId])
    {
        owner.sendSysChat("今日挑战次数已用完");
        return;
    }

    //最好做一次背包物品的判断，防止创建场景后发现物品不足还要再销毁场景，做无用功
    //...

    //创建动态场景,创建后等回调函数即可
    using namespace std::placeholders;
    SceneManager::me().createDynamicScene(bossTpl->transferMapId, std::bind(&PrivateBoss::createDynamicSceneCallback, this, _1, owner.id(), bossId, bossTpl->posx, bossTpl->posy, *bossTpl));

}

void PrivateBoss::createDynamicSceneCallback(SceneId sceneId, RoleId roleId, uint32_t bossId, uint16_t posx, uint16_t posy, PrivateBossTpl bossTpl)
{
    if(sceneId == 0)
    {
        owner.sendSysChat("个人boss地图暂时无法使用");
        return;
    }
    //跳场景,角色等级与道具限制都在传送内部判断
    bool tansFerSuccess = RolesAndScenes::me().gotoOtherScene(roleId, sceneId, Coord2D(posx, posy));
    if(tansFerSuccess)
    {
        //设置场景上的boss标识，可以理解为此地图的主人是哪个boss，只有副本有
        if(owner.scene() != nullptr)
        {
            //设置个人boss场景的bossId
            owner.scene()->setBossId(bossId);
            //设置创建场景的时间，用于倒计时
            owner.scene()->setCreateTime(); 
            //个人挑战boss次数增加
            owner.m_roleSundry.privateBossMap[bossId]++;
            owner.m_roleSundry.saveSundry();
        }
    }
    else
    {
        SceneManager::me().destroyDynamicScene(sceneId);
    }
}

uint16_t PrivateBoss::getEnterTimes(PrivateBossTpl::Ptr bossTpl)
{
    if(bossTpl->enterTimes.size() <= owner.vipLevel())
    {
        LOG_ERROR("个人boss读取配置错误, 为找到vip对应的可太欧战次数, vip={}", owner.vipLevel());
        return 0;
    }
    return bossTpl->enterTimes[owner.vipLevel()];
}

void PrivateBoss::checkTimeOut()
{
    if(owner.scene() == nullptr)
        return;
    auto sceneId = owner.scene()->id();
    //时间到，只要强制离开即可
    bool tansFerSuccess = RolesAndScenes::me().gotoOtherScene(owner.id(), owner.preSceneId(), owner.prePos());
    if(!tansFerSuccess)
    {
        LOG_ERROR("离开个人boss场景失败");
        return;
    }
    SceneManager::me().destroyDynamicScene(sceneId);
    owner.sendSysChat("个人boss挑战失败，你未在规定时间内杀死所有怪物");
}


}
