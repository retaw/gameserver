#include "gm.h"
#include "role_manager.h"
#include "roles_and_scenes.h"
#include "world.h"
#include "mail_manager.h"
#include "scene.h"
#include "channel.h"
#include "task_base.h"
#include "ai.h"
#include "npc_manager.h"

#include "water/componet/string_kit.h"

#include "protocol/rawmsg/private/channel_info.h"
#include "protocol/rawmsg/private/channel_info.codedef.private.h"

#include "protocol/rawmsg/private/mail.h"
#include "protocol/rawmsg/private/mail.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/faction.h"
#include "protocol/rawmsg/private/faction.codedef.private.h"


namespace world{

using namespace std::placeholders;

Gm Gm::m_me;

Gm& Gm::me()
{
	return m_me;
}

Gm::Gm()
{
	init();
}

void Gm::regMsgHandler()
{
	REG_RAWMSG_PRIVATE(BroadcastGmMsgToGlobal, std::bind(&Gm::servermsg_BroadcastGmMsgToGlobal, this, _1, _2));
}

//func广播端发来的GM指令
void Gm::servermsg_BroadcastGmMsgToGlobal(const uint8_t* msgData, uint32_t msgSize)
{
	auto rev = reinterpret_cast<const PrivateRaw::BroadcastGmMsgToGlobal*>(msgData);
	if(!rev)
		return;

	Role::Ptr role = RoleManager::me().getById(rev->roleId);
	if(role == nullptr)
		return;

	if(rev->textSize + sizeof(*rev) > msgSize)
	{
		LOG_ERROR("GM, 收到的消息长度非法, revSize={}, needSize={}", msgSize, rev->textSize + sizeof(*rev));
		return;
	}

	std::string keyStr = getKeyString(rev->text);
	auto pos = m_gmMsgMap.find(keyStr);
	if(pos == m_gmMsgMap.end())
    {
        role->sendSysChat("不存在的GM指令");
		return;
    }

	pos->second(role, rev->text);
	return;
}


void Gm::init()
{
#ifdef DEBUG
    m_gmMsgMap["lzj"] = lzjTest;
#endif
    /*******************************/
	m_gmMsgMap["addmoney"] = addMoney;
	m_gmMsgMap["addobj"] = addObject;
	m_gmMsgMap["levelup"] = levelUp;
	m_gmMsgMap["addexp"] = addExp;
	m_gmMsgMap["addhp"] = addHp;
	m_gmMsgMap["addmp"] = addMp;
	m_gmMsgMap["learnskill"] = learnSkill;
	m_gmMsgMap["goto"] = goTo;
	m_gmMsgMap["gotouser"] = goToUser;
	m_gmMsgMap["catchuser"] = catchUser;
	m_gmMsgMap["sendmail"] = sendMail;
	m_gmMsgMap["addtitle"] = addTitle;
	m_gmMsgMap["addfaction"] = addfaction;
	m_gmMsgMap["summontrigger"] = summonTrigger;
	m_gmMsgMap["addanger"] = addAnger;
	m_gmMsgMap["demaxiya"] = demaxiya;
	m_gmMsgMap["wudi"] = wudi;
	m_gmMsgMap["systime"] = systime;
	m_gmMsgMap["sendnotice"] = sendNotice;
	m_gmMsgMap["summonnpc"] = summonNpc;
	m_gmMsgMap["task"] = task;
}

std::string Gm::getKeyString(std::string str) const
{
	std::string keyStr;
	std::vector<std::string> strItems = water::componet::splitString(str, " ");  
	if(strItems.empty())
		return keyStr;

	return strItems[0];
}

uint32_t Gm::getSubStrValue(std::string str, std::string subStr)
{
	subStr = subStr.append("=");
	std::string::size_type subStrSize = subStr.size();
	std::vector<std::string> strItems = water::componet::splitString(str, " ");  
	
	for(const std::string& item : strItems) 
	{
		std::string::size_type index = item.find(subStr);
		if(index == std::string::npos)
			continue;

		if(item.substr(index).size() <= subStrSize)
			return (uint32_t)-1;
	
		std::string value = item.substr(index + subStrSize);
		return atoi(value.c_str());
	}

	return (uint32_t)-1;
}

std::string Gm::getSubStr(std::string str, std::string subStr)
{
	subStr = subStr.append("=");
	std::string::size_type subStrSize = subStr.size();
	std::vector<std::string> strItems = water::componet::splitString(str, " ");  
	
	for(const std::string& item : strItems) 
	{
		std::string::size_type index = item.find(subStr);
		if(index == std::string::npos)
			continue;

		if(item.substr(index).size() <= subStrSize)
			return "";
	
		return item.substr(index + subStrSize);
	}

    return "";
}

void Gm::lzjTest(std::shared_ptr<Role> role, std::string str)
{
	if(role == nullptr)
		return;

    ai::AIManager::me().loadConfig(World::me().cfgDir());

    auto& npcs = NpcManager::me();
    for(auto it = npcs.begin(); it != npcs.end(); ++it)
        it->second->resetAI();

    role->sendSysChat("lzjTest");
}

void Gm::addMoney(Role::Ptr role, std::string str)
{
	if(role == nullptr)
		return;

	uint32_t type = getSubStrValue(str, "type");
    uint64_t num = getSubStrValue<uint64_t>(str, "num");

	role->addMoney(static_cast<MoneyType>(type), num, "GM指令");
	role->sendSysChat("加{}成功, num={}", 
					  role->getMoneyName(static_cast<MoneyType>(type)), num);
	return;
}

void Gm::addObject(Role::Ptr role, std::string str)
{
	if(role == nullptr)
		return;

	TplId tplId  = getSubStrValue(str, "id");
	uint32_t num = getSubStrValue(str, "num");
	if((uint32_t)-1 == tplId)
		return;

	if((uint32_t)-1 == num)
		num = 1;

	if(0 == role->putObj(tplId, num, Bind::no, PackageType::role))
	{
		role->sendSysChat("背包空间不足");
		return;
	}
	role->sendSysChat("获得物品, tplId={}, num={}", tplId, num);
	return;
}

void Gm::levelUp(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    uint32_t type = getSubStrValue(str, "type");
    uint32_t num = getSubStrValue(str, "num");
	if((uint32_t)-1 == num)
        num = 1;

    if((uint32_t)-1 == type || static_cast<SceneItemType>(type) == SceneItemType::role)
	{
        role->levelUp(num, true);
		role->sendSysChat("角色升级成功, level={}", role->level());
	}
	else if(static_cast<SceneItemType>(type) == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getSummonHero();
		if(hero == nullptr)
		{
			role->sendSysChat("未召唤英雄");
			return;
		}

		hero->levelUp(num, true);
		role->sendSysChat("英雄升级成功, level={}", hero->level());
	}

	return;
}

void Gm::addExp(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    uint32_t type = getSubStrValue(str, "type");
    uint64_t num = getSubStrValue<uint64_t>(str, "num");

    if((uint32_t)-1 == type || static_cast<SceneItemType>(type) == SceneItemType::role)
	{    
		role->addExp(num);
		role->sendSysChat("角色加经验成功, num={}", num);
	}
	else if(static_cast<SceneItemType>(type) == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getSummonHero();
		if(hero == nullptr)
		{
			role->sendSysChat("未召唤英雄");
			return;
		}

		hero->addExp(num);
		role->sendSysChat("英雄加经验成功, num={}", num);
	}

	return;
}

void Gm::addHp(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    uint32_t type = getSubStrValue(str, "type");
    uint32_t num = getSubStrValue(str, "num");
	if((uint32_t)-1 == num)
		return;

    if((uint32_t)-1 == type || static_cast<SceneItemType>(type) == SceneItemType::role)
	{
		role->changeHp(num);
		role->sendSysChat("角色加血成功, num={}", num);
	}
	else if(static_cast<SceneItemType>(type) == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getSummonHero();
		if(hero == nullptr)
		{
			role->sendSysChat("未召唤英雄");
			return;
		}

		hero->changeHp(num);
		role->sendSysChat("英雄加血成功, num={}", num);
	}

	return;
}

void Gm::addMp(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    uint32_t type = getSubStrValue(str, "type");
    uint32_t num = getSubStrValue(str, "num");
	if((uint32_t)-1 == num)
		return;

    if((uint32_t)-1 == type || static_cast<SceneItemType>(type) == SceneItemType::role)
	{
		role->changeMp(num);
		role->sendSysChat("角色加蓝成功, num={}", num);
	}
	else if(static_cast<SceneItemType>(type) == SceneItemType::hero)
	{
		Hero::Ptr hero = role->m_heroManager.getSummonHero();
		if(hero == nullptr)
		{
			role->sendSysChat("未召唤英雄");
			return;
		}

		hero->changeMp(num);
		role->sendSysChat("英雄加蓝成功, num={}", num);
	}

	return;
}

void Gm::learnSkill(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    uint32_t id = getSubStrValue(str, "id");
    uint32_t level = getSubStrValue(str, "level");
	if((uint32_t)-1 == id)
    {
        role->sendSysChat("请输入技能id");
		return;
    }

    level = (uint32_t)-1 == level ? 1 : level;
    role->upgradeSkill(id, level, true);
	role->sendSysChat("角色学习技能, id={}, level={}", id, level);
}

void Gm::goTo(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    uint32_t id = getSubStrValue(str, "id");
    Coord2D pos;
    pos.x = getSubStrValue(str, "x");
    pos.y = getSubStrValue(str, "y");
	if(0 == id || (uint32_t)-1 == id)
    {
		role->changePos(pos, role->dir(), MoveType::blink);
		role->sendSysChat("同场景跳地图, pos=[{},{}]", pos.x, pos.y);
        return;
    }

    if(!RolesAndScenes::me().gotoOtherScene(role->id(), id, pos))
        return;
	
	role->sendSysChat("跨场景跳地图, id={}, pos=[{},{}]", id, pos.x, pos.y);
}

void Gm::goToUser(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    std::string name = getSubStr(str, "name");
    if("" == name)
    {
        role->sendSysChat("请输入玩家名字");
        return;
    }

	role->sendSysChat("跳转到 {} 附近", name);
    Role::Ptr targetRole = RoleManager::me().getByName(name);
    if(targetRole == nullptr)
    {
        //发往session, 请求玩家所在场景id,pos
        PrivateRaw::RoleGotoTargetRoleScene send;
        send.rid = role->id();
        name.copy(send.targetName, sizeof(send.targetName)-1);

        ProcessIdentity sessionId("session", 1);
        World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(RoleGotoTargetRoleScene), &send, sizeof(send));
        return;
    }

    SceneId targetSceneId = targetRole->sceneId();
    if(targetSceneId == role->sceneId())
        role->changePos(targetRole->pos(), targetRole->dir(), MoveType::blink);
    else
        RolesAndScenes::me().gotoOtherScene(role->id(), targetSceneId, targetRole->pos());
}

void Gm::catchUser(Role::Ptr role, std::string str)
{
    if(role == nullptr)
        return;

    std::string name = getSubStr(str, "name");
    if("" == name)
    {
        role->sendSysChat("请输入玩家名字");
        return;
    }

	role->sendSysChat("将 {} 拉到身边", name);
    Role::Ptr targetRole = RoleManager::me().getByName(name);
    if(targetRole == nullptr)
    {
        //发往session
        PrivateRaw::WorldCatchRole send;
        send.rid = role->id();
        send.newSceneId = role->sceneId();
        send.newPosx = role->pos().x;
        send.newPosy = role->pos().y;
        name.copy(send.targetName, sizeof(send.targetName)-1);

        ProcessIdentity sessionId("session", 1);
        World::me().sendToPrivate(sessionId, RAWMSG_CODE_PRIVATE(WorldCatchRole), &send, sizeof(send));
        return;
    }

    if(targetRole->sceneId() == role->sceneId())
        targetRole->changePos(role->pos(), role->dir(), MoveType::blink);
    else
        RolesAndScenes::me().gotoOtherScene(targetRole->id(), role->sceneId(), role->pos());
}

void Gm::sendMail(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    std::string receiver = getSubStr(str, "receiver");
    if(receiver == "")
        receiver = role->name();
    std::string title = getSubStr(str, "title");
    std::string text = getSubStr(str, "text");

    std::vector<ObjItem> mailObjs;
    std::string objstr = getSubStr(str, "obj");
    std::vector<std::string> vs = water::componet::splitString(objstr, ",");
    for(const auto& iter : vs)
    {
        ObjItem obj;
        std::vector<std::string> subvs;
        subvs = water::componet::splitString(iter, "-");
        if(subvs.size() >= 2)
        {
            obj.tplId = atoi(subvs[0].c_str());
            obj.num = atoi(subvs[1].c_str());
            obj.bind = Bind::no;
            if(subvs.size() >= 3)
                obj.bind = static_cast<Bind>(atoi(subvs[2].c_str()));

            mailObjs.push_back(obj);
        }
    }

    MailManager::me().send(receiver, title, text, mailObjs);
	role->sendSysChat("给 {} 发送邮件, title={}, text={} obj={}",
					  receiver, title, text, objstr);
}

void Gm::addTitle(Role::Ptr role, std::string str)
{
	if(role == nullptr)
		return;

	uint32_t titleId = getSubStrValue(str, "id");
	if((uint32_t)-1 == titleId)
		return;

	role->addTitle(titleId);
	role->sendSysChat("加称号, id={}", titleId);
	return;
}

void Gm::addfaction(std::shared_ptr<Role> role, std::string str)
{
    if(role == nullptr)
        return;
    PrivateRaw::ObjectDonate send;
    send.roleId = role->id();
    send.exp = getSubStrValue(str, "exp");
    send.resource = getSubStrValue(str, "resource");
    send.banggong = getSubStrValue(str, "banggong");
    ProcessIdentity funcId("func", 1);
    World::me().sendToPrivate(funcId, RAWMSG_CODE_PRIVATE(ObjectDonate), &send, sizeof(send));
    LOG_DEBUG("GM指令->FUNC, 帮派属性增加");

    //上面个已经将帮贡同步到了func，这里就不要再同步了
    role->setBanggongWithoutSysn(send.resource + role->banggong());
	role->sendSysChat("加帮派属性, exp={}, resource={}, banggong={}", 
					  send.exp, send.resource, send.banggong);
}

void Gm::summonTrigger(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    auto s = role->scene();
    if(nullptr == s)
        return;

    uint32_t tplId = getSubStrValue(str, "id");
    if((uint32_t)-1 == tplId)
        return;
    s->summonTrigger(tplId, role->pos(), 1);
    role->sendSysChat("召唤机关成功, id:{}", tplId);
}

void Gm::addAnger(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    uint16_t num = getSubStrValue(str, "num");
    if((uint32_t)-1 == num)
        return;
    role->addAnger(num);
	role->sendSysChat("加怒气成功 num={}", num);
}

void Gm::demaxiya(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    auto s = role->scene();
    if(nullptr == s)
        return;

    if(role->m_demaxiya)
    {
        role->m_demaxiya = false;
        role->sendSysChat("GM秒杀状态关闭");
    }
    else
    {
        role->m_demaxiya = true;
        role->sendSysChat("GM开启秒杀");
    }
}

void Gm::wudi(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    if(role->m_wudi)
    {
        role->m_wudi = false;
        role->sendSysChat("GM无敌状态关闭");
    }
    else
    {
        role->m_wudi = true;
        role->sendSysChat("GM开启无敌");
    }
}

void Gm::systime(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    std::string timestr = timePointToString(Clock::now());
    role->sendSysChat("服务器当前系统时间:{}", timestr);
}

void Gm::sendNotice(Role::Ptr role, std::string str)
{
	if(nullptr == role)
		return;

    std::string text = getSubStr(str, "text");
    Channel::me().sendSysNotifyToGlobal(ChannelType::screen_top, "{}", text);
	role->sendSysChat("发送全服公告 {}", text);
}

void Gm::summonNpc(std::shared_ptr<Role> role, std::string str)
{
    if(nullptr == role)
        return;

    auto s = role->scene();
    if(s == nullptr)
        return;

    NpcTplId npcTplId = getSubStrValue<NpcTplId>(str, "id");
    auto npc = s->summonNpc(npcTplId, role->pos().neighbor(role->dir()), 5); //面前一格
    role->sendSysChat("召唤{} {}", npcTplId, npc == nullptr ? "失败" : "成功");
}

void Gm::task(Role::Ptr role, std::string str)
{
    if(nullptr == role)
        return;

    std::string opt = getSubStr(str, "opt");
    if(opt == "")
    {
        role->sendSysChat("请输入opt选项");
        return;
    }
    TaskId taskId = getSubStrValue(str, "id");
    if(-1 == taskId)
    {
        role->sendSysChat("请输入任务id");
        return;
    }
    TaskTpl::Ptr taskPtr = TaskBase::me().getTaskTpl(taskId);
    if(nullptr == taskPtr)
    {
        role->sendSysChat("配置中不存在该任务, id:{}", taskId);
        return;
    }
    if(opt == "redo")
    {
        role->m_roleTask.redoTask(taskId);
        role->sendSysChat("重置任务成功, taskId:{}", taskId);
        return;
    }
    else if(opt == "finish")
    {
        role->m_roleTask.changeTaskState(taskId, TaskState::finished);
        role->sendSysChat("完成任务, taskId:{}", taskId);
    }
}

}
