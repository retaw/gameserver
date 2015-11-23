/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-13 16:24 +0800
 *
 * Modified: 2015-04-13 16:24 +0800
 *
 * Description: 场景的抽象
 */

#ifndef PROCESSES_WORLD_SCENE_H
#define PROCESSES_WORLD_SCENE_H



#include "water/common/scenedef.h"
#include "water/componet/fast_travel_unordered_map.h"
#include "position.h"
#include "screen.h"
#include "grid.h"
#include "water/componet/datetime.h"

#include "role.h"
#include "npc.h"
#include "hero.h"
#include "scene_object.h"
#include "map_base.h"
#include "pet.h"
#include "fire.h"
#include "trigger_manager.h"

#include <vector>
#include <array>
#include <functional>

namespace world{

class Scene
{
public:
    TYPEDEF_PTR(Scene);
    CREATE_FUN_NEW(Scene);

    struct Transmission
    {
        Coord2D grid;
        Coord1D maxDistance;
        MapId   destinationMapId;
        Coord2D destinationGrid;
    };

public:
    Scene(SceneId id, const BasicMapInfo& mapInfo);
    ~Scene();

    SceneId id() const;
    MapId mapId() const;
    const std::string& name() const;

    bool isDynamic() const;
    bool avaliable() const;
    uint32_t minRoleLevel() const;

    void timerExec(StdInterval interval, const TimePoint& now);

    void cleanup();
    //移除场景上SceneItem
    void removeSceneItem(SceneItemType type = SceneItemType::none);

    Grid* getGridByGridCoord(Coord2D coord);
    Screen* getScreenByGridCoord(Coord2D coord);

    //格子为中心的最多9格, 返回的序列已按坐标排序
    std::vector<Grid*> get9GridByGridCoord(Coord2D coord);
    //格子所在屏为中心的最多9屏, 返回的序列已按屏坐标排序
    std::vector<Screen*> get9ScreensByGridCoord(Coord2D coord);

    bool enterable(Coord2D pos, SceneItemType sceneItemType);

    bool addRole(Role::Ptr role, Coord2D centre, Coord1D radius);
    bool addRole(Role::Ptr role, Coord2D pos);
    bool addRole(Role::Ptr role);
    void eraseRole(Role::Ptr role);
    void addDeadRole(RoleId roleId, Coord2D pos);
    void eraseDeadRole(RoleId roleId, Coord2D pos);
    Role::Ptr getRoleById(RoleId roleId) const;

    Npc::Ptr summonNpc(NpcTplId npcTplId, Coord2D centre, Coord1D radius);
    Npc::Ptr summonNpc(NpcTplId npcTplId, Coord2D pos);
    void summonStaticNpcs(NpcTplId npcTplId, uint32_t totalNum, const std::vector<Coord2D>& area);
    bool addNpc(Npc::Ptr npc, Coord2D centre, Coord1D radius);
    bool addNpc(Npc::Ptr npc, Coord2D pos);
    void eraseNpc(Npc::Ptr npc);
    Npc::Ptr getNpcById(NpcId npcId) const;

    void broadCastDropObj(const std::string& roleName, SceneObject::Ptr obj, const std::string& npcName);
    void addSceneObj(Coord2D center, std::vector<SceneObject::Ptr>& sceneObjList, const std::string& roleName = "", const std::string& npcName = "", bool broadCast = false);
    bool addObj(SceneObject::Ptr obj, Coord2D centre, Coord1D radius);
    bool addObj(SceneObject::Ptr obj, Coord2D pos);
	void eraseObj(ObjectId objId);
	void eraseObj(SceneObject::Ptr obj);
    SceneObject::Ptr getObjById(ObjectId objId) const;

    bool addHero(Hero::Ptr hero, Coord2D centre, Coord1D radius);
    bool addHero(Hero::Ptr hero, Coord2D pos);
	void eraseHero(HeroId heroId);
	void eraseHero(Hero::Ptr hero);
    Hero::Ptr getHeroById(HeroId heroId) const;

    //宠物
    bool addPet(Pet::Ptr pet, Coord2D center, Coord1D radius);
    bool addPet(Pet::Ptr pet, Coord2D pos);
    void erasePet(PKId petId);
    void erasePet(Pet::Ptr pet);
    Pet::Ptr getPetById(PKId petId) const;

    //火墙
    bool addFire(Fire::Ptr fire, Coord2D pos);
    void eraseFire(Fire::Ptr fire);

    PK::Ptr getPKByIdAndType(PKId id, SceneItemType sceneItem) const;

    //沿渐开线操作格子, 一旦执行结果为true, 即停止
    bool tryExecSpiral(Coord2D centre, Coord1D radius, std::function<bool (Coord2D)> exec);

    //圆形区域遍历
    bool tryExecAreaByCircle(Coord2D centre, Coord1D radius, std::function<bool (Coord2D)> exec);
    //环形遍历
    bool execCircle(Coord2D start, Coord1D radius, std::function<bool (Coord2D)> exec); 

    Coord2D gridCoord2ScreenCoord(Coord2D coord);
    
    void setMapBlock(Coord2D block);
    void setMapBlocks(const std::vector<Coord2D>& blocks);
    void setReliveArea(const std::vector<Coord2D>& area);
    bool existReliveArea() const;

    //地图上随机一个SceneItem可进入的坐标
    Coord2D randAvailablePos(SceneItemType sceneItemType);
	//地图上随机一个复活点坐标
	Coord2D randomRelivePos();

    //地图配置相关
    void setMapTpl(MapId);
    MapTpl::Ptr mapTpl() const;
    attack_mode mapAttackMode() const;
    uint8_t crime() const;
    uint8_t dropEquip() const;
    CopyMap copyType() const;
    bool canRide() const;
    uint16_t reliveYB() const;

    const std::map<int32_t, Transmission>& transmissions() const;
    void setTransmissions(const std::map<int32_t, Transmission>& transmissions);

    std::list<Coord2D> findPath(Coord2D start, Coord2D goal, SceneItemType sceneItemType);

	//9屏消息发送接口
	bool sendCmdTo9(TcpMsgCode msgCode, const void* msg, uint32_t msgSize, Coord2D pos);
	void sendToScreens(const std::vector<Screen*>& screens, TcpMsgCode msgCode, const void* msg, uint32_t msgSize) const; 

	void setArea(const std::vector<Coord2D>& coordVec, AreaType type);
	void setArea(Coord2D coord, AreaType type);
    void unsetArea(Coord2D coord, AreaType type);
    bool isArea(Coord2D coord, AreaType type);

    //遍历地图上角色
    void execRole(std::function<void (Role::Ptr)> exec);
    //遍历地图上npc
    void execNpc(std::function<void (Npc::Ptr)> exec);
    //遍历地图上道具
    void execSceneObj(std::function<void (SceneObject::Ptr)> exec);
    //遍历地图上机关物件
    void execTrigger(std::function<void (Trigger::Ptr)> exec);

    //场景动态召唤机关物件
    Trigger::Ptr summonTrigger(uint32_t triggerTplId, Coord2D center, uint16_t radius);
    bool addTrigger(Trigger::Ptr trigger, Coord2D pos);
    void eraseTrigger(Trigger::Ptr trigger);

    void setBossId(uint32_t bossId);
    uint32_t bossId() const;
    void setCreateTime();
    water::componet::TimePoint createTime() const;
    void setRoleInTime();
    bool roleEmpty() const;

public:
    static void highlightMapBlock(std::shared_ptr<Role> role, Coord2D pos, uint8_t state = 1, uint32_t duration = 500);
    static void highlightMapArea(std::shared_ptr<Role> role, const std::list<Coord2D>& area, uint8_t state = 1, uint32_t duration = 500);

private:
    const SceneId m_id;
    const BasicMapInfo m_basicInfo;
    MapTpl::Ptr m_mapTpl;
    std::vector<std::vector<Grid>> m_grids;
    std::vector<std::vector<Screen>> m_screens;
    RoleMap m_roles;
    NpcMap m_npcs;
	SceneObjMap m_sceneObjs;
	HeroMap m_heros;
    PetMap m_pets;
    FireMap m_fires;
    TriggerMap m_triggers;

    std::vector<Coord2D> m_relivePos;

    std::map<int32_t, Transmission> m_transmissions;
    uint32_t m_bossId = 0;//此场景的主人，用于副本，如个人boss中，此字段代表本场景的主人bossId,场景创建即赋值
    water::componet::TimePoint m_roleInTime = water::componet::EPOCH;    //进入场景的时间
    water::componet::TimePoint m_createTime = water::componet::EPOCH;    //创建场景时间
    bool m_avaliable = true;
public:
    std::unordered_map<NpcTplId, uint32_t> m_npcInScene;   //<npcId, 死亡的个数>

};

}

#endif
