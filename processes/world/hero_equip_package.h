/*
 * Author: zhupengfei
 *
 * Created: 2015-04-13 +0800
 *
 * Modified: 2015-04-13 +0800
 *
 * Description: 英雄装备包
 */

#ifndef PROCESS_WORLD_HERO_EQUIP_PACKAGE_HPP
#define PROCESS_WORLD_HERO_EQUIP_PACKAGE_HPP

#include "water/common/roledef.h"
#include "water/componet/class_helper.h"

#include "object.h"
#include "equip_package.h"
#include "package.h"

#include <memory>
#include <vector>
#include <map>

namespace world{

//向背包中put obj 时，需要多少个格子由packageSet来处理
//需要多个格子，则对应create多个 Object::Ptr
//确保背包中每个格子中的Object::Ptr唯一

class Role;

/*********************************************/
class HeroEquipPackage : public EquipPackage
{
public:
	TYPEDEF_PTR(HeroEquipPackage)
	CREATE_FUN_NEW(HeroEquipPackage) 

public:
	HeroEquipPackage(SceneItemType sceneItem, PackageType packageType, uint16_t totalCellNum, uint16_t unlockNum, const std::vector<CellInfo>& cellVec);

	HeroEquipPackage() = default;
	virtual ~HeroEquipPackage() = default;

public:
	bool checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind) override;

public:
	void sendAllAttributeToMe() override;
	void updateAllEquipSkills() override;

	//将装备包显示外观的物品列表发到九屏
	void sendObjListToNine() override;

};


}

#endif
