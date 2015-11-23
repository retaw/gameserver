/*
 * Author: zhupengfei
 *
 * Created: 2015-06-11 15:00 +0800
 *
 * Modified: 2015-06-11 15:00 +0800
 *
 * Description: 处理GM相关逻辑
 */

#ifndef PROCESS_WORLD_GM_HPP
#define PROCESS_WORLD_GM_HPP

#include "water/process/process_id.h"
#include "water/common/roledef.h"
#include "water/componet/string_kit.h"
#include "water/componet/coord.h"

#include <functional>
#include <memory>
#include <map>

namespace world{

using water::process::ProcessIdentity;

class Role;

class Gm
{
	typedef std::function<void (std::shared_ptr<Role>, std::string)> GmMsgHandler; 
public:
	Gm();
	~Gm() = default;
    static Gm& me();
private:
	static Gm m_me;

public:
    void regMsgHandler();

private:
	//func广播端发来的GM指令
	void servermsg_BroadcastGmMsgToGlobal(const uint8_t* msgData, uint32_t msgSize);

	void init();

	std::string getKeyString(std::string str) const;

	static uint32_t getSubStrValue(std::string str, std::string subStr);
	static std::string getSubStr(std::string str, std::string subStr);

    template<typename T>
    static T getSubStrValue(std::string str, std::string subStr)
    {
        return water::componet::fromString<T>(getSubStr(str, subStr));
    }

private:
    static void lzjTest(std::shared_ptr<Role> role, std::string str); //lzj个人专用, 可能是任何功能

private:
	static void addMoney(std::shared_ptr<Role> role, std::string str);
	static void addObject(std::shared_ptr<Role> role, std::string str);
	static void levelUp(std::shared_ptr<Role> role, std::string str);
	static void addExp(std::shared_ptr<Role> role, std::string str);
	static void addHp(std::shared_ptr<Role> role, std::string str);
	static void addMp(std::shared_ptr<Role> role, std::string str);
	static void learnSkill(std::shared_ptr<Role> role, std::string str);
	static void goTo(std::shared_ptr<Role> role, std::string str);
	static void goToUser(std::shared_ptr<Role> role, std::string str);
	static void catchUser(std::shared_ptr<Role> role, std::string str);
	static void sendMail(std::shared_ptr<Role> role, std::string str);
	static void addTitle(std::shared_ptr<Role> role, std::string str);
	static void addfaction(std::shared_ptr<Role> role, std::string str);
	static void summonTrigger(std::shared_ptr<Role> role, std::string str);
	static void addAnger(std::shared_ptr<Role> role, std::string str);
	static void demaxiya(std::shared_ptr<Role> role, std::string str);
	static void wudi(std::shared_ptr<Role> role, std::string str);
	static void systime(std::shared_ptr<Role> role, std::string str);
	static void sendNotice(std::shared_ptr<Role> role, std::string str);
    static void summonNpc(std::shared_ptr<Role> role, std::string str);
    static void task(std::shared_ptr<Role> role, std::string str);

private:
	std::map<std::string, GmMsgHandler> m_gmMsgMap; 

};



}




#endif
