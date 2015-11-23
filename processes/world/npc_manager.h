/*
 * Author: LiZhaojia
 *
 * Created: 2015-05-11 21:03 +0800
 *
 * Modified: 2015-05-11 21:03 +0800
 *
 * Description: 
 */

#include "npc.h"

#include <unordered_map>

namespace world{

class NpcManager : public NpcMap
{
public:
    NpcManager();
    ~NpcManager();

    Npc::Ptr getById(NpcId id);

    Npc::Ptr createNpc(NpcTplId tplId);

    void loadConfig(const std::string& configDir);

    void regTimer();

private:
    void timerLoop(StdInterval interval, const componet::TimePoint& now);

private:
    NpcId m_lastNpcId = 0;
    std::unordered_map<NpcTplId, NpcTpl::Ptr> m_tpls;

public:
    static NpcManager& me();

};


}
