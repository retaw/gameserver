/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-08 19:39 +0800
 *
 * Modified: 2015-04-08 19:39 +0800
 *
 * Description: 场景分配器, 管理着副本world的分配已经所有scene到world的索引关系
 *
 *              创建副本的流程,
 *              session -> world  发送sceneId
 *              world -> session  session保存新建scene的索引
 *
 *              销毁流程
 *              world -> session  发送sceneId, 通知session有scene需要销毁
 *              session -> world  删掉scene索引, 并发送sceneId到world, world收到后销毁scene
 *
 *              总之, 副本的索引, world先建立后销毁, sessin后建立先销毁
 *              
 */

#ifndef PROCESSES_SESSION_SCENE_DISPENSER_H
#define PROCESSES_SESSION_SCENE_DISPENSER_H


#include "water/common/commdef.h"
#include "water/common/scenedef.h"
#include "water/componet/datetime.h"
#include "water/process/process_id.h"

#include <map>
#include <set>
#include <string>
#include <functional>


namespace session{

using namespace water;
using process::ProcessIdentity;

class SceneDispenser 
{
public:
    void loadConfig(const std::string& cfgDir);
    process::ProcessIdentity sceneId2WorldId(SceneId sceneId) const;

    //创建一个副本, 异步, 用回调处理创建结果
    void createDynamicScene(MapId mapId, const std::function<void (SceneId)>& callback);

    void destroyDynamicScene(SceneId sceneId);

    void regMsgHandler();
    void regTimer();

private:
    void timerExec(const componet::TimePoint& now);

    void servermsg_RetCreateDynamicScene(const uint8_t* data, uint32_t size, ProcessIdentity worldId);
    //其它进程发来的请求创建动态场景
    void servermsg_ReqCreateDynamicSceneToSession(const uint8_t* data, uint32_t size, ProcessIdentity worldId);
    //其他进程请求销毁动态场景
    void servermsg_ReqDestroyDynamicSceneToSession(const uint8_t* data, uint32_t size, ProcessIdentity worldId);

private:
    struct Cfg
    {
        std::map<MapId, process::ProcessIdentity> staticMapId2WorldId;
        std::map<MapId, std::set<process::ProcessIdentity>> dynamicMapId2WorldId;
    } m_cfg;

    //不分静态动态, 全在一起
    std::map<SceneId, process::ProcessIdentity> m_sceneInWorld;

    //动态场景创建请求计数器
    uint16_t m_lastDynamicSceneCounter;
    //已注册的动态场景创建回调
    std::map<SceneId, std::function<void (SceneId)>> m_createDynamicSceneCallbacks;

public:
    static SceneDispenser& me();
};

}


#endif
