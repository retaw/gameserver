#include "npc.h"
#include "role.h"
#include "scene.h"
#include "ai.h"
#include "shabake.h"

#include "water/componet/random.h"
#include "scene_object_manager.h"

#include "protocol/rawmsg/public/npc_scene.h"
#include "protocol/rawmsg/public/npc_scene.codedef.public.h"
#include "protocol/rawmsg/public/object_scene.h"
#include "protocol/rawmsg/public/object_scene.codedef.public.h"
#include "object_config.h"

#include "npc_manager.h"
#include "relations_manager.h"
#include "world_boss.h"
#include <algorithm>
#include "field_boss_manager.h"
#include "channel.h"
#include "shabake.h"
#include "boss_home_manager.h"
#include "reward_manager.h"

namespace world{


Npc::Npc(NpcId id, NpcTpl::Ptr tpl)
: PK(id, tpl->name, tpl->job, SceneItemType::npc), m_tpl(tpl), m_dead(false)
{
    setDir(componet::Direction::down);
}

const NpcTpl& Npc::tpl() const
{
    return *m_tpl;
}

NpcTplId Npc::tplId() const
{
    return tpl().id;
}

uint32_t Npc::aiTplId() const
{
    return tpl().aiTplId;
}

void Npc::resetAI()
{
    m_ai = ai::AIManager::me().create(aiTplId());
}

NpcType Npc::type() const
{
    return tpl().type;
}

std::string Npc::toString() const
{
    return water::componet::format("[{}, {}, {}]", id(), name(), tplId());
}

uint16_t Npc::extend() const
{
    return tpl().extend;
}

uint16_t Npc::belongtime() const
{
    return tpl().timeOfBelongTo;
}

void Npc::initTplData()
{
    setLevel(tpl().level);
    setAttribute(tpl().props);
    setHp(getMaxHp());
    setMp(getMaxMp());
    m_ai = ai::AIManager::me().create(aiTplId());
    m_moveData.stepInterval = tpl().stepCost + tpl().stopCost;
    m_moveData.stepDuration = tpl().stepCost;
    m_skillM.initSkill(tpl().skillTplId);
}


void Npc::toDie(PK::Ptr atk)
{
    LOG_TRACE("npc:{},{} 死亡", name(), tplId());
    setDead();
    m_skillEffectM.clear();
    m_buffM.processDeath();

    auto atkRole = getRole(atk);
    if(nullptr != atkRole)
    {
        atkRole->addExp(tpl().roleExp);
        auto atkHero = atkRole->m_heroManager.getDefaultHero();
        if(nullptr != atkHero)
            atkHero->addExp(tpl().heroExp);

        TaskParam param;
        param.element.npcTplId = tplId();
        param.element.npclevel = level();
        atkRole->dispatchTask(TaskContent::kill_npc, param);

        WorldBoss::me().npcDie(tplId(), atkRole);
    }

    ShaBaKe::me().npcDie(std::static_pointer_cast<Npc>(shared_from_this()));
    dropObj(atk);  //掉落物品
    m_moveData.path.clear(); //死亡不能再移动
}

void Npc::setDead()
{
    m_collectRoleId = 0;
    m_collectTp = EPOCH;
    if(type() == NpcType::collect && 1 == m_tpl->keepCorpseSec)//采集后不消失
        return;
    m_dead = true;
    m_isCorpseExist = true;
    m_dieTime = componet::Clock::now();

	Scene::Ptr s = scene();
	if(s == nullptr)
		return;
    PublicRaw::NpcDieInfo info;
    info.id = id(); 
    s->sendCmdTo9(RAWMSG_CODE_PUBLIC(NpcDieInfo), &info, sizeof(info), pos());
}

bool Npc::isDead() const
{
    return m_dead;
}

void Npc::fillScreenData(PublicRaw::NpcScreenData* data) const
{
    data->id    = id();
    data->tplId = tplId();
    std::memset(data->name, 0, sizeof(data->name)-1);
    if(data->tplId == ShaBaKe::me().kingStatueId() && "" != ShaBaKe::me().kingStatue())
    {
        ShaBaKe::me().kingStatue().copy(data->name, sizeof(data->name)-1);
    }
    else
    {
        name().copy(data->name, sizeof(data->name)-1);
    }
    data->level = level();
    data->posX = pos().x;
    data->posY = pos().y;
    data->maxhp = getMaxHp();
    data->hp = getHp();
    data->dir = static_cast<decltype(data->dir)>(dir());
    data->pkStatus = m_pkstate.pkstatus();
}

void Npc::setOwnerId(PKId ownerId)
{
    m_ownerId = ownerId;
}

PKId Npc::getOwnerId() const
{
    return m_ownerId;
}

void Npc::handleAIEvent(const ai::AIEvent* aiEvent)
{
    if(m_ai == nullptr)
        return;
    if(isDead())
        return;

    m_ai->handleEvent(this, aiEvent);
}

AIData& Npc:: aiData()
{
    return m_aiData;
}

void Npc::afterEnterScene()
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return;

    enterVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void Npc::beforeLeaveScene()
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return;

    leaveVisualScreens(s->get9ScreensByGridCoord(pos()));
}

void Npc::enterVisualScreens(const std::vector<Screen*>& screens) const
{
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;

    //自己进入别人视野
    PublicRaw::NpcsAroundMe send;
    fillScreenData(&(send.npcs[0]));
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(NpcsAroundMe), &send, sizeof(send));
}

void Npc::leaveVisualScreens(const std::vector<Screen*>& screens) const
{
	Scene::Ptr s = scene();
	if(nullptr == s)
		return;

    //将自己从别人的视野中删除
    PublicRaw::NpcLeaveInfo send;
    send.id[0] = id();
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(NpcLeaveInfo), &send, sizeof(send));
}

void Npc::tryIdelMove()
{
    componet::TimePoint now = componet::Clock::now();
    //正在走, 就不执行这个了
    if(!m_moveData.path.empty() && m_moveData.nextStepAble > now)
        return;

    //正在打架, 不执行
    if(m_aiData.target.type != SceneItemType::none)
        return;

    auto s = scene();
    if(s == nullptr)
        return;

    auto neighbors = pos().allNeighbors();
    componet::Random<uint32_t> rand(0, neighbors.size() - 1); 

    for(uint32_t i = 0, j = rand.get(); i < neighbors.size(); ++i)
    {   
        Coord2D goal = neighbors[(j + i) % neighbors.size()];
        if(goal.manhattanDistance(homePos()) > 3)
            continue;

        Grid* grid = s->getGridByGridCoord(goal);
        if(grid == nullptr)
            continue;

        if(changePos(goal, pos().direction(goal), MoveType::walk))
            return;
    }
}

void Npc::moveToNearby(Coord2D goal, Coord1D nearby /*= 0*/)
{
    m_moveData.nearby = nearby;

    //正在向那边走, 不用再寻路了
    if(!m_moveData.path.empty() && 
       (std::abs(m_moveData.path.back().x - goal.x) <= nearby && 
        std::abs(m_moveData.path.back().y - goal.y <= nearby)))
        return;

    auto s = scene();
    if(s == nullptr)
        return;

    componet::TimePoint now = componet::Clock::now();

    Coord2D start = m_moveData.path.empty() ? pos() : m_moveData.path.front();
    m_moveData.path = s->findPath(start, goal, SceneItemType::npc);
    m_moveData.goal = goal;

    if(!m_moveData.path.empty() && m_moveData.nextStepAble < now)//说明当前立刻可以开走
    {
        m_moveData.nextStepAble = now;
        followThePath(now);
    }

}

void Npc::followThePath(const componet::TimePoint& now)
{
    auto s = scene();
    if(s == nullptr)
        return;

    //当前这一步还没走完
    if(now < m_moveData.nextStepAble)
        return;

    //路径已空
    if(m_moveData.path.empty())
        return;

    if(std::abs(pos().x - m_moveData.goal.x) <= m_moveData.nearby &&
       std::abs(pos().y - m_moveData.goal.y) <= m_moveData.nearby)
    {
        m_moveData.path.clear();
        return;
    }

    MoveType moveType = MoveType::walk;

    Coord2D nextPos = m_moveData.path.front();
    LOG_DEBUG("npc followPath to {}", nextPos);
    if(!changePos(nextPos, pos().direction(nextPos), moveType))
    {
        m_moveData.path = s->findPath(pos(), m_moveData.goal, SceneItemType::npc);
        if(m_moveData.path.empty())
            return;

        if(!changePos(nextPos, pos().direction(nextPos), moveType)) //不可能
        {
            LOG_ERROR("npc 寻路成功但路径的第一格不可达, {}->{}", pos(), m_moveData.path.front());
            m_moveData.path.clear();
            return;
        }
    }

    m_moveData.path.pop_front();
    m_moveData.nextStepAble = m_moveData.nextStepAble + m_moveData.stepInterval;
    m_moveData.lastStepDone = m_moveData.lastStepDone + m_moveData.stepDuration;
}

void Npc::checkDealCorpseAndrelive(const componet::TimePoint& now)
{
    if(needErase())
        return;
    Scene::Ptr s = scene();
    if(s == nullptr)
        return;
    Npc::Ptr me = std::static_pointer_cast<Npc>(shared_from_this());
    if(m_tpl->keepCorpseSec > m_tpl->reliveSec && m_tpl->reliveSec != 0)
    {
        LOG_ERROR("怪物死亡复活, npc.xml中keepCorpseSec大于reliveSec的情况, 配置出错, 此怪物不再复活");
        markErase(true);
        beforeLeaveScene();
        s->eraseNpc(me);
        return;
    }
    else if(now >= m_dieTime + std::chrono::seconds(m_tpl->keepCorpseSec) && m_isCorpseExist)
    {
        m_isCorpseExist = false;
        beforeLeaveScene();
        s->eraseNpc(me);
        if(m_tpl->reliveSec == 0)
        {
            LOG_DEBUG("怪物复活时间为0, 不再复活, 清除怪物, npcId={}", id());
            markErase(true);
            return;
        }
    }   

    if(now >= m_dieTime + std::chrono::seconds(m_tpl->reliveSec) && m_tpl->reliveSec != 0)
    {   
        m_dead = false;
        setPos(m_originalPos);  //位置恢复到初始位置
        initTplData();
        if(!(s->addNpc(me,m_originalPos,10)))
        {
            LOG_ERROR("怪物死亡复活, 场景添加复活的怪物失败, 初始点周围10范围内无法添加, posx = {}, posy = {}",
                      m_originalPos.x, m_originalPos.y);
            return;
        }
        afterEnterScene();

        //野外boss复活
        if(type() == NpcType::boss && scene()->mapTpl()->type == CopyMap::none)
        {
            FieldBossManager::me().refreshFieldBoss(tplId(), scene()->mapId());
        }
        //boss之家复活
        if(scene()->mapTpl()->type == CopyMap::boss_home)
        {
            BossHomeManager::me().refreshbossHome(tplId(), scene()->mapId());
        }

        LOG_DEBUG("物死亡复活, 原始点:posX = {}, posY = {}, 当前点:posX = {}, posY= {} ",
                  m_originalPos.x,m_originalPos.y,pos().x,pos().y);
    }   
}

void Npc::dropObj(PK::Ptr atk)
{
    LOG_DEBUG("怪物死亡物品掉落, 共掉落{}种物品, 本怪物物品归属时间:{}", 
              m_tpl->objDropInfos.size(), m_tpl->timeOfBelongTo);

    Scene::Ptr s = scene();
    if(s == nullptr)
        return;
    if(nullptr == atk)
        return;
    Role::Ptr role = getRole(atk);
    if(nullptr == role)
        return;

	std::vector<RoleId> ownerVec;
    if(role->teamId() != 0)//有队伍
    {
        ownerVec = RelationsManager::me().getMemberVecByTeamId(role->teamId());
        if(ownerVec.size() == 0)
        {
            LOG_DEBUG("怪物死亡物品掉落, 队员得到了空的队员列表, roleId={}, teamId={}",
                      role->id(), role->teamId());
            ownerVec.push_back(role->id());//出现这种情况即role有队伍，但是队伍信息没有同步到当前world，逻辑错误
        }
    }
    else
	{
		ownerVec.push_back(role->id());
	}

	std::vector<SceneObject::Ptr> allObjsToScene;
    //基础掉落
    water::componet::Random<uint16_t> rand(1, 10000);
    for(NpcTpl::ObjDropInfo& objDropInfo : m_tpl->objDropInfos)
    {
        if(rand.get() >= objDropInfo.probability)
            continue;
        componet::TimePoint time = componet::EPOCH;
        if(m_tpl->timeOfBelongTo != 0)
            time = componet::Clock::now() + std::chrono::seconds(m_tpl->timeOfBelongTo);
        //获得物品最大叠加数
        const auto& objMap = ObjectConfig::me().objectCfg.m_objBasicDataMap;
        auto it = objMap.find(objDropInfo.objId);
        if(it == objMap.end())
        {
            continue;
        }
        uint16_t m = SAFE_DIV(objDropInfo.num, it->second.maxStackNum);
        uint16_t n = SAFE_MOD(objDropInfo.num, it->second.maxStackNum);
        for(uint16_t i = 0; i < m; i++)
        {
            SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(objDropInfo.objId, it->second.maxStackNum, objDropInfo.isOwnByOne, ownerVec, time);
            if(sceneObj == nullptr)
                continue;
            allObjsToScene.push_back(sceneObj);
        }
        if(n > 0)
        {
            SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(objDropInfo.objId, n, objDropInfo.isOwnByOne, ownerVec, time);
            if(sceneObj == nullptr)
                continue;
            allObjsToScene.push_back(sceneObj);
        }

    }
    //特殊掉落(走随机奖励机制)
	std::vector<ObjItem> specialObjects;
	if(RewardManager::me().getRandomReward(m_tpl->rewardRandomId, 1, role->level(), role->job(), specialObjects))
	{
		for(auto& rewardData : specialObjects)
		{
			componet::TimePoint time = componet::EPOCH;
			if(m_tpl->timeOfBelongTo != 0)
				time = componet::Clock::now() + std::chrono::seconds(m_tpl->timeOfBelongTo);
			//获得物品最大叠加数
			const auto& objMap = ObjectConfig::me().objectCfg.m_objBasicDataMap;
			auto it = objMap.find(rewardData.tplId);
			if(it == objMap.end())
			{
				continue;
			}
			uint16_t m = SAFE_DIV(rewardData.num, it->second.maxStackNum);
			uint16_t n = SAFE_MOD(rewardData.num, it->second.maxStackNum);
			for(uint16_t i = 0; i < m; i++)
			{
				SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(rewardData.tplId, it->second.maxStackNum, rewardData.bind, ownerVec, time);
				if(sceneObj == nullptr)
					continue;
				allObjsToScene.push_back(sceneObj);
			}
			if(n > 0)
			{
				SceneObject::Ptr sceneObj = SceneObjectManager::me().createObj(rewardData.tplId, n, rewardData.bind, ownerVec, time);
				if(sceneObj == nullptr)
					continue;
				allObjsToScene.push_back(sceneObj);
			}
		}
	}
    s->addSceneObj(pos(), allObjsToScene, role->name(), name(), true);

/***掉落物品广播，不做通用，要求根据自己模块自己广播(可能有的物品在不同的怪物身上掉落不一定广播)，待提出单独功能函数***/

    //野外boss
    if(type() == NpcType::boss && scene()->mapTpl()->type == CopyMap::none)
    {
        std::vector<uint32_t> objTplIdVec;
        for(auto& it : allObjsToScene)
        {
            if(it->broadCast() != BroadCast::none)
                objTplIdVec.push_back(it->tplId());
        }

        FieldBossManager::me().toDie(tplId(), scene()->mapId(), objTplIdVec, role);
    }
    //个人boss
    if(scene()->mapTpl()->type == CopyMap::private_boss)
    {
        std::vector<uint32_t> objTplIdVec;
        for(auto& it : allObjsToScene)
        {
            if(it->broadCast() != BroadCast::none)
            {
                if(it->broadCast() != BroadCast::none)
                    objTplIdVec.push_back(it->tplId());
            }
        }

        role->m_privateBoss.npcDie(tplId(), objTplIdVec);
    }
    //boss之家
    if(scene()->mapTpl()->type == CopyMap::boss_home)
    {
        std::vector<uint32_t> objTplIdVec;
        for(auto& it : allObjsToScene)
        {
            if(it->broadCast() != BroadCast::none)
            {
                if(it->broadCast() != BroadCast::none)
                    objTplIdVec.push_back(it->tplId());
            }
        }

        BossHomeManager::me().toDie(tplId(), scene()->mapId(), objTplIdVec, role);
    }

}

void Npc::setOriginalPos(Coord2D pos)
{
    if((m_originalPos.x == 0) && (m_originalPos.y == 0))
    {
        m_originalPos = pos;
    }
}

const Coord2D& Npc::originalPos() const
{
    return m_originalPos;
}

void Npc::setHomePos(Coord2D pos)
{
    m_homePos = pos;
}

const Coord2D& Npc::homePos() const
{
    if((m_homePos.x == 0) && (m_homePos.y == 0))
        return m_originalPos;
    return m_homePos;
}

void Npc::timerLoop(StdInterval interval, const componet::TimePoint& now)
{
    if(needErase())
        return;
//    LOG_DEBUG("npc::timerloop::{}", interval);
    PK::timerLoop(interval, now);

    {//ai的定时器事件
        ai::TimerEmit aiEvent;
        aiEvent.interval = interval;
        aiEvent.now = now;
        handleAIEvent(&aiEvent);
    }

    switch(interval)
    {
    case StdInterval::msec_100:
        followThePath(now);
        break;
    case StdInterval::msec_500:
        {//npc死亡
            if(isDead())
            {   
                checkDealCorpseAndrelive(now);
            }   
        }   
        break;
    case StdInterval::sec_1:
        break;
    default:
        break;
    }
}

bool Npc::changePos(Coord2D newPos, componet::Direction dir, MoveType type)
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return false;

    Coord2D oldPos = pos();
    if(type == MoveType::hitback && oldPos == newPos)
        return false;

    Grid* newGrid = s->getGridByGridCoord(newPos);
    if(newGrid == nullptr)
        return false;

    Grid* oldGrid = s->getGridByGridCoord(oldPos);
    if(oldGrid == nullptr)//不可能
        return false;

    Screen* newScreen = s->getScreenByGridCoord(newPos);
    if(newScreen == nullptr)
        return false;

    Screen* oldScreen = s->getScreenByGridCoord(oldPos);
    if(oldScreen == nullptr)
        return false;

    //距离过远
    bool legalMove = false;
    if(type == MoveType::walk)
    {
        legalMove = (oldPos.x == newPos.x || std::abs(oldPos.x - newPos.x) == 1) && (oldPos.y - newPos.y == 0 || std::abs(oldPos.y - newPos.y) == 1);
    }
    else if(type == MoveType::run)
    {
        legalMove = (oldPos.x == newPos.x || std::abs(oldPos.x - newPos.x) == 2) && (oldPos.y - newPos.y == 0 || std::abs(oldPos.y - newPos.y) == 2);
        if(legalMove)
        {
            Grid* middleGrid = s->getGridByGridCoord(Coord2D((oldPos.x + newPos.x) / 2, (oldPos.y + newPos.y) / 2));
            if(middleGrid == nullptr || !middleGrid->enterable(SceneItemType::npc))
                legalMove = false;
        }
    }
    else if(type == MoveType::chongfeng || type == MoveType::hitback)
    {
        legalMove = true;
    }

    if(!legalMove)
    {
        return false;
    }

    /*
     * 判断是否走完了上一格, 防加速外挂
     */

    //从老格子到新格子
    Npc::Ptr me = std::static_pointer_cast<Npc>(shared_from_this());;
    if(!newGrid->addNpc(me))
    {
        //LOG_DEBUG("npc:{}, 目标格子不可进入, changePos {}->{}", *this, oldPos, newPos);
        return false;
    }
    //发生了屏切换, 有离开和进入视野问题要解决
    if(newScreen != oldScreen)
    {
        if(!newScreen->addNpc(me))
        {
            LOG_DEBUG("npc:{}, 目标屏不可进入, changePos {}->{}", *this, oldPos, newPos);
            newGrid->eraseNpc(me);
            return false;
        }
        oldScreen->eraseNpc(me);

        //已序的新老9屏
        std::vector<Screen*> old9Screens = s->get9ScreensByGridCoord(oldPos);
        std::sort(old9Screens.begin(), old9Screens.end());
        std::vector<Screen*> new9Screens = s->get9ScreensByGridCoord(newPos);
        std::sort(new9Screens.begin(), new9Screens.end());
        //脱离视野
        std::vector<Screen*> leavedScreens;
        std::set_difference(old9Screens.begin(), old9Screens.end(),
                            new9Screens.begin(), new9Screens.end(),
                            std::back_inserter(leavedScreens));
        leaveVisualScreens(leavedScreens);
        //进入视野
        std::vector<Screen*> enteredScreens;
        std::set_difference(new9Screens.begin(), new9Screens.end(),
                            old9Screens.begin(), old9Screens.end(),
                            std::back_inserter(enteredScreens));
        enterVisualScreens(enteredScreens);
    }
    oldGrid->eraseNpc(me);
    setPos(newPos);

    setDir(dir);

    if(MoveType::chongfeng == type || MoveType::hitback == type)
        return true;
    //发送新坐标给9屏内的玩家
    syncNewPosTo9Screens(type);

    {//位置改变的ai事件
        //AIPosChanged event;
        //event.oldPos = oldPos;
        //event.newPos = newPos;
        //handleAIEvent(&event);
    }

    return true;
}

void Npc::syncNewPosTo9Screens(MoveType type) const
{
    Scene::Ptr s = scene();
    if(s == nullptr)
        return;

    //LOG_DEBUG("npc syncpos {}", pos());

    PublicRaw::UpdateNpcPosToClient send;
    send.npcId = id();
    send.posX = pos().x;
    send.posY = pos().y;
    send.dir = static_cast<decltype(send.dir)>(dir());
    send.type = type;
    std::vector<Screen*> screens = s->get9ScreensByGridCoord(pos());
    s->sendToScreens(screens, RAWMSG_CODE_PUBLIC(UpdateNpcPosToClient), &send, sizeof(send));
}

void Npc::releaseSkill(int32_t skillTplId, const Coord2D& pos)
{
    componet::TimePoint now = componet::Clock::now();
    if(m_moveData.lastStepDone > now) //正在做走的动作, 不能攻击
        return;

    launchAttack(skillTplId, pos);
    m_moveData.lastStepDone = now;  //攻击时不移动
}

void Npc::setCollectRoleId(PKId id)
{
    m_collectRoleId = id;
    if(id > 0)
        m_collectTp = Clock::now();
    else
        m_collectTp = EPOCH;
}

PKId Npc::collectRoleId() const
{
    if(m_collectRoleId > 0 && m_collectTp + std::chrono::seconds{10} < Clock::now())
        return 0;
    return m_collectRoleId;
}


/*
 * 生命上限
 */
uint32_t Npc::getMaxHp() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(maxhp);
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxhp, ret);
    return ret;
}


/*
 * 魔法上限
 */
uint32_t Npc::getMaxMp() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(maxmp);
    ret = m_buffM.getHpMpBuffConstProp(PropertyType::maxmp, ret);
    return ret;
}


/*
 * 最小物攻
 */
uint32_t Npc::getTotalPAtkMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_attackMin);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMin, ret);
    return ret;
}


/*
 * 最大物攻
 */
uint32_t Npc::getTotalPAtkMax() const 
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_attackMax);
    ret = m_buffM.getBuffProps(PropertyType::p_attackMax, ret);
    return ret;
}


/*
 * 最小魔攻
 */
uint32_t Npc::getTotalMAtkMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_attackMin);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMin, ret);
    return ret;
}


/*
 * 最大魔攻
 */
uint32_t Npc::getTotalMAtkMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_attackMax);
    ret = m_buffM.getBuffProps(PropertyType::m_attackMax, ret);
    return ret;
}


/*
 * 最小道术
 */
uint32_t Npc::getTotalWitchMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(witchMin);
    ret = m_buffM.getBuffProps(PropertyType::witchMin, ret);
    return ret;
}


/*
 * 最大道术
 */
uint32_t Npc::getTotalWitchMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(witchMax);
    ret = m_buffM.getBuffProps(PropertyType::witchMax, ret);
    return ret;
}


/*
 * min物防
 */
uint32_t Npc::getTotalPDefMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_defenceMin);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMin, ret);
    return ret;
}


/*
 * max物防
 */
uint32_t Npc::getTotalPDefMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_defenceMax);
    ret = m_buffM.getBuffProps(PropertyType::p_defenceMax, ret);
    return ret;
}


/*
 * min魔防
 */
uint32_t Npc::getTotalMDefMin() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_defenceMin);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMin, ret);
    return ret;
}


/*
 * max魔防
 */
uint32_t Npc::getTotalMDefMax() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_defenceMax);
    ret = m_buffM.getBuffProps(PropertyType::m_defenceMax, ret);
    return ret;
}


/*
 * 幸运值
 */
uint32_t Npc::getTotalLucky() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(lucky);
    ret = m_buffM.getBuffProps(PropertyType::lucky, ret);
    return ret;
}


/*
 * 诅咒值
 */
uint32_t Npc::getTotalEvil() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(evil);
    ret = m_buffM.getBuffProps(PropertyType::evil, ret);
    return ret;
}


/*
 * 命中
 */
uint32_t Npc::getTotalShot() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(shot);
    ret = m_buffM.getBuffProps(PropertyType::shot, ret);
    return ret;
}


/*
 * 命中率
 */
uint32_t Npc::getTotalShotRatio() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(shotRatio);
    ret = m_buffM.getBuffProps(PropertyType::shotRatio, ret);
    return ret;
}


/*
 * 物闪
 */
uint32_t Npc::getTotalPEscape() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(p_escape);
    ret = m_buffM.getBuffProps(PropertyType::p_escape, ret);
    return ret;
}


/*
 * 魔闪
 */
uint32_t Npc::getTotalMEscape() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(m_escape);
    ret = m_buffM.getBuffProps(PropertyType::m_escape, ret);
    return ret;
}


/*
 * 闪避率
 */
uint32_t Npc::getTotalEscapeRatio() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(escapeRatio);
    ret = m_buffM.getBuffProps(PropertyType::escapeRatio, ret);
    return ret;
}


/*
 * 暴击
 */
uint32_t Npc::getTotalCrit() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(crit);
    ret = m_buffM.getBuffProps(PropertyType::crit, ret);
    return ret;
}


/*
 * 暴击率
 */
uint32_t Npc::getTotalCritRatio() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(critRatio);
    ret = m_buffM.getBuffProps(PropertyType::critRatio, ret);
    return ret;
}


/*
 * 坚韧
 */
uint32_t Npc::getTotalAntiCrit() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(antiCrit);
    ret = m_buffM.getBuffProps(PropertyType::antiCrit, ret);
    return ret;
}


/*
 * 暴伤
 */
uint32_t Npc::getTotalCritDmg() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(critDamage);
    ret = m_buffM.getBuffProps(PropertyType::critDamage, ret);
    return ret;
}


/*
 * 增伤
 */
uint32_t Npc::getTotalDmgAdd() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(damageAdd);
    ret = m_buffM.getBuffProps(PropertyType::damageAdd, ret);
    return ret;
}


/*
 * 减伤
 */
uint32_t Npc::getTotalDmgReduce() const
{
    uint32_t ret = 0;
    ret += GET_ATTRIBUTE(damageReduce);
    ret = m_buffM.getBuffProps(PropertyType::damageReduce, ret);
    return ret;
}

}
