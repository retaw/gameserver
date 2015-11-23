/*
 * Author: zhupengfei
 *
 * Created: 2015-04-13 +0800  
 *
 * Modified: 2015-04-13 +0800
 *
 * Description: 背包管理
 */

#ifndef PROCESS_WORLD_PACKAGE_SET_HPP
#define PROCESS_WORLD_PACKAGE_SET_HPP

#include "package.h"
#include "pk.h"

#include "water/common/roledef.h"
#include "water/componet/exception.h"
#include "water/componet/class_helper.h"
#include "water/process/process_id.h"

#include "protocol/rawmsg/private/role_scene.h" 
#include "protocol/rawmsg/private/role_scene.codedef.private.h"

#include <memory>
#include <vector>
#include <map>

namespace world{

using namespace water;
using water::process::ProcessIdentity;

DEFINE_EXCEPTION(CreatePackageSetFailed, componet::ExceptionBase)  

class Role;

class PackageSet
{
public:
	PackageSet(SceneItemType sceneItem, Job job, RoleId roleId, const uint8_t unlockCellNumOfRole = 0, const uint8_t unlockCellNumOfHero = 0, const uint8_t unlockCellNumOfStorage = 0);
	~PackageSet() = default;

private:
	void init(SceneItemType sceneItem);

public:
	void loadFromDB(const std::vector<RoleObjData::ObjData> objVec);

private:
	//创建背包
	void createPackage(const std::map<PackageType, std::vector<RoleObjData::ObjData> >& packageMap);
	
	uint32_t getSkillIdByNonsuchId(uint32_t nonsuchId) const; 

	PackageType getHeroEquipPackageType() const;

public:
	void setOwner(std::shared_ptr<PK> owner);
	std::shared_ptr<PK> getOwner() const;

	SceneItemType sceneItemType() const;
	
	Object::Ptr createObj(TplId tplId, const ObjectId objId = 0, const uint32_t skillId = (uint32_t)-1, const uint8_t strongLevel = 0, const uint8_t luckyLevel = 0) const;

private:
	void updateUnlockCellNumTODB();

	std::shared_ptr<Role> getRole() const;
	RoleId getRoleId() const;

public:
	Package::Ptr getPackageByPackageType(PackageType packageType) const;  
	
    uint16_t getEmptyCellNum(PackageType packageType) const;
	uint16_t getUnlockCellNum(PackageType packageType) const;

private:
	uint16_t getTotalCellNum(PackageType packageType) const;

	void setUnlockCellNum(uint16_t num, PackageType packageType);


public:
	bool checkPutObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType); 
	bool checkPutObjByCell(TplId tplId, uint16_t cell, uint16_t num, Bind bind, PackageType packageType);

private:
	bool checkPutObj(Object::Ptr obj, uint16_t num, Bind bind, PackageType packageType); 
	bool checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, PackageType packageType);

public:
	bool exchangeCell(uint16_t fromCell, uint16_t toCell, PackageType packageType);	

	uint16_t putObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType, uint32_t skillId = (uint32_t)-1, uint8_t strongLevel = 0, uint8_t luckyLevel = 0, bool newObj = true);

	uint16_t putObjByCell(TplId tplId, uint16_t cell, uint16_t num, Bind bind, PackageType packageType, uint32_t skillId = (uint32_t)-1, uint8_t strongLevel = 0, uint8_t luckyLevel = 0, bool newObj = true);

private:
	uint16_t putObj(Object::Ptr obj, uint16_t num, Bind bind, PackageType packageType, bool newObj = true);
	uint16_t putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, PackageType packageType, bool newObj = true);

	uint16_t tryEraseObj(TplId tplId, uint16_t num, PackageType packageType, const std::string& text);
	
	uint16_t tryEraseObjByCell(uint16_t cell, PackageType packageType, const std::string& text);
	uint16_t tryEraseObjByCell(uint16_t cell, uint16_t num, PackageType packageType, const std::string& text);

public:
	bool eraseObj(TplId tplId, uint16_t num, PackageType packageType, const std::string& text);
    bool eraseObj(TplId tplId, uint16_t num, Bind bind, PackageType packageType, const std::string& text);

	Object::Ptr eraseObjByCell(uint16_t cell, PackageType packageType, const std::string& text, bool notify = true);
	Object::Ptr eraseObjByCell(uint16_t cell, uint16_t num, PackageType packageType, const std::string& text, bool notify = true);

    Object::Ptr eraseFixedCellObj(uint16_t cell, PackageType packageType, const std::string& text);


public:
	uint16_t getObjNum(TplId tplId, PackageType packageType) const;   
	uint16_t getObjNumByCell(uint16_t cell, PackageType packageType) const;
	Object::Ptr getObjByCell(uint16_t cell, PackageType packageType) const;
	Bind getBindByCell(uint16_t cell, PackageType packageType) const;

	void fixCell(uint16_t cell, PackageType packageType);
	void cancelFixCell(uint16_t cell, PackageType packageType);
	bool isCellFixed(uint16_t cell, PackageType packageType) const; 

private:
	TplId getTplIdByCell(uint16_t cell, PackageType packageType) const;

	bool isCanDiscard(uint16_t cell, PackageType packageType) const;

	bool isRoleEquipPackage(PackageType packageType) const;
	bool isHeroEquipPackage(PackageType packageType) const;
	bool isStonePackage(PackageType packageType) const;

public:
	TplId getTplIdByObjChildType(ObjChildType childType, PackageType packageType) const;
	
public:
	/******************************** 获取装备包属性 begin ****************************/
	//获取生命
	uint32_t getHp(PackageType packageType) const;

	//获取魔法
	uint32_t getMp(PackageType packageType) const;

	//获取生命等级
	uint32_t getHpLv(PackageType packageType) const;

	//获取魔法等级
	uint32_t getMpLv(PackageType packageType) const;

	//获取物攻Min
	uint32_t getPAtkMin(PackageType packageType) const;

	//获取物攻Max
	uint32_t getPAtkMax(PackageType packageType) const;
	
	//获取魔攻Min
	uint32_t getMAtkMin(PackageType packageType) const;

	//获取魔攻Max
	uint32_t getMAtkMax(PackageType packageType) const;

	//获取道术Min
	uint32_t getWitchMin(PackageType packageType) const;

	//获取道术Max
	uint32_t getWitchMax(PackageType packageType) const;

	//获取物防Min
	uint32_t getPDefMin(PackageType packageType) const;

	//获取物防Max
	uint32_t getPDefMax(PackageType packageType) const;

	//获取魔防Min
	uint32_t getMDefMin(PackageType packageType) const;

	//获取魔防Max
	uint32_t getMDefMax(PackageType packageType) const;

	//获取幸运
	uint32_t getLucky(PackageType packageType) const;

	//获取诅咒
	uint32_t getEvil(PackageType packageType) const; 

	//获取命中
	uint32_t getShot(PackageType packageType) const;

	//获取命中率
	uint32_t getShotRatio(PackageType packageType) const; 

	//获取物闪
	uint32_t getPEscape(PackageType packageType) const;

	//获取魔闪
	uint32_t getMEscape(PackageType packageType) const;

	//获取闪避率
	uint32_t getEscapeRatio(PackageType packageType) const;  

	//获取暴击
	uint32_t getCrit(PackageType packageType) const;

	//获取暴击率
	uint32_t getCritRatio(PackageType packageType) const; 

	//获取防爆(暴击抗性, 坚韧)
	uint32_t getAntiCrit(PackageType packageType) const;

	//获取暴伤
	uint32_t getCritDamage(PackageType packageType) const;

	//获取增伤
	uint32_t getDamageAdd(PackageType packageType) const; 

	//获取增伤等级
	uint32_t getDamageAddLv(PackageType packageType) const;

	//获取减伤
	uint32_t getDamageReduce(PackageType packageType) const;

	//获取减伤等级
	uint32_t getDamageReduceLv(PackageType packageType) const;

    //防爆装备属性
    uint32_t getAntiDropEquip(PackageType packageType) const;

	/******************************** 获取装备包属性 end ****************************/



public:
	/********************************* 处理背包操作消息 begin *****************************/
	//返回背包物品列表
	void sendObjList(PackageType packageType);

	//返回背包已解锁的格子数
	void sendUnlockCellNum(PackageType packageType);

	//请求摧毁一定数量的某类型物品, 且指定格子
	void requestDestoryObjByCell(uint16_t cell, uint16_t num, PackageType packageType);

	//请求丢弃一定数量的某类型物品, 且指定格子
	void requestDiscardObjByCell(uint16_t cell, uint16_t num, PackageType packageType);

	//请求拆分物品数量
	void requestSplitObjNum(uint16_t cell, uint16_t num, PackageType packageType);

	//请求交换格子, 支持同一背包及跨背包
	void requestExchangeCell(uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage);

	//请求存取物品，跨背包, 不指定格子
	void requestMoveObj(uint16_t fromCell, PackageType fromPackage, PackageType toPackage);

	//请求整理物品
	void requestSortObj(PackageType packageType);

    //请求出售物品
    void requestSellObj(uint16_t fromCell, uint16_t num, PackageType fromPackage);

    //请求回购物品
    void requestRepurchaseObj(uint16_t fromCell);

	//返回正在解锁的格子剩余解锁时间(主背包)
	void sendUnlockCellNeedSec();

	//请求解锁格子需要的元宝数
	void requestUnlockCellNeedMoney(uint16_t num, PackageType packageType);

	//请求解锁格子
	void requestUnlockCell(uint16_t num, PackageType packageType);

private:
	bool mergerObj(uint16_t fromCell, uint16_t toCell, PackageType packageType);
	
	//移动物品，跨背包
	void moveObj(uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage);

	//移动物品到宝石背包, 特殊处理
	void moveObjToStonePackage(uint16_t fromCell, uint16_t toCell, PackageType fromPackage, PackageType toPackage);

	//解锁主背包格子
	void unlockCellOfRole(uint16_t num);

	//解锁仓库格子
	void unlockCellOfStorage(uint16_t num);

	//获取解锁格子需要的元宝数
	uint32_t getUnlockCellNeedMoney(uint16_t num, uint32_t& addExp);

	/********************************* 处理背包操作消息 end *******************************/


public:
	/********************************* 处理场景上的物品 *******************************/
	//请求拾取物品
	void requestPickupObject(ObjectId objIdOfScene);


public:
    void execPackageCell(PackageType packageType, std::function<bool (CellInfo& cellInfo)>);

private:
	const SceneItemType m_sceneItem;
	const Job m_job;
	const RoleId m_roleId;
	uint16_t m_unlockCellNumOfRole;		//角色已解锁的格子数
	uint16_t m_unlockCellNumOfHero;		//英雄已解锁的格子数 
	uint16_t m_unlockCellNumOfStorage;	//仓库已解锁的格子数
	TimePoint m_sortObjTimePoint;		//整理物品时间点

	uint8_t m_packageNum = 0; //角色背包数量(主背包、英雄背包、仓库、角色装备包) 及英雄装备包

private:
	std::weak_ptr<PK> m_owner;	

private:
	std::map<PackageType, Package::Ptr> m_packageMap;
};


}
#endif
