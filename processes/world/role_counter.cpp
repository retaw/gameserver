#include "role_counter.h"
#include "role.h"
#include "world.h"
#include "water/componet/serialize.h"

#include "protocol/rawmsg/private/counter.h"
#include "protocol/rawmsg/private/counter.codedef.private.h"


namespace world{

RoleCounter::RoleCounter(Role& me)
: m_owner(me)
{
    init();
}

void RoleCounter::init()
{
    m_counters.clear();
    setCounterSaveMode(CounterType::dayFlag, CounterSaveMode::day);
    setCounterSaveMode(CounterType::guanzhiReward, CounterSaveMode::day);
	setCounterSaveMode(CounterType::autoAddExpSec, CounterSaveMode::day);
	setCounterSaveMode(CounterType::dayRefreshFacShopTime, CounterSaveMode::day);
	setCounterSaveMode(CounterType::bonfire, CounterSaveMode::day);
	setCounterSaveMode(CounterType::finishAllTaskDayCount, CounterSaveMode::forever);
	setCounterSaveMode(CounterType::dailyTaskCount, CounterSaveMode::forever);
	setCounterSaveMode(CounterType::finishTopStarTaskNum, CounterSaveMode::day);
	setCounterSaveMode(CounterType::shabakeDailyAward, CounterSaveMode::day);
}

void RoleCounter::setCounterSaveMode(CounterType type, const CounterSaveMode& mode)
{
    uint32_t ctype = static_cast<uint32_t>(type);
    if(m_counters.find(ctype) != m_counters.end())
        return;

    CountElement ele;
    ele.type = type;
    ele.mode = mode;
    ele.count = 0;
    ele.time = Clock::now();
    m_counters.insert(std::make_pair(ctype, ele));
}


void RoleCounter::clear(CounterType type)
{
    if(m_counters.find(static_cast<uint32_t>(type)) == m_counters.end())
        return;

    CountElement& ele = m_counters[static_cast<uint32_t>(type)];
    ele.count = 0;
    ele.time = Clock::now();
}


uint32_t RoleCounter::add(CounterType type, uint32_t count/*=1*/)
{
    if(m_counters.find(static_cast<uint32_t>(type)) == m_counters.end())
        return (uint32_t)-1;

    CountElement& ele = m_counters[static_cast<uint32_t>(type)];
    switch(ele.mode)
    {
    case CounterSaveMode::forever:
        ele.count += count;
        break;
    case CounterSaveMode::day:
        {
            if(!inSameDay(ele.time, Clock::now()))
                ele.count = count;
            else
                ele.count += count;
        }
        break;
    case CounterSaveMode::week:
        {
            if(!inSameWeek(ele.time, Clock::now()))
                ele.count = count;
            else
                ele.count += count;
        }
        break;
    case CounterSaveMode::month:
        {
            if(!inSameMonth(ele.time, Clock::now()))
                ele.count = count;
            else
                ele.count += count;
        }
        break;
    default:
        break;
    }
    ele.time = Clock::now();
    saveToDB();
    return ele.count;
}

void RoleCounter::set(CounterType type, uint32_t count)
{
	auto pos = m_counters.find(static_cast<uint32_t>(type));
	if(pos == m_counters.end())
		return;

	pos->second.count = count;
	pos->second.time = Clock::now();
    saveToDB();
	return;
}

uint32_t RoleCounter::get(CounterType type)
{
    if(m_counters.find(static_cast<uint32_t>(type)) == m_counters.end())
        return 0;

    CountElement& ele = m_counters[static_cast<uint32_t>(type)];
    switch(ele.mode)
    {
    case CounterSaveMode::forever:
        break;
    case CounterSaveMode::day:
        {
            if(!inSameDay(ele.time, Clock::now()))
                ele.count = 0;
        }
        break;
    case CounterSaveMode::week:
        {
            if(!inSameWeek(ele.time, Clock::now()))
                ele.count = 0;
        }
        break;
    case CounterSaveMode::month:
        {
            if(!inSameMonth(ele.time, Clock::now()))
                ele.count = 0;
        }
        break;
    default:
        break;
    }

    return ele.count;
}


void RoleCounter::saveToDB() const
{
    std::vector<uint8_t> buf;
    buf.reserve(128);

    std::vector<CounterInfo> counterInfo;
    for(const auto& it : m_counters)
    {
        CounterInfo info;
        info.counterType = it.first;
        info.count = it.second.count;
        info.time = toUnixTime(it.second.time);
        counterInfo.push_back(info);
    }

    Serialize<std::string> iss;
    iss.reset();
    iss << counterInfo;
    buf.resize(sizeof(PrivateRaw::ModifyCounterInfo) + iss.tellp());
    auto msg = reinterpret_cast<PrivateRaw::ModifyCounterInfo*>(buf.data());
    msg->size = iss.tellp();
    msg->roleId = m_owner.id();
    std::memcpy(msg->buf, iss.buffer()->data(), iss.tellp());

    ProcessIdentity dbcachedId("dbcached", 1);
    World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(ModifyCounterInfo), buf.data(), buf.size());
}


void RoleCounter::loadFromDB(const std::string& counterStr)
{
    Deserialize<std::string> ds(&counterStr);
    std::vector<CounterInfo> counterInfo;
    ds >> counterInfo;
    for(const auto& iter : counterInfo)
    {
        if(m_counters.find(iter.counterType) == m_counters.end())
            continue;

        CountElement& ele = m_counters[iter.counterType];
        ele.count = iter.count;
        ele.time = Clock::from_time_t(iter.time);
    }
}

}
