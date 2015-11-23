/*
 * Author: zhupengfei
 *
 * Created: 2015-07-10 14:05:00 +0800
 *
 * Modified: 2015-07-10 14:05:00 +0800
 *
 * Description: 宝石背包(镶嵌宝石)
 */

#ifndef PROCESS_WORLD_STONE_PACKAGE_HPP
#define PROCESS_WORLD_STONE_PACKAGE_HPP

#include "water/common/roledef.h"
#include "water/componet/class_helper.h"

#include "object.h"
#include "package.h"

#include <memory>
#include <vector>

namespace world{

//向背包中put obj 时，需要多少个格子由packageSet来处理
//需要多个格子，则对应create多个 Object::Ptr
//确保背包中每个格子中的Object::Ptr唯一

class Role;

/*********************************************/
class StonePackage : public Package
{
public:
	TYPEDEF_PTR(StonePackage)

public:
	StonePackage(SceneItemType sceneItem, PackageType packageType, uint16_t totalCellNum, uint16_t unlockNum, const std::vector<CellInfo>& cellVec);

	StonePackage() = default;
	virtual ~StonePackage() = default;

public:
	void setOwner(std::shared_ptr<PK> owner) override;

	//镶嵌宝石不支持不指定格子
	bool checkPutObj(Object::Ptr obj, uint16_t num, Bind bind) override;
	bool checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind) override;

	//宝石包不支持交换格子
	bool exchangeCell(uint16_t fromCell, uint16_t toCell) override;

	//镶嵌宝石不支持不指定格子
	uint16_t putObj(Object::Ptr obj, uint16_t num, Bind bind, bool newObj = true) override;
	uint16_t putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj = true) override;	

public:
	void sendAllAttributeToMe();

	uint32_t getStoneTotalLevel() const;

private:
	void calculateAllAttribute();
	void resetAllAttribute();


public:
	//获取生命
	uint32_t getHp() const;

	//获取魔法
	uint32_t getMp() const;
	
	//获取生命等级
	uint32_t getHpLv() const;

	//获取魔法等级
	uint32_t getMpLv() const;

	//获取物攻Min
	uint32_t getPAtkMin() const;

	//获取物攻Max
	uint32_t getPAtkMax() const;
	
	//获取魔攻Min
	uint32_t getMAtkMin() const;

	//获取魔攻Max
	uint32_t getMAtkMax() const;

	//获取道术Min
	uint32_t getWitchMin() const;

	//获取道术Max
	uint32_t getWitchMax() const;

	//获取物防Min
	uint32_t getPDefMin() const;

	//获取物防Max
	uint32_t getPDefMax() const;

	//获取魔防Min
	uint32_t getMDefMin() const;

	//获取魔防Max
	uint32_t getMDefMax() const;

	//获取幸运
	uint32_t getLucky() const;

	//获取诅咒
	uint32_t getEvil() const;

	//获取命中
	uint32_t getShot() const;

	//获取命中率
	uint32_t getShotRatio() const;

	//获取物闪
	uint32_t getPEscape() const;

	//获取魔闪
	uint32_t getMEscape() const;

	//获取闪避率
	uint32_t getEscapeRatio() const;

	//获取暴击
	uint32_t getCrit() const;

	//获取暴击率
	uint32_t getCritRatio() const;

	//获取防爆(暴击抗性)
	uint32_t getAntiCrit() const;

	//获取暴伤
	uint32_t getCritDamage() const;

	//获取增伤
	uint32_t getDamageAdd() const;

	//获取增伤等级
	uint32_t getDamageAddLv() const;

	//获取减伤
	uint32_t getDamageReduce() const;

	//获取减伤等级
	uint32_t getDamageReduceLv() const;

    //获取防爆(装备掉落)
    uint32_t getAntiDropEquip() const;

private:
    uint32_t m_hp;			//生命
    uint32_t m_mp;			//魔法
	uint32_t m_hpLv;		//生命等级
	uint32_t m_mpLv;		//魔法等级
	uint32_t m_pAttackMin;	//物攻Min
	uint32_t m_pAttackMax;	//物攻Max
	uint32_t m_mAttackMin;	//魔攻Min
	uint32_t m_mAttackMax;	//魔攻Max
	uint32_t m_witchMin;	//道术Min
	uint32_t m_witchMax;	//道术Max
	uint32_t m_pDefenceMin;	//物防Min
	uint32_t m_pDefenceMax;	//物防Max
	uint32_t m_mDefenceMin;	//魔防Min
	uint32_t m_mDefenceMax;	//魔防Max

	uint32_t m_lucky;		//幸运
	uint32_t m_evil;		//诅咒
	uint32_t m_shot;		//命中
	uint32_t m_shotRatio;	//命中率
	uint32_t m_pEscape;		//物闪
	uint32_t m_mEscape;		//魔闪
	uint32_t m_escapeRatio;	//闪避率
	uint32_t m_crit;		//暴击
	uint32_t m_critRatio;	//暴击率
	uint32_t m_antiCrit;	//防爆(暴击抗性, 坚韧)

	uint32_t m_critDamage;	//暴伤
	uint32_t m_damageAdd;	//增伤
	uint32_t m_damageAddLv;	//增伤等级
	uint32_t m_damageReduce;	//减伤
	uint32_t m_damageReduceLv;	//减伤等级

    uint32_t m_antiDropEquip;

};


}

#endif
