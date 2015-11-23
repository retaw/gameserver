/*
 * Author: LiZhaojia
 *
 * Created: 2015-06-17 09:37 +0800
 *
 * Modified: 2015-06-17 09:37 +0800
 *
 * Description: 用户计数器
 */

#ifndef PROCESS_WORLD_ROLE_COUNTER_H
#define PROCESS_WORLD_ROLE_COUNTER_H

#include "water/common/roledef.h"
#include "water/componet/datetime.h"
#include <unordered_map>
#include <vector>
#include <memory>

namespace world{

using namespace water::componet;

enum class CounterType : uint32_t
{
    example					= 0,
    dayFlag					= 1, //零点/跨天 处理
	guanzhiReward			= 2, //官职奖励
	autoAddExpSec			= 3, //自动加经验的时间
    dayRefreshFacShopTime	= 4, //每天累加刷新帮派商店次数
	bonfire					= 5, //篝火烤火
	finishAllTaskDayCount	= 6, //日常任务，连续完成所有任务的天数
	dailyTaskCount			= 7, //日常任务，当天第几个任务
	finishTopStarTaskNum    = 8, //日常任务，完成满星任务数量
    shabakeDailyAward       = 9, //沙巴克日常奖励领取
};


//保存计数模式
enum class CounterSaveMode : uint8_t
{
    forever = 0,    //一直保存
    day     = 1,    //同一天
    week    = 2,    //同一礼拜
    month   = 3,    //同一个月

    //...
};

#pragma pack(1)
struct CountElement
{
    CounterType     type;
    CounterSaveMode mode;
    uint32_t        count;
    TimePoint       time;
};
#pragma pack()

class Role;
class RoleCounter
{
public:
    explicit RoleCounter(Role& me);
    ~RoleCounter() = default;

private:
    void init();
    void setCounterSaveMode(CounterType type, const CounterSaveMode& mode);

public:
    void clear(CounterType type);
    uint32_t add(CounterType type, uint32_t count=1);
    void set(CounterType type, uint32_t count);
	uint32_t get(CounterType type);

		
    void saveToDB() const;
    void loadFromDB(const std::string& counterStr);


private:
    Role& m_owner;
    std::unordered_map<uint32_t, CountElement> m_counters;
};


}

#endif

