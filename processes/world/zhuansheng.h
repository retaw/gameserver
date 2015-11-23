/*
 * Author: zhupengfei
 *
 * Created: 2015-09-06 14:15 +0800
 *
 * Modified: 2015-09-06 14:15 +0800
 *
 * Description: 转生
 */

#ifndef PROCESS_WORLD_ZHUAN_SHENG_HPP
#define PROCESS_WORLD_ZHUAN_SHENG_HPP

#include "attribute.h"
#include "zhuansheng_config.h"

#include "water/common/roledef.h"

namespace world{

class PK;
class Role;

class Zhuansheng : public Attribute
{
public:
	explicit Zhuansheng(SceneItemType sceneItem, RoleId roleId, PK& owner);
	~Zhuansheng() = default;

public:
	void requestZhuansheng();
	void calcAttribute();  

private:
	std::shared_ptr<Role> getRole() const;

	bool judgeZhuansheng();
	bool judgeLimitRole(LimitType type, uint32_t needLevel);
	bool judgeLimitHero(LimitType type, uint32_t needLevel);

	void calcAttributeOfRole();
	void calcAttributeOfHero();

	void sendZhuanshengResult(OperateRetCode code);

private:
	const SceneItemType m_sceneItem; 
	const RoleId m_roleId;
	PK& m_owner;
};


}

#endif
