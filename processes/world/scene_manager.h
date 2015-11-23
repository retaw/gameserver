/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-09 21:30 +0800
 *
 * Modified: 2015-04-09 21:30 +0800
 *
 * Description: 场景管理器
 *              static scene:  id == mapId
 *              dynamic scene: id 由session分配
 */

#ifndef PROCESSES_WORLD_SCENE_MANAGER_H
#define PROCESSES_WORLD_SCENE_MANAGER_H


#include "water/common/scenedef.h"
#include "water/componet/exception.h"

#include "position.h"
#include "scene.h"
#include "npc.h"


#include <set>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>


namespace world{


DEFINE_EXCEPTION(CreateStaticSceneFailed, water::componet::ExceptionBase);

class SceneManager
{
public:
    ~SceneManager() = default;
    void loadConfig(const std::string& cfgDir);
    void regMsgHandler();

    Scene::Ptr getById(SceneId sceneId) const;

    void regTimer();
    void timerExec(StdInterval interval, const componet::TimePoint& now);

    void createDynamicScene(MapId mapId, const std::function<void (SceneId)>& callback);
    void destroyDynamicScene(SceneId sceneId);

    //获得某个地图的所有npc类型的数量
    std::unordered_map<NpcTplId, uint32_t> getNpcTplSize(SceneId sceneId);

private:
    SceneManager() = default;
    void loadWorldConfig(const std::string& cfgDir);
    void loadMapConfig(MapId mapId, const std::string& cfgDir);

    void createStaticScene(MapId mapId);
    Scene::Ptr createScene(SceneId sceneId);

    uint64_t getDynamicSceneFunctionKey();


private:
    void servemsg_CreateDynamicScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    void servemsg_DestroyDynamicScene(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    //world发起的创建静态场景的回复
    void servemsg_RetCreateDynamicSceneToOther(const uint8_t* msgData, uint32_t msgSize, ProcessIdentity pid);
    
private:
    struct Cfg
    {
        struct MapCfg
        {
            //基本信息
            BasicMapInfo basic;

            //阻挡
            std::vector<Coord2D> blocks;
            //默认复活区
            std::vector<Coord2D> defaultReliveArea;
            //经验区
            std::vector<Coord2D> expArea;
            //安全区
            std::vector<Coord2D> safeArea;
            //无碰撞区
            std::vector<Coord2D> collisionArea;

            std::map<int32_t, Scene::Transmission> transmissions;

            struct NpcArea
            {
                NpcTplId npcTplId;
                uint32_t npcSize;
                std::vector<Coord2D> grids;
            };
            std::vector<NpcArea> npcAreas;
        };
        std::map<MapId, MapCfg> allMaps;

        //map资源的static和dynamic并不互斥, 这里两个set可能有交集
        std::set<MapId> staticMapIds;
        std::set<MapId> dynamicMapIds;
    } m_cfg;

    std::map<SceneId, Scene::Ptr> m_scenes;

    //动态场景回调,使用map保证最后的一位key是最大
    std::map<uint64_t, std::function<void (SceneId)>> m_dynamicSceneFunctions;
public:
    static SceneManager& me();
};

}

#endif
