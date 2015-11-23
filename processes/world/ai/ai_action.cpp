#include "ai_action.h"

#include "ai_event.h"

#include "scene.h"
#include "npc.h"

#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"
#include "water/componet/random.h"


#include <queue>


namespace world{
namespace ai{

void AIAction::setId(char id)
{
    m_id = id;
}

char AIAction::id() const
{
    return m_id;
}

/*********************************************/
AIAction::Ptr IdelMove::clone() const
{
    auto ret = std::make_shared<IdelMove>();
    *ret = *this;
    return ret;
}

void IdelMove::exec(Npc* npc, const AIEvent* event)
{
    npc->tryIdelMove();
}

/*********************************************/


bool MobAction::parseCfg(XmlParseNode& node)
{
    m_idelViewRange   = node.getAttr<int16_t>("idelViewRange");
//    m_idelViewRange   = node.getAttr<int16_t>("guardRange");
    m_combatViewRange = node.getAttr<int16_t>("combatViewRange");
    m_attRange        = node.getAttr<int16_t>("attRange");
    m_homeless        = node.getAttr<bool>("homeless");
    return true;
}

void MobAction::exec(Npc* npc, const AIEvent* event)
{
    if(event->type != EventType::timerEmit)
        return;

    auto scene = npc->scene();
    if(scene == nullptr)
        return;

    const std::chrono::seconds m_forgetDruation(5);

    auto timerEmit = reinterpret_cast<const TimerEmit*>(event);
    auto distCheck = [](Coord2D pos1, Coord2D pos2, Coord1D dist) -> bool
    {
        return std::abs(pos1.x - pos2.x) <= dist && std::abs(pos1.y - pos2.y) <= dist;
    };

    const Coord2D pos = npc->pos();
    AIData& aiData = npc->aiData();
    
    //主动攻击, 且当前未锁定敌人
    if(!aiData.backToHome && m_idelViewRange > 0)
    {
        if(aiData.target.enemy.expired() ||
           aiData.target.type == SceneItemType::none ||
           aiData.target.lastHarmTime + m_forgetDruation < timerEmit->now)
        {
            aiData.target.enemy.reset();
            std::priority_queue<std::pair<int32_t, PK::Ptr>, std::vector<std::pair<int32_t, PK::Ptr>>, std::greater<std::pair<int32_t, PK::Ptr>>> sceneItems;
            auto screens = scene->get9ScreensByGridCoord(npc->pos());
            for(auto screen : screens)
            {
                for(auto& item : screen->roles())
                {
                    auto role = item.second;
                    if(!distCheck(role->pos(), pos, m_idelViewRange))
                        continue;
                    if(role->m_pkstate.issetStatus(visual_status::hide))
                        continue;

                    sceneItems.push({role->pos().manhattanDistance(pos), role});
                }
                for(auto& item : screen->heros())
                {
                    auto hero = item.second;
                    if(!distCheck(hero->pos(), pos, m_idelViewRange))
                        continue;
                    if(hero->m_pkstate.issetStatus(visual_status::hide))
                        continue;

                    sceneItems.push({hero->pos().manhattanDistance(pos), hero});
                }
                for(auto& item : screen->pets())
                {
                    auto pet = item.second;
                    if(!distCheck(pet->pos(), pos, m_idelViewRange))
                        continue;
                    if(pet->m_pkstate.issetStatus(visual_status::hide))
                        continue;

                    sceneItems.push({pet->pos().manhattanDistance(pos), pet});
                }
            }

            while(!sceneItems.empty())
            {
                PK::Ptr pk = sceneItems.top().second;
                sceneItems.pop();
                //if(pk 可被 npc 攻击)
                {
                    aiData.target.enemy = pk;
                    if(pk->sceneItemType() == SceneItemType::role)
                        aiData.target.type  = SceneItemType::role;
                    if(pk->sceneItemType() == SceneItemType::hero)
                        aiData.target.type  = SceneItemType::hero;
                    if(pk->sceneItemType() == SceneItemType::pet)
                        aiData.target.type  = SceneItemType::pet;
                    aiData.target.lastHarmTime = timerEmit->now;
                    break;
                }
            }
        }
    }

    const Coord1D idelRange = 2;

    Coord2D homePos = m_homeless ? pos : npc->homePos();
    PK::Ptr enemy = aiData.target.enemy.lock();
    if(enemy == nullptr ||
       aiData.target.type == SceneItemType::none ||
       aiData.target.lastHarmTime + m_forgetDruation < timerEmit->now)
    {
        if(aiData.backToHome) //正在返回驻地
        {
            if(distCheck(npc->pos(), homePos, idelRange)) //已经回去了
            {
                aiData.backToHome = false;
                npc->setHp(npc->getMaxHp()); //recovery healthy immediately
            }
            else
            {
                //1, quick recovery healthy
                npc->changeHp(npc->getMaxHp() / 20);

                //2, doBackToHome
                npc->moveToNearby(homePos, idelRange);
            }
        }
        return;
    }

    //追丢
    if(enemy->isDead() || !distCheck(pos, enemy->pos(), m_combatViewRange) || !distCheck(pos, homePos, m_idelViewRange))
    {
        aiData.target.enemy.reset();
        aiData.target.type = SceneItemType::none;
        aiData.backToHome = true;
        if(m_homeless)
            npc->setHomePos(npc->pos());
        return;
    }

    if(distCheck(pos, enemy->pos(), m_attRange))
    {//打的到就打
        npc->releaseSkill(npc->tpl().skillTplId, enemy->pos());
    }
    else
    {//打不到就追
        npc->moveToNearby(enemy->pos(), m_attRange);
    }

    return;
}

AIAction::Ptr MobAction::clone() const
{
    auto ret = std::make_shared<MobAction>();
    *ret = *this;
    return ret;
}

/************/

bool Tower::parseCfg(XmlParseNode& node)
{
    m_attRange = node.getAttr<uint16_t>("attRange");
    return true;
}

void Tower::exec(Npc* npc, const AIEvent* event)
{
    if(event->type != EventType::timerEmit)
        return;

    auto scene = npc->scene();
    if(scene == nullptr)
        return;

    const std::chrono::seconds m_forgetDruation(5);

    auto timerEmit = reinterpret_cast<const TimerEmit*>(event);
    auto distCheck = [](Coord2D pos1, Coord2D pos2, Coord1D dist) -> bool
    {
        return std::abs(pos1.x - pos2.x) <= dist && std::abs(pos1.y - pos2.y) <= dist;
    };

    const Coord2D pos = npc->pos();
    AIData& aiData = npc->aiData();

    PK::Ptr enemy = aiData.target.enemy.lock();
    if(enemy == nullptr ||
       aiData.target.type == SceneItemType::none ||
       distCheck(pos, enemy->pos(), m_attRange) ||
       aiData.target.lastHarmTime + m_forgetDruation < timerEmit->now)
    {
        aiData.target.enemy.reset();
        std::priority_queue<std::pair<int32_t, PK::Ptr>, std::vector<std::pair<int32_t, PK::Ptr>>, std::greater<std::pair<int32_t, PK::Ptr>>> sceneItems;
        auto screens = scene->get9ScreensByGridCoord(npc->pos());
        for(auto screen : screens)
        {
            for(auto& item : screen->roles())
            {
                auto role = item.second;
                if(!distCheck(role->pos(), pos, m_attRange))
                    continue;
                if(role->m_pkstate.issetStatus(visual_status::hide))
                    continue;

                sceneItems.push({role->pos().manhattanDistance(pos), role});
            }
            for(auto& item : screen->heros())
            {
                auto hero = item.second;
                if(!distCheck(hero->pos(), pos, m_attRange))
                    continue;
                if(hero->m_pkstate.issetStatus(visual_status::hide))
                    continue;

                sceneItems.push({hero->pos().manhattanDistance(pos), hero});
            }
            for(auto& item : screen->pets())
            {
                auto pet = item.second;
                if(!distCheck(pet->pos(), pos, m_attRange))
                    continue;
                if(pet->m_pkstate.issetStatus(visual_status::hide))
                    continue;

                sceneItems.push({pet->pos().manhattanDistance(pos), pet});
            }
        }

        while(!sceneItems.empty())
        {
            PK::Ptr pk = sceneItems.top().second;
            sceneItems.pop();
          //  if( pk 可被 npc 攻击)
            {
                enemy = pk;
                aiData.target.enemy = pk;
                if(pk->sceneItemType() == SceneItemType::role)
                    aiData.target.type  = SceneItemType::role;
                if(pk->sceneItemType() == SceneItemType::hero)
                    aiData.target.type  = SceneItemType::hero;
                if(pk->sceneItemType() == SceneItemType::pet)
                    aiData.target.type  = SceneItemType::pet;
                aiData.target.lastHarmTime = timerEmit->now;
                break;
            }
        }
    }

    if(enemy == nullptr)
        return;
    npc->releaseSkill(npc->tpl().skillTplId, enemy->pos());
}

AIAction::Ptr Tower::clone() const
{
    auto ret = std::make_shared<Tower>();
    *ret = *this;
    return ret;
}

}}

