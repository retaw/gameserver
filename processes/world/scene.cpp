#include "scene.h"

#include "npc_manager.h"
#include "scene_object_manager.h"
#include "channel.h"

#include "water/componet/logger.h"
#include "water/componet/random.h"
#include "water/componet/astar.h"

#include "protocol/rawmsg/public/object_scene.h"
#include "protocol/rawmsg/public/object_scene.codedef.public.h"

#include "protocol/rawmsg/public/test.h"
#include "protocol/rawmsg/public/test.codedef.public.h"

namespace world{

Scene::Scene(SceneId id, const BasicMapInfo& mapInfo)
    : m_id(id), m_basicInfo(mapInfo)
{
    if(mapInfo.width == 0 || mapInfo.height == 0)
        return;

    m_grids.resize(mapInfo.width);
    for(auto& gridsInOneColumn : m_grids)
        gridsInOneColumn.resize(mapInfo.height);

    Coord2D maxGridCoord(mapInfo.width - 1, mapInfo.height - 1);
    Coord2D maxScreenCoord = gridCoord2ScreenCoord(maxGridCoord);
    m_screens.resize(maxScreenCoord.x + 1);
    for(auto& screenInOneColumn : m_screens)
        screenInOneColumn.resize(maxGridCoord.y + 1);
}

Scene::~Scene()
{
}

SceneId Scene::id() const
{
    return m_id;
}

MapId Scene::mapId() const
{
    return m_basicInfo.id;
}

const std::string& Scene::name() const
{
    return m_basicInfo.name;
}

bool Scene::isDynamic() const
{
    return isDynamicSceneId(id());
}

void Scene::timerExec(StdInterval interval, const TimePoint& now)
{
    using namespace std::chrono;
    switch(interval)
    {
    case StdInterval::min_1:
        {
            //这里需要改为统一机制，所有的动态场景，超过多长时间没人统一销毁...
            if(m_roleInTime != water::componet::EPOCH && mapTpl()->type == CopyMap::private_boss)
            {
                if(m_roles.empty())
                {
                    uint32_t counter = duration_cast<seconds>(now - m_roleInTime).count();
                    if(counter >= 60*3)//3分钟
                    {
                        m_avaliable = false;//此动态场景无效    
                    }
                }
            }
            break;
        }
    case StdInterval::sec_1:
        {
            //privateBoss是否到时间检测
            if(m_createTime != water::componet::EPOCH && mapTpl()->type == CopyMap::private_boss && m_bossId != 0)
            {
                auto privateBossDuration = PrivateBossBase::me().getKeepDuration(m_bossId);
                uint32_t counter = duration_cast<seconds>(now - m_createTime).count();
                if(counter > privateBossDuration && privateBossDuration != 0)
                {
                    for(auto& rolePair : m_roles)
                    {
                        rolePair.second->m_privateBoss.checkTimeOut();
                    }
                }
            }

            break;
        }
    default:
        break;
    }
}

bool Scene::avaliable() const
{
    return m_avaliable;
}

void Scene::cleanup()
{
    //受到卸载指令时调用
}

void Scene::removeSceneItem(SceneItemType type/*=SceneItemType::none*/)
{
    auto removeNpcExec = [this] (Npc::Ptr npc)
    {
        if(nullptr == npc || npc->needErase())
            return;
        npc->beforeLeaveScene();
        npc->markErase(true);
    };

    auto removeSceneObjExec = [this] (SceneObject::Ptr sceneObj)
    {
        if(nullptr == sceneObj || sceneObj->needErase())
            return;
        sceneObj->markErase();
    };

    auto removeTriggerExec = [this] (Trigger::Ptr trigger)
    {
        if(nullptr == trigger || trigger->needErase())
            return;
        trigger->markErase();
    };

    if(type == SceneItemType::none)
    {
        execNpc(removeNpcExec);
        execSceneObj(removeSceneObjExec);
        execTrigger(removeTriggerExec);
    }
    else if(type == SceneItemType::npc)
    {
        execNpc(removeNpcExec);
    }
    else if(type == SceneItemType::object)
    {
        execSceneObj(removeSceneObjExec);
    }
    else if(type == SceneItemType::trigger)
    {
        execTrigger(removeTriggerExec);
    }
}

Grid* Scene::getGridByGridCoord(Coord2D coord)
{
    if(coord.x < 0 || coord.y < 0)
        return nullptr;
    if(coord.ux < m_grids.size() && coord.uy < m_grids[coord.x].size())
        return &(m_grids[coord.x][coord.y]);
    return nullptr;
}

Screen* Scene::getScreenByGridCoord(Coord2D coord)
{
    if(coord.x < 0 || coord.y < 0)
        return nullptr;

    Coord2D screenCoord = gridCoord2ScreenCoord(coord);

    if(screenCoord.ux < m_screens.size() && screenCoord.uy < m_screens[screenCoord.x].size())
        return &(m_screens[screenCoord.x][screenCoord.y]);

    return nullptr;
}

std::vector<Screen*> Scene::get9ScreensByGridCoord(Coord2D coord)
{
    std::vector<Screen*> ret;
    if(coord.x < 0 || coord.y < 0)
        return ret;

    ret.reserve(9);

    Coord2D selfPos = gridCoord2ScreenCoord(coord);
    std::array<Coord2D, 9> allPos = selfPos.meAndAllNeighbors();
    for(Coord2D coord : allPos)
    {
        if(coord.x < 0 || coord.y < 0)
            continue;
        if(coord.ux < m_screens.size() && coord.uy < m_screens[coord.x].size())
            ret.push_back(&m_screens[coord.x][coord.y]);
    }
    return ret;
}

bool Scene::enterable(Coord2D pos, SceneItemType sceneItemType)
{
    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr)
        return false;
    return grid->enterable(sceneItemType);
}

bool Scene::addRole(Role::Ptr role)
{
    //在复活点附近加入
    if(m_relivePos.empty())
    {
        LOG_ERROR("角色进场景, 默认出生点(复活点)不存在, role:{}", *role);
        return false;
    }

    water::componet::Random<decltype(m_relivePos.size())> rander(0, m_relivePos.size() - 1);
    Coord2D pos = m_relivePos[rander.get()];
    Coord1D radius = 8;
    
    return addRole(role, pos, radius);
}

bool Scene::addRole(Role::Ptr role, Coord2D centre, Coord1D radius)
{
    return tryExecSpiral(centre, radius, 
                     [this, role](Coord2D pos) -> bool {return addRole(role, pos);});
}

bool Scene::addRole(Role::Ptr role, Coord2D pos)
{
	if(0 == pos.x && 0 == pos.y)
		pos = randomRelivePos();

    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addRole(role))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addRole(role))
    {
        grid->eraseRole(role);
        return false;
    }

    if(!m_roles.insert(std::make_pair(role->id(), role)))
    {
        grid->eraseRole(role);
        screen->eraseRole(role);
        return false;
    }

    role->setPos(pos);
    role->setSceneId(id());
    return true;
}

void Scene::eraseRole(Role::Ptr role)
{
    Coord2D pos = role->pos();

    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->eraseRole(role);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->eraseRole(role);

    m_roles.erase(role->id());
    role->setSceneId(0);
}

void Scene::addDeadRole(RoleId roleId, Coord2D pos)
{
    Grid* grid = getGridByGridCoord(pos);
    if(nullptr != grid)
        grid->addDeadRole(roleId);
}

void Scene::eraseDeadRole(RoleId roleId, Coord2D pos)
{
    Grid* grid = getGridByGridCoord(pos);
    if(nullptr != grid)
        grid->eraseDeadRole(roleId);
}

Role::Ptr Scene::getRoleById(RoleId roleId) const
{
    auto it = m_roles.find(roleId);
    if(it == m_roles.end())
        return nullptr;
    return it->second;
}

bool Scene::addNpc(Npc::Ptr npc, Coord2D centre, Coord1D radius)
{
    return tryExecSpiral(centre, radius, 
                        [this, npc](Coord2D pos) {return addNpc(npc, pos);});
}

bool Scene::addNpc(Npc::Ptr npc, Coord2D pos)
{
    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addNpc(npc))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addNpc(npc))
    {
        grid->eraseNpc(npc);
        return false;
    }

    if(!m_npcs.insert(std::make_pair(npc->id(), npc)))
    {
        grid->eraseNpc(npc);
        screen->eraseNpc(npc);
        return false;
    }

    npc->setPos(pos);
    npc->setOriginalPos(pos);
    return true;
}

Npc::Ptr Scene::summonNpc(NpcTplId npcTplId, Coord2D centre, Coord1D radius)
{
    auto npc = NpcManager::me().createNpc(npcTplId);
    if(npc == nullptr)
    {
        LOG_DEBUG("限定范围召唤npc createNpc失败, sceneId={}, tplId={}, centre={}", id(), npcTplId, centre);
        return nullptr;
    }

    if(!addNpc(npc, centre, radius))
    {
        NpcManager::me().erase(npc->id());
        LOG_DEBUG("限定范围召唤npc addNpc失败, sceneId={}, tplId={}, centre={}", id(), npcTplId, centre);
        return nullptr;
    }

    npc->setSceneId(id());
    npc->afterEnterScene();
    //LOG_DEBUG("限定范围召唤npc成功, sceneId={}, tplId={}, centre={}", id(), npcTplId, centre);
    return npc;
}

Npc::Ptr Scene::summonNpc(NpcTplId npcTplId, Coord2D pos)
{
    auto npc = NpcManager::me().createNpc(npcTplId);
    if(npc == nullptr)
    {
        LOG_DEBUG("限定格子召唤npc createNpc失败, sceneId={}, tplId={}, pos={}", id(), npcTplId, pos);
        return nullptr;
    }

    if(!addNpc(npc, pos))
    {
        NpcManager::me().erase(npc->id());
        LOG_DEBUG("限定格子召唤npc addNpc失败, sceneId={}, tplId={}, pos={}", id(), npcTplId, pos);
        return nullptr;
    }

    npc->setSceneId(id());
    npc->afterEnterScene();
    //LOG_DEBUG("限定格子召唤npc成功, sceneId={}, tplId={}, pos={}", id(), npcTplId, pos);
    return npc;
}

void Scene::eraseNpc(Npc::Ptr npc)
{
    Coord2D pos = npc->pos();

    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->eraseNpc(npc);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->eraseNpc(npc);

    m_npcs.erase(npc->id());
    //npc->setSceneId(0);
}

Npc::Ptr Scene::getNpcById(NpcId npcId) const
{
    auto it = m_npcs.find(npcId);
    if(it == m_npcs.end())
        return nullptr;
    return it->second;
}

/*
void Scene::broadCastDropObj(const std::string& roleName, SceneObject::Ptr obj, const std::string& npcName)
{
    if(obj->broadCast() == BroadCast::none)
        return;

    switch(obj->broadCast())
    {
    case BroadCast::world :
        Channel::me().sendSysNotifyToGlobal(ChannelType::global, "玩家{}在{}消灭{}掉落了{}", roleName, m_basicInfo.name, obj->name());
    default :
        break;
    }
}
*/
void Scene::addSceneObj(Coord2D center, std::vector<SceneObject::Ptr>& sceneObjList, const std::string& roleName, const std::string& npcName, bool broadCast)
{
    uint32_t length = sceneObjList.size();
    if(0 == length)
        return;
    uint32_t index = 0;
    std::map<std::vector<Screen*>, std::vector<uint8_t>> screensObjectsMap; //key是9屏，value是9屏内中心screen上的物品
    auto addSceneObjExec = [&, this](Coord2D pos) -> bool
    {
        if(index == length)
            return true;
        auto screens = get9ScreensByGridCoord(pos);
        SceneObject::Ptr obj = sceneObjList[index];
        if(addObj(obj, pos))
        {
            auto& bufObjects = screensObjectsMap[screens];
            if(bufObjects.size() == 0)
            {
                bufObjects.reserve(512);
                bufObjects.resize(sizeof(PublicRaw::ObjectsAroundMe) - sizeof(PublicRaw::ObjectScreenData));
            }
            auto it = screensObjectsMap.find(screens);
            bufObjects.resize(it->second.size() + sizeof(PublicRaw::ObjectScreenData));
            auto* msg = reinterpret_cast<PublicRaw::ObjectsAroundMe*>(bufObjects.data());
            obj->fillScreenData(msg->objects + msg->size);
            ++msg->size;
            ++index;
        }
        return false;
    };
    if(!tryExecSpiral(center, 10, addSceneObjExec))
        return;

    for(auto &it : screensObjectsMap)
    {
        sendToScreens(it.first, RAWMSG_CODE_PUBLIC(ObjectsAroundMe), it.second.data(), it.second.size());
    }
}

bool Scene::addObj(SceneObject::Ptr obj, Coord2D centre, Coord1D radius)
{
    return tryExecSpiral(centre, radius, 
                        [this, obj](Coord2D pos) {return addObj(obj, pos);});
}

bool Scene::addObj(SceneObject::Ptr obj, Coord2D pos)
{
    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addObj(obj))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addObj(obj))
    {
        grid->eraseObj(obj);
        return false;
    }

    if(!m_sceneObjs.insert(std::make_pair(obj->objId(), obj)))
    {
        grid->eraseObj(obj);
        screen->eraseObj(obj);
        return false;
    }

    obj->setPos(pos);
    obj->setSceneId(id());
    LOG_TRACE("场景物品, 添加, 成功, objId={}, tplId={}, item={}, pos={}", 
			  obj->objId(), obj->tplId(), obj->item(),  pos);

    return true;
}

void Scene::eraseObj(ObjectId objId)
{
	SceneObject::Ptr obj = getObjById(objId);
	if(obj == nullptr)
		return;

	eraseObj(obj);
}

void Scene::eraseObj(SceneObject::Ptr obj)
{
	if(obj == nullptr)
		return;

    Coord2D pos = obj->pos();

    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->eraseObj(obj);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->eraseObj(obj);

    m_sceneObjs.erase(obj->objId());
    obj->setSceneId(0);
    LOG_TRACE("场景物品, 删除, 成功, objId={}, tplId={}, item={}, pos={}", 
			  obj->objId(), obj->tplId(), obj->item(),  pos);
}

SceneObject::Ptr Scene::getObjById(ObjectId objId) const
{
	auto pos = m_sceneObjs.find(objId);
	if(pos == m_sceneObjs.end())
		return nullptr;

	return pos->second;
}

bool Scene::addHero(Hero::Ptr hero, Coord2D centre, Coord1D radius)
{
    return tryExecSpiral(centre, radius, 
                        [this, hero](Coord2D pos) {return addHero(hero, pos);});
}

bool Scene::addHero(Hero::Ptr hero, Coord2D pos)
{
	if(hero == nullptr)
		return false;

    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addHero(hero))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addHero(hero))
    {
        grid->eraseHero(hero);
        return false;
    }

    if(!m_heros.insert(std::make_pair(hero->id(), hero)))
    {
        grid->eraseHero(hero);
        screen->eraseHero(hero);
        return false;
    }

    hero->setPos(pos);
    hero->setSceneId(id());

	Role::Ptr role = hero->getOwner();
    if(role == nullptr)
		return false;

	LOG_TRACE("场景英雄, 添加, 成功, heroId={}, job={}, pos={}, owner=({},{},{})", 
			  hero->id(), hero->job(), pos, role->name(), role->id(), role->account());
    return true;
}

void Scene::eraseHero(HeroId heroId)
{
	Hero::Ptr hero = getHeroById(heroId);
	if(hero == nullptr)
		return;

	eraseHero(hero);
}

void Scene::eraseHero(Hero::Ptr hero)
{
	if(hero == nullptr)
		return;

    Coord2D pos = hero->pos();

    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->eraseHero(hero);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->eraseHero(hero);

    m_heros.erase(hero->id());
    hero->setSceneId(0);
	
	Role::Ptr role = hero->getOwner();
    if(role == nullptr)
		return;

    LOG_TRACE("场景英雄, 删除, 成功, objId={}, job={}, pos={}, owner=({}, {}, {})", 
			  hero->id(), hero->job(), pos, role->name(), role->id(), role->account());
}

Hero::Ptr Scene::getHeroById(HeroId heroId) const
{
	auto pos = m_heros.find(heroId);
	if(pos == m_heros.end())
		return nullptr;

	return pos->second;
}

bool Scene::addPet(Pet::Ptr pet, Coord2D center, Coord1D radius)
{
    return tryExecSpiral(center, radius, 
                        [&](Coord2D pos) {return center == pos ? false : addPet(pet, pos);});
}

bool Scene::addPet(Pet::Ptr pet, Coord2D pos)
{
	if(pet == nullptr)
		return false;

    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addPet(pet))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addPet(pet))
    {
        grid->erasePet(pet);
        return false;
    }

    if(!m_pets.insert(std::make_pair(pet->id(), pet)))
    {
        grid->erasePet(pet);
        screen->erasePet(pet);
        return false;
    }

    pet->setPos(pos);
    pet->setSceneId(id());
    return true;
}

void Scene::erasePet(PKId petId)
{
    auto pet = getPetById(petId);
    if(nullptr == pet)
        return;

    erasePet(pet);
}

void Scene::erasePet(Pet::Ptr pet)
{
	if(nullptr == pet)
		return;

    Coord2D pos = pet->pos();

    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->erasePet(pet);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->erasePet(pet);

    m_pets.erase(pet->id());
    pet->setSceneId(0);
}

Pet::Ptr Scene::getPetById(PKId petId) const
{
    auto it = m_pets.find(petId);
    if(it == m_pets.end())
        return nullptr;

    return it->second;
}

bool Scene::addFire(Fire::Ptr fire, Coord2D pos)
{
	if(fire == nullptr)
		return false;

    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addFire(fire))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addFire(fire))
    {
        grid->eraseFire(fire);
        return false;
    }

    if(!m_fires.insert(std::make_pair(fire->id(), fire)))
    {
        grid->eraseFire(fire);
        screen->eraseFire(fire);
        return false;
    }

    fire->setPos(pos);
    fire->setSceneId(id());
    return true;
}

void Scene::eraseFire(Fire::Ptr fire)
{
	if(fire == nullptr)
		return;

    Coord2D pos = fire->pos();
    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->eraseFire(fire);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->eraseFire(fire);

    m_fires.erase(fire->id());
    fire->setSceneId(0);
}

bool Scene::tryExecSpiral(Coord2D centre, Coord1D radius, std::function<bool (Coord2D pos)> exec)
{
    //先看起点
    if(exec(centre))
        return true;

    //只有一个点视为边长长为0, 则边长为:
    const int32_t sideLength = radius * 2 + 1;

    //螺旋线遍历, 起始点除外, 即实际起步从1开始计数
    //则, 步长的通项公式length = (step + 1) / 2, 其中除法为 下取整
    //方向按  上->右->下->左->...  的顺序沿着螺旋线遍历区域
    const uint32_t dirSize = 4;
    Direction dirs[dirSize] = {Direction::up, Direction::right, Direction::down, Direction::left};


    Coord2D lastPos = centre;
    for(Coord1D step = 1; true; ++step)
    {
        const Coord1D stepLength = (step + 1) / 2;
        Direction dir = dirs[step % dirSize];
        
        for(Coord1D i = 1; i <= stepLength; ++i)
        {
            //下个格子的坐标, lastPos  dir方向的邻居
            Coord2D pos = lastPos.neighbor(dir);
            if(exec(pos))
                return true;

            lastPos = pos;
        }
        if(stepLength >= sideLength)
            break;
    }

    return false;
}

/*
 * 
 */
bool Scene::execCircle(Coord2D start, Coord1D radius, std::function<bool (Coord2D)> exec)
{
    //边长 (radius * 2) + 1;
    const int32_t sideLength = radius * 2 + 1;

    //环形遍历 4步就okay 方向  右->下->左->上
    const uint32_t dirSize = 4;
    Direction dirs[dirSize] = {Direction::up, Direction::right, Direction::down, Direction::left};

    Coord2D lastPos = start;
    for(Coord1D step = 1; step <= 4; ++step)
    {
        auto dir = dirs[step % dirSize];
        for(Coord1D side = 1; side < sideLength; ++side)
        {
            Coord2D pos = lastPos.neighbor(dir);
            if(exec(pos))
                return true;

            lastPos = pos;
        }
    }
    return false;
}

bool Scene::tryExecAreaByCircle(Coord2D centre, Coord1D radius, std::function<bool (Coord2D pos)> exec)
{
    if(exec(centre))
        return true;

    //重置起始点方向 左上
    const Direction resetDir = Direction::leftup;
    Coord2D startPos = centre;
    for(Coord1D i = 0; i < radius; ++i)
    {
        //重置起始坐标点
        startPos = startPos.neighbor(resetDir);
        if(execCircle(startPos, radius, exec))
            return true;
    }
    return false;
}


Coord2D Scene::gridCoord2ScreenCoord(Coord2D coord)
{
    return Coord2D(coord.x / Screen::WIDTH, coord.y / Screen::HEIGHT);
}

void Scene::setMapBlock(Coord2D coord)
{
    Grid* grid = getGridByGridCoord(coord);
    if(grid == nullptr)
        return;
    grid->setBlock();
}

void Scene::setMapBlocks(const std::vector<Coord2D>& blocks)
{
    for(const Coord2D& coord : blocks)
        setMapBlock(coord);
}

void Scene::summonStaticNpcs(NpcTplId npcTplId, uint32_t totalNum, const std::vector<Coord2D>& area)
{
#ifdef WATER_DEBUG_NPC_AI
    return;
#endif
    if(area.empty())
        return;

    componet::Random<uint32_t> rand(0, area.size() - 1);
    for(uint32_t i = 0; i < totalNum; ++i)
    {
        summonNpc(npcTplId, area[rand.get()], 10);
    }
}

void Scene::setReliveArea(const std::vector<Coord2D>& area)
{
    m_relivePos = area;
}

bool Scene::existReliveArea() const
{
    return !m_relivePos.empty();
}

void Scene::setMapTpl(MapId mapid)
{
    m_mapTpl = MapBase::me().getMapTpl(mapid);
}

uint32_t Scene::minRoleLevel() const
{
    return m_mapTpl->roleMinLevel;
}

MapTpl::Ptr Scene::mapTpl() const
{
    return m_mapTpl;
}

attack_mode Scene::mapAttackMode() const
{
    return nullptr == m_mapTpl ? attack_mode::none : m_mapTpl->mode;
}

uint8_t Scene::crime() const
{
    return nullptr == m_mapTpl ? 0 : m_mapTpl->crime;
}

uint8_t Scene::dropEquip() const
{
    return nullptr == m_mapTpl ? 0 : m_mapTpl->dropEquip;
}

CopyMap Scene::copyType() const
{
    return nullptr == m_mapTpl ? CopyMap::none : m_mapTpl->type;
}

bool Scene::canRide() const
{
    if(nullptr == m_mapTpl)
        return false;
    return 0 == m_mapTpl->ride ? true : false;
}

uint16_t Scene::reliveYB() const
{
    return nullptr == m_mapTpl ? 0 : m_mapTpl->reliveyb;
}

Coord2D Scene::randAvailablePos(SceneItemType sceneItemType)
{
    Coord2D dest(0,0);
    Coord2D randCenter;
    water::componet::Random<decltype(m_grids.size())> rander_x(0, m_grids.size() - 1);
    randCenter.x = rander_x.get();
    water::componet::Random<decltype(m_grids.size())> rander_y(0, m_grids[dest.x].size() - 1);
    randCenter.y = rander_y.get();

    auto randPosExec = [&, this](Coord2D pos) -> bool
    {
        if(enterable(pos, sceneItemType))
        {
            dest = pos;
            return true;
        }
        return false;
    };

    tryExecAreaByCircle(randCenter, 20, randPosExec);
    return dest;
}

Coord2D Scene::randomRelivePos()
{
    if(m_relivePos.empty())
    {
        LOG_ERROR("复活点, 配置错误, 随机失败, m_relivePos={}", m_relivePos.size());
        return Coord2D(0,0);
    }

    water::componet::Random<decltype(m_relivePos.size())> rander(0, m_relivePos.size() - 1);
    Coord2D pos = m_relivePos[rander.get()];
	return pos;
}

const std::map<int32_t, Scene::Transmission>& Scene::transmissions() const
{
    return m_transmissions;
}

void Scene::setTransmissions(const std::map<int32_t, Transmission>& transmissions)
{
    m_transmissions = transmissions;
}

std::list<Coord2D> Scene::findPath(Coord2D start, Coord2D goal, SceneItemType sceneItemType)
{
    struct Nighbors
    {
        std::array<Coord2D, 8> operator()(const Coord2D& pos) const
        {   
            return pos.allNeighbors();
        }   
    };

    struct Heruistic
    {
        int32_t operator() (const Coord2D& pos, const Coord2D& goal) const
        {   
            return 10 * (std::abs(pos.x - goal.x) + std::abs(pos.y - goal.y));
        }   
    };

    struct StepCost
    {
        int32_t operator()(const Coord2D& from, const Coord2D& to) const
        {   
            if(from.x == to.x || from.y == to.y)
                return 10; 
            return 15; 
        }   
    };

    struct Enterable
    {
        bool operator()(const Coord2D& pos) const
        {
            return scene->enterable(pos, sceneItemType);
        }
        Scene* scene;
        SceneItemType sceneItemType;
    };

    water::componet::AStar<Coord2D, Nighbors, Enterable, StepCost, Heruistic> astar;
    astar.enterable().scene = this;
    astar.enterable().sceneItemType = sceneItemType;
    return astar.findPath(start, goal);
}

bool Scene::sendCmdTo9(TcpMsgCode msgCode, const void* msg, uint32_t msgSize, Coord2D pos)
{
	std::vector<Screen*> screens = get9ScreensByGridCoord(pos);
	sendToScreens(screens, msgCode, msg, msgSize);
	return true;
}

void Scene::sendToScreens(const std::vector<Screen*>& screens, TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const
{
	for(Screen* screen : screens)
	{
		if(msgCode == RAWMSG_CODE_PUBLIC(ObjectLeaveInfo))
		{
			auto rev = reinterpret_cast<const PublicRaw::ObjectLeaveInfo*>(msg);
			if(!rev)
				continue;

			for(ArraySize i = 0 ; i < rev->size; i++)
			{
				LOG_TRACE("场景物品, 离开九屏, s->c, 发给九屏, 成功, objId={}, i={}, size={}, screen={}",
						  rev->objId[i], i, rev->size, screen);
			}
		}

		if(msgCode == RAWMSG_CODE_PUBLIC(ObjectsAroundMe))
		{
			auto rev = reinterpret_cast<const PublicRaw::ObjectsAroundMe*>(msg);
			if(!rev)
				continue;

			for(ArraySize i = 0; i < rev->size; i++)
			{
				LOG_TRACE("场景物品, 进入九屏, s->c, 发给九屏, 成功, objId={}, tplId={}, pos=({},{}), i={}, size={}, screen={}",
						  rev->objects[i].objId, rev->objects[i].tplId, rev->objects[i].posX,
						  rev->objects[i].posY, i, rev->size, screen);
			}
		}

		for(auto iter = screen->roles().begin(); iter != screen->roles().end(); ++iter)
		{
			PK::Ptr role = iter->second;
			if(nullptr == role)
				continue;
			role->sendToMe(msgCode, msg, msgSize);
		}
	}
}

void Scene::setArea(const std::vector<Coord2D>& coordVec, AreaType type)
{
    for(const Coord2D& coord : coordVec)
        setArea(coord, type);
}

void Scene::setArea(Coord2D coord, AreaType type)
{
    Grid* grid = getGridByGridCoord(coord);
    if(grid == nullptr)
        return;

    grid->setArea(type);
}

void Scene::unsetArea(Coord2D coord, AreaType type)
{
	Grid* grid = getGridByGridCoord(coord);
	if(grid == nullptr)
		return;

	grid->unsetArea(type);
}

bool Scene::isArea(Coord2D coord, AreaType type)
{
	Grid* grid = getGridByGridCoord(coord);
	if(grid == nullptr)
		return false;

	return grid->isArea(type);
}

void Scene::execRole(std::function<void (Role::Ptr)> exec)
{
    for(auto& iter : m_roles)
        exec(iter.second);
}

void Scene::execNpc(std::function<void (Npc::Ptr)> exec)
{
    for(auto& iter : m_npcs)
        exec(iter.second);
}

void Scene::execSceneObj(std::function<void (SceneObject::Ptr)> exec)
{
    for(auto& iter : m_sceneObjs)
        exec(iter.second);
}

void Scene::execTrigger(std::function<void (Trigger::Ptr)> exec)
{
    for(auto& iter : m_triggers)
        exec(iter.second);
}

Trigger::Ptr Scene::summonTrigger(uint32_t triggerTplId, Coord2D center, uint16_t radius)
{
    TriggerTpl::Ptr triggerTpl = TriggerCfg::me().getById(triggerTplId);
    if(nullptr == triggerTpl)
    {
        LOG_ERROR("找不到机关对应的配置表, tplId:{}", triggerTplId);
        return nullptr;
    }

    Trigger::Ptr trigger = nullptr;
    auto summonExec = [&, this] (Coord2D pos) -> bool
    {
        Grid* grid = getGridByGridCoord(pos);
        if(nullptr == grid)
            return false;
        if(!grid->enterable(SceneItemType::trigger))
            return false;

        trigger = Trigger::create(triggerTpl->type, triggerTplId, TriggerManager::me().allocId());
        if(nullptr == trigger)
        {
            LOG_DEBUG("机关, 召唤失败, tplId:{}", triggerTplId);
            return true;
        }
        if(!addTrigger(trigger, pos))
        {
            LOG_DEBUG("机关, 添加到场景失败, tplId:{}", triggerTplId);
            trigger = nullptr;
            return false;  
        }
        if(!TriggerManager::me().insert(trigger))
        {
            LOG_DEBUG("机关, 添加到机关管理器失败, tplId:{}", triggerTplId);
            trigger = nullptr;
            return false;
        }
        LOG_DEBUG("机关, summon成功, 场景:{}, tplId:{}, pos:{}", name(), triggerTplId, pos);

        trigger->setLifetime(triggerTpl->lifetime);
        trigger->afterEnterScene();
        return true;
    };

    tryExecAreaByCircle(center, radius, summonExec);
    return trigger;
}

bool Scene::addTrigger(Trigger::Ptr trigger, Coord2D pos)
{
	if(trigger == nullptr)
		return false;

    Grid* grid = getGridByGridCoord(pos);
    if(grid == nullptr || !grid->addTrigger(trigger))
        return false;

    Screen* screen = getScreenByGridCoord(pos);
    if(screen == nullptr || !screen->addTrigger(trigger))
    {
        grid->eraseTrigger(trigger);
        return false;
    }

    if(!m_triggers.insert(std::make_pair(trigger->id(), trigger)))
    {
        grid->eraseTrigger(trigger);
        screen->eraseTrigger(trigger);
        return false;
    }

    trigger->setPos(pos);
    trigger->setSceneId(id());
    return true;
}

void Scene::eraseTrigger(Trigger::Ptr trigger)
{
	if(trigger == nullptr)
		return;

    Coord2D pos = trigger->pos();
    Grid* grid = getGridByGridCoord(pos);
    if(grid != nullptr)
        grid->eraseTrigger(trigger);

    Screen* screen = getScreenByGridCoord(pos);
    if(screen != nullptr)
        screen->eraseTrigger(trigger);

    m_triggers.erase(trigger->id());
    trigger->setSceneId(0);
}

void Scene::setBossId(uint32_t bossId)
{
    m_bossId = bossId;    
}

PK::Ptr Scene::getPKByIdAndType(PKId id, SceneItemType sceneItem) const
{
    PK::Ptr target(nullptr);
    SceneItemType type = static_cast<SceneItemType>(sceneItem);
    switch(type)
    {
    case SceneItemType::role:
        target = getRoleById(id);
        break;
    case SceneItemType::hero:
        target = getHeroById(id);
        break;
    case SceneItemType::npc:
        target = getNpcById(id);
        break;
    case SceneItemType::pet:
        target = getPetById(id);
        break;
    default:
        break;
    }
    return target;
}

uint32_t Scene::bossId() const
{
    return m_bossId;
}

void Scene::setRoleInTime()
{
    m_roleInTime = water::componet::Clock::now();
}

void Scene::setCreateTime()
{
    m_createTime = water::componet::Clock::now();
}

water::componet::TimePoint Scene::createTime() const
{
    return m_createTime;
}

bool Scene::roleEmpty() const
{
    return m_roles.empty();
}

void Scene::highlightMapBlock(std::shared_ptr<Role> role, Coord2D pos, uint8_t state, uint32_t duration)
{
    highlightMapArea(role, std::list<Coord2D>{pos}, state, duration);
}

void Scene::highlightMapArea(std::shared_ptr<Role> role, const std::list<Coord2D>& area, uint8_t state, uint32_t duration)
{
    if(role == nullptr)
        return;

    if(area.empty())
        return;

    std::vector<uint8_t> buf;
    buf.reserve(512);
    auto send = reinterpret_cast<PublicRaw::HighlightMapBlock*>(buf.data());
    send->size = 0;
    for(const auto& pos : area)
    {
        auto needSize = sizeof(*send) + sizeof(send->list[0]) * (send->size + 1);
        if(needSize > buf.size())
        {
            buf.resize(needSize);
            send = reinterpret_cast<PublicRaw::HighlightMapBlock*>(buf.data());
        }
        send->list[send->size].x = pos.x;
        send->list[send->size].y = pos.y;
        send->list[send->size].duration = duration;
        send->list[send->size].state = state;
        ++send->size;
    }

    role->sendToMe(RAWMSG_CODE_PUBLIC(HighlightMapBlock), buf.data(), buf.size());
}

}
