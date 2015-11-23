/*
 * Author:
 *
 * Created: 2015-09-27 16:15 +0800
 *
 * Modified: 2015-09-27 16:15 +0800
 *
 * Description:角色的需要入库的杂项数据通用模块
 */

#ifndef PROCESS_WORLD_ROLE_SUNDRY_H
#define PROCESS_WORLD_ROLE_SUNDRY_H

#include "water/common/roledef.h"
#include "water/common/scenedef.h"
#include <unordered_map>
#include <vector>
#include <set>
#include <map>
//#include <memory>

namespace world{

class RoleSundry
{
    //data
public:
    void saveSundry();
    void timerSaveSundry(); //定时器调用

    void setFacShop(time_t time, uint16_t facShopTab, std::vector<uint32_t>& goosId);
    std::set<TplId>& heroSkillSetting(Job job);

public:
    //添加数据
    /*不常更新数据*/
    bool        m_refreshedFactionShop = false;//是否已刷新过
    time_t      m_refreshFacShopTime = 0;//上次刷新的时间
    uint16_t    m_facShopTab = 0;//商店tab
    std::vector<std::pair<uint32_t, bool>> m_facShopGoodsId;    //<货物tplId, 货物是否可购买, 可购买true>
    std::unordered_map<uint32_t, uint16_t> m_buyStoreDayLimitObj; //商店购买每日限量道具记录
    std::unordered_map<uint32_t, uint16_t> privateBossMap;  //个人boss挑战次数<bossId, 当天挑战次数>
    std::unordered_map<uint16_t, uint16_t> m_dragonSkills; //龙心能量模块<技能id, 技能等级>
    uint32_t    m_energe;   //龙心能量值
    
    struct
    {
        std::map<Job, std::set<TplId>> heroSkillSetting;//hero skillSetting
    } m_heroData;


    /*频繁定时更新数据*/
    uint16_t    m_testTimerData = 99;
    uint32_t    m_worldBossDamage = 0;      //对世界boss造成的伤害
    std::set<uint32_t> m_receivedWBDamageAward; //已领取的世界boss伤害奖励 <damageIndex>

public:
    RoleSundry() = default;
    ~RoleSundry() = default;
    void loadSundry(std::string& sundry, RoleId roleId);//不常更新数据,手动调用
    void loadSundryForTimer(std::string& sundry);//频繁更新


private:
    RoleId m_roleId;

};

}

#endif
