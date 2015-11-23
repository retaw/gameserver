/*
 * Author: zhupengfei
 *
 * Created: 2015-04-13 +0800
 *
 * Modified: 2015-04-13 +0800
 *
 * Description: 背包基类
 */

#ifndef PROCESS_WORLD_PACKAGE_HPP
#define PROCESS_WORLD_PACKAGE_HPP

#include "object.h"
#include "pk.h"

#include "water/common/roledef.h"
#include "water/componet/class_helper.h"

#include <memory>
#include <vector>

namespace world{

struct CellInfo
{
	Object::Ptr objPtr = nullptr;
	uint64_t objId = 0;		//相对于role唯一的objId
	uint16_t item = 0;		//一个格子存放此物品的总数
	uint16_t cell = 0;
	Bind bind = Bind::none;
	ModifyType modifyType = ModifyType::none; 
    uint32_t sellTime = 0;  //被出售时间(放入回购背包时间)
    uint8_t fixed = 0;  //1:表示该格子物品被锁定,不能有任何对该格子物品的操作
};

//向packge中put obj 时，需要多少个格子由packageSet来处理
//需要多个格子，则对应create多个 Object::Ptr
//确保package中每个格子中的Object::Ptr唯一


/*********************************************/
class Package : public std::enable_shared_from_this<Package>
{
public:
	TYPEDEF_PTR(Package);
	
public:
	explicit Package(SceneItemType sceneItem, PackageType packageType, uint16_t totalCellNum, uint16_t unlockNum, const std::vector<CellInfo>& cellVec);

	Package() = default;
	virtual ~Package() = default;

protected:
	SceneItemType sceneItemType() const;
	RoleId getRoleId() const;

public:
	virtual void setOwner(std::shared_ptr<PK> owner);
	std::shared_ptr<PK> getOwner() const;

	virtual bool checkPutObj(Object::Ptr obj, uint16_t num, Bind bind);
	virtual bool checkPutObjByCell(Object::Ptr, uint16_t cell, uint16_t num, Bind bind);

	virtual bool exchangeCell(uint16_t fromCell, uint16_t toCell);	

	virtual uint16_t putObj(Object::Ptr obj, uint16_t num, Bind bind, bool newObj = true);
	virtual uint16_t putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj = true);	

protected:
	bool putObjInCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj=true);

public:
	//删除物品, 返回成功删除的个数
	uint16_t tryEraseObj(TplId tplId, uint16_t num, const std::string& text);
    uint16_t tryEraseObj(TplId tplId, uint16_t num, Bind bind, const std::string& text);
	uint16_t tryEraseObjByCell(uint16_t cell, const std::string& text);
	uint16_t tryEraseObjByCell(uint16_t cell, uint16_t num, const std::string& text);

	//根据tplId删除num个物品，返回删除成功与否
	bool eraseObj(TplId tplId, uint16_t num, const std::string& text);
    bool eraseObj(TplId tplId, uint16_t num, Bind bind, const std::string& text);

	//根据cell清空一个格子
	Object::Ptr eraseObjByCell(uint16_t cell, const std::string& text, bool force=false, bool notify = true);	

	//删除一个cell的num个物品
	Object::Ptr eraseObjByCell(uint16_t cell, uint16_t num, const std::string& text, bool force=false, bool notify = true);

public:
	Object::Ptr getObjByCell(uint16_t cell) const;
	uint16_t getObjNum(TplId tplId) const;
	uint16_t getObjNumByCell(uint16_t cell) const;

	uint16_t getUnlockCellNum() const;
	uint16_t getFirstEmptyCellIndex() const;

	TplId getTplIdByCell(uint16_t cell) const; 
	TplId getTplIdByObjChildType(ObjChildType childType) const; 

	PackageType getPackageType() const;
	Bind getBindByCell(uint16_t cell) const;

	bool isEquipPackage(PackageType packageType);
	bool isStonePackage(PackageType packageType);

    uint16_t getEmptyCellNum() const;

protected:
	const std::vector<CellInfo>& getCellVec() const;

	void updateModifyObj(CellInfo info);

private:
	void clearModifyObjVec();

	//将变动的物品列表更新到数据库
	bool updateModifyObjListToDB();

	//将变动的物品列表发送给自己
	bool sendModifyObjListToMe(CellInfo info, bool newObj = false);

public:
	//将物品列表发送给自己
	bool sendObjListToMe();

	void fillObjList(std::vector<uint8_t>* buf);

	//解锁格子
	bool unlockCell(uint8_t num);

	//物品排序
	void sortObj();

    //遍历格子,回调处理函数
    void execCell(std::function<bool (CellInfo& cellInfo)>);

    //固定格子(置灰,不能对该格子操作)
    void fixCell(uint16_t cell);
    void cancelFixCell(uint16_t cell);
	bool isCellFixed(uint16_t cell) const;

private:
	//降序排序
	static bool compareGreater(const CellInfo& lsh, const CellInfo& rsh);

	//遍历m_cellVec，并返回叠加后的vector
	std::vector<CellInfo> putObjInVector(bool& error) const;

public:
	bool setStrongLevel(uint16_t cell, uint8_t level);
	uint8_t getStrongLevelByCell(uint16_t cell) const;

	bool setWeaponLuckyLevel(uint8_t level);
	uint8_t getWeaponLuckyLevel() const;

private:
	const SceneItemType m_sceneItem;				//属于role或hero
	const PackageType m_packageType;	//背包类型 0角色 1英雄 2仓库
	const uint16_t m_totalCellNum;		//总格子数
	uint16_t m_unlockCellNum;			//已解锁的格子数
	std::vector<CellInfo> m_cellVec;	//背包的所有格子

private:
	std::weak_ptr<PK> m_owner;				//PK::WPtr
	std::vector<CellInfo> m_modifyObjVec;	//<ObjId, CellInfo>变动的obj
	
};


}

#endif
