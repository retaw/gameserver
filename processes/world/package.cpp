#include "role.h"
#include "world.h"
#include "package.h"
#include "nonsuch_config.h"
#include "scene.h"
#include "equip_package.h"
#include "role_equip_package.h"
#include "hero_equip_package.h"
#include "stone_package.h"

#include "water/common/commdef.h"
#include "water/componet/logger.h"
#include "water/componet/random.h"
#include "water/process/process_id.h"

#include "protocol/rawmsg/public/package.h"
#include "protocol/rawmsg/public/package.codedef.public.h"

#include "protocol/rawmsg/private/package.h"
#include "protocol/rawmsg/private/package.codedef.private.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

Package::Package(SceneItemType sceneItem, PackageType packageType, uint16_t totalCellNum, uint16_t unlockNum, const std::vector<CellInfo>& cellVec)
: m_sceneItem(sceneItem)
, m_packageType(packageType)
, m_totalCellNum(totalCellNum)
, m_unlockCellNum(unlockNum)
, m_cellVec(cellVec)
{
}

SceneItemType Package::sceneItemType() const
{
	return m_sceneItem;
}

RoleId Package::getRoleId() const
{
	Role::Ptr role = getRole(getOwner());
	if(role == nullptr)
		return 0;
	
	return role->id();
}

void Package::setOwner(std::shared_ptr<PK> owner)
{
	if(owner == nullptr)
		return;

	m_owner = owner;
	return;
}

std::shared_ptr<PK> Package::getOwner() const
{
	PK::Ptr owner = m_owner.lock();
	if(owner == nullptr)
	{
		return nullptr;
	}

	return owner;
}

bool Package::checkPutObj(Object::Ptr obj, uint16_t num, Bind bind)
{
	if(nullptr == obj || 0 == num || bind == Bind::none)
		return false;

	if(m_cellVec.empty())
		return false;

	const uint16_t maxStackNum = obj->maxStackNum();
	if(0 == maxStackNum)
		return false;

	const uint16_t needPutObjNum = num;
	uint16_t totalPutObjNum = 0; 

	for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
	{
		uint16_t putObjNum = SAFE_SUB(needPutObjNum, totalPutObjNum);
		if(0 == putObjNum)
			break;

		if(iter->objPtr == nullptr)
		{
			totalPutObjNum += maxStackNum;
		}
		else if(iter->objPtr->tplId() == obj->tplId() && iter->bind == bind && iter->fixed < 1)
		{
			if(iter->item + putObjNum >= maxStackNum)
			{
				putObjNum = SAFE_SUB(maxStackNum, iter->item);
			}

			totalPutObjNum += putObjNum;
		}
	}

	if(totalPutObjNum >= needPutObjNum)
		return true;

	return false;	
}

bool Package::checkPutObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind)
{
	if(nullptr == obj || 0 == num || bind == Bind::none)
		return false;

	if(m_cellVec.empty())
		return false;

	if(cell >= m_cellVec.size())
		return false;

    if(m_cellVec[cell].fixed >= 1)
        return false;

	const uint16_t maxStackNum = obj->maxStackNum();
	if(num > maxStackNum || 0 == maxStackNum)
		return false;

	const auto iter = m_cellVec[cell];
	if(iter.objPtr == nullptr)
	{
		return true;
	}
	else if(iter.objPtr->tplId() == obj->tplId() 
			&& iter.item + num <= maxStackNum 
			&& iter.bind == bind)
	{
		return true;
	}

	return false;
}

bool Package::exchangeCell(uint16_t fromCell, uint16_t toCell)
{
	if(fromCell >= m_cellVec.size() 
       || toCell >= m_cellVec.size()
       || m_cellVec[fromCell].fixed >= 1
       || m_cellVec[toCell].fixed >= 1)
		return false;

	CellInfo fromObj = m_cellVec[fromCell];
	CellInfo toObj = m_cellVec[toCell];

	{//若是同类物品，且可叠加，则叠加
		if(fromObj.objPtr != nullptr && toObj.objPtr != nullptr)
		{
			if(fromObj.objPtr->tplId() == toObj.objPtr->tplId())
			{

				if(checkPutObjByCell(fromObj.objPtr, toCell, fromObj.item, fromObj.bind))
				{
					eraseObjByCell(fromCell, "交换格子叠加", false, false);
					if(fromObj.item ==  putObjByCell(fromObj.objPtr, toCell, 
													 fromObj.item, fromObj.bind))
					{
						return true;
					}
					return false;
				}
			}
		}
	}

	m_cellVec[toCell] = fromObj;
	m_cellVec[fromCell] = toObj;

	//更新变动的物品
	m_cellVec[toCell].cell = toCell;
	m_cellVec[fromCell].cell = fromCell;
	
	m_cellVec[fromCell].modifyType = ModifyType::modify;
	m_cellVec[toCell].modifyType = ModifyType::modify;

	updateModifyObj(m_cellVec[fromCell]);
	updateModifyObj(m_cellVec[toCell]);
	sendModifyObjListToMe(m_cellVec[fromCell]);
	sendModifyObjListToMe(m_cellVec[toCell]);

	return true;
}


//物品放入背包, 且返回成功放入的物品个数
uint16_t Package::putObj(Object::Ptr obj, uint16_t num, Bind bind, bool newObj/*=true*/)
{
	if(obj == nullptr || 0 == num || bind == Bind::none)
		return 0;

	uint16_t maxStackNum = obj->maxStackNum();
	if(0 == maxStackNum || num > maxStackNum)
		return 0;

	for(uint16_t i = 0; i < m_cellVec.size(); i++)
	{
		if(!putObjInCell(obj, i, num, bind, newObj))
			continue;
        return num;
	}

    //回购背包已满,删除出售时间最早的物品
    if(m_packageType == PackageType::repurchase)
    {
        uint32_t minSellTime = 0;
        uint16_t cell = 0;
        for(uint16_t i = 0; i < m_cellVec.size(); ++i)
        {
            if(0 == minSellTime || minSellTime > m_cellVec[i].sellTime)
            {
                minSellTime = m_cellVec[i].sellTime;
                cell = i;
            }
        }
        auto& cellInfo = m_cellVec[cell];
        cellInfo.modifyType = ModifyType::erase;
        updateModifyObj(cellInfo);

        m_cellVec[cell].objPtr = nullptr;
        putObjInCell(obj, cell, num, bind, newObj);
        return num;
    }
	return 0;
}

uint16_t Package::putObjByCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj/*=true*/)
{
	if(obj == nullptr || 0 == num || bind == Bind::none)
		return 0;

	if(putObjInCell(obj, cell, num, bind, newObj))
		return num;

	return 0;
}

bool Package::putObjInCell(Object::Ptr obj, uint16_t cell, uint16_t num, Bind bind, bool newObj/*=true*/)
{
	if(obj == nullptr || 0 == num || bind == Bind::none)
		return false;

	if(cell >= m_cellVec.size())
		return false;

    if(m_cellVec[cell].fixed >= 1)
        return false;

	uint16_t maxStackNum = obj->maxStackNum();
	if(0 == maxStackNum || num > maxStackNum)
		return false;

	auto& iter = m_cellVec[cell];
    //回购包裹特殊处理,不能叠加
    if(m_packageType == PackageType::repurchase)
    {
        if(iter.objPtr == nullptr)
        {
            iter.objPtr = obj;
            iter.item = num;
            iter.objId = obj->objId();
            iter.cell = cell;
            iter.bind = bind;
            iter.modifyType = ModifyType::modify;
            iter.sellTime = toUnixTime(Clock::now());

            //更新变动的物品
            updateModifyObj(iter);
		    sendModifyObjListToMe(iter);
            return true;
        }
        return false;
    }

	if(iter.objPtr == nullptr)
	{
		iter.objPtr = obj;
		iter.item = num;
		iter.objId = obj->objId();
		iter.cell = cell;
		iter.bind = bind;
		iter.modifyType = ModifyType::modify;
		
		//更新变动的物品
		updateModifyObj(iter);
		sendModifyObjListToMe(iter, newObj);
	}
	else if(iter.objPtr->tplId() == obj->tplId() && iter.bind == bind)
	{
		if(iter.item + num > maxStackNum)
			return false;

		iter.item  = iter.item + num;
		iter.modifyType = ModifyType::modify;
		
		//更新变动的物品
		updateModifyObj(iter);
		sendModifyObjListToMe(iter, newObj);
	}
	else 
	{
		return false;
	}

	Role::Ptr role = getRole(getOwner());
	if(role != nullptr && newObj)
	{
		role->sendSysChat(ChannelType::screen_right_down, "获得 {}*{}", obj->name(), num);
	}
	
	return true;
}

//根据objtypeid尝试去删除num个物品，返回成功删除的个数 
uint16_t Package::tryEraseObj(TplId tplId, uint16_t num, const std::string& text)
{
	if(0 == tplId || 0 == num)
		return 0;

	if(m_cellVec.empty())
		return 0;
	
	uint16_t needEraseNum = num;	//需要删除物品的剩余数量
	uint16_t totalEraseNum = 0;		//累计删除物品的总数量
	for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;
		
		const TplId typeId = iter->objPtr->tplId();
		if(typeId != tplId)
			continue;

        if(iter->fixed >= 1)
            continue;
		
		if(totalEraseNum >= num || 0 == needEraseNum)
			break;
	
		uint16_t eraseNum = 0;		//本次操作删除物品的数量
		if(needEraseNum >= iter->item)
		{
			eraseNum = tryEraseObjByCell(iter->cell, iter->item, text);
			needEraseNum = SAFE_SUB(needEraseNum, eraseNum);
			totalEraseNum += eraseNum;
		}
		else 
		{
			eraseNum = needEraseNum;
			eraseNum = tryEraseObjByCell(iter->cell, needEraseNum, text);
			if(eraseNum == needEraseNum)
			{
				needEraseNum = 0;
			}

			totalEraseNum += eraseNum;
		}
		LOG_TRACE("背包, 删除物品, 指定数量，不指定绑定, 详细, ({}), roleId={}, packageType={}, objId={}, tplId={}, needEraseNum={}, eraseNum={}, totalEraseNum={}",
				  text, getRoleId(), getPackageType(), iter->objId,
				  tplId, num, eraseNum, totalEraseNum);
	}

	LOG_TRACE("背包, 删除物品, 指定数量，不指定绑定, 总体, ({}), roleId={}, packageType={}, tplId={}, needEraseNum={}, totalEraseNum={}",
			  text, getRoleId(), getPackageType(), tplId, num, totalEraseNum);

	return totalEraseNum;
}

uint16_t Package::tryEraseObj(TplId tplId, uint16_t num, Bind bind, const std::string& text)
{
	if(0 == tplId || 0 == num)
		return 0;

	if(m_cellVec.empty())
		return 0;
	
	uint16_t needEraseNum = num;	//需要删除物品的剩余数量
	uint16_t totalEraseNum = 0;		//累计删除物品的总数量
	for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;

        if(bind != iter->bind)
            continue;
		
		const TplId typeId = iter->objPtr->tplId();
		if(typeId != tplId)
			continue;

        if(iter->fixed >= 1)
            continue;
		
		if(totalEraseNum >= num || 0 == needEraseNum)
			break;
	
		uint16_t eraseNum = 0;		//本次操作删除物品的数量
		if(needEraseNum >= iter->item)
		{
			eraseNum = tryEraseObjByCell(iter->cell, iter->item, text);
			needEraseNum = SAFE_SUB(needEraseNum, eraseNum);
			totalEraseNum += eraseNum;
		}
		else 
		{
			eraseNum = needEraseNum;
			eraseNum = tryEraseObjByCell(iter->cell, needEraseNum, text);
			if(eraseNum == needEraseNum)
			{
				needEraseNum = 0;
			}

			totalEraseNum += eraseNum;
		}
		LOG_TRACE("背包, 删除物品, 指定数量，指定绑定, 详细, ({}), roleId={}, packageType={}, objId={}, tplId={}, bind={}, needEraseNum={}, eraseNum={}, totalEraseNum={}",
				  text, getRoleId(), getPackageType(), iter->objId, 
				  tplId, bind, num, eraseNum, totalEraseNum);
	}

	LOG_TRACE("背包, 删除物品, 指定数量，指定绑定, 总体, ({}), roleId={}, packageType={}, tplId={}, bind={}, needEraseNum={}, totalEraseNum={}",
			  text, getRoleId(), getPackageType(), tplId, bind, num, totalEraseNum);

	return totalEraseNum;
}

uint16_t Package::tryEraseObjByCell(uint16_t cell, const std::string& text)
{
	if(cell >= m_cellVec.size())
		return 0;

	return tryEraseObj(cell, m_cellVec[cell].item, text);
}

uint16_t Package::tryEraseObjByCell(uint16_t cell, uint16_t num, const std::string& text)
{
	Role::Ptr role = getRole(getOwner());
	if(role == nullptr)
		return 0;

	if(0 == num || m_cellVec.empty())
		return 0;

	if(cell >= m_cellVec.size())
		return 0;

	auto& iter = m_cellVec[cell];
	if(iter.objPtr == nullptr)
		return 0;
	
    if(iter.fixed >= 1)
        return 0;

	if(num > iter.item)
	{
		return 0;
	}

	uint16_t oldItem = iter.item;
	TplId tplId = iter.objPtr->tplId();
	std::string objName = iter.objPtr->name();

	if(num == iter.item) 
	{
		iter.objPtr = nullptr;
		iter.item = 0;
		iter.bind = Bind::no;
		iter.modifyType = ModifyType::erase;
	}
	else if(num < iter.item)
	{
		iter.item = SAFE_SUB(iter.item, num);
		iter.modifyType = ModifyType::modify;
	}
	
	//更新变动的物品  
	updateModifyObj(iter);
	sendModifyObjListToMe(iter);

	LOG_TRACE("背包, 删除物品, 指定格子，且指定数量, ({}), roleId={}, packageType={}, objId={}, tplId={}, cell={}, item={}->{}, needEraseNum={}",
			  text, getRoleId(), getPackageType(), 
			  iter.objId, tplId, cell, oldItem, iter.item, num);

	role->sendSysChat(ChannelType::screen_right_down, "失去 {}*{}", objName, num);
	return num;
}


//根据tplId删除num个物品，返回删除成功与否 
bool Package::eraseObj(TplId tplId, uint16_t num, const std::string& text)
{
	if(0 == tplId || 0 == num)
		return false;

	uint16_t totalEraseNum = tryEraseObj(tplId, num, text);
	if(totalEraseNum != num)
	{
		LOG_ERROR("背包, 删除物品, 指定数量, 失败, ({}), roleId={}, packageType={}, tplId={}, needEraseNum={}, totalEraseNum={}",
				  text, getRoleId(), getPackageType(), tplId, num, totalEraseNum);
		return false;
	}
	
	LOG_TRACE("背包, 删除物品, 指定数量, 成功, ({}), roleId={}, packageType={}, tplId={}, needEraseNum={}, totalEraseNum={}",
			  text, getRoleId(), getPackageType(), tplId, num, totalEraseNum);
	return true;
}


bool Package::eraseObj(TplId tplId, uint16_t num, Bind bind, const std::string& text)
{
	if(0 == tplId || 0 == num)
		return false;

	uint16_t totalEraseNum = tryEraseObj(tplId, num, bind, text);
	if(totalEraseNum != num)
	{
		LOG_ERROR("背包, 删除物品, 指定数量, 指定绑定, 失败, ({}), roleId={}, packageType={}, tplId={}, bind={}, needEraseNum={}, totalEraseNum={}",
				  text, getRoleId(), getPackageType(), tplId, bind, num, totalEraseNum);
		return false;
	}
	
	LOG_TRACE("背包, 删除物品, 指定数量, 指定绑定, 成功, ({}), roleId={}, packageType={}, tplId={}, bind={}, needEraseNum={}, totalEraseNum={}",
			  text, getRoleId(), getPackageType(), tplId, bind, num, totalEraseNum);
	return true;
}


//根据cell清空一个格子
Object::Ptr Package::eraseObjByCell(uint16_t cell, const std::string& text, bool force/*=false*/, bool notify/*=true*/)
{
	if(cell >= m_cellVec.size())
		return nullptr;

	return eraseObjByCell(cell, m_cellVec[cell].item, text, force, notify);
}

//删除一个cell的num个物品
Object::Ptr Package::eraseObjByCell(uint16_t cell, uint16_t num, const std::string& text, bool force/*=false*/, bool notify/*=true*/)
{
	Role::Ptr role = getRole(getOwner());
	if(role == nullptr)
		return nullptr;

	if(0 == num)
		return nullptr;

	if(cell >= m_cellVec.size())
		return nullptr;

	auto& iter = m_cellVec[cell];
	if(iter.objPtr == nullptr)
		return nullptr;

    if(force)
       iter.fixed = 0;
    else if(iter.fixed >= 1)
        return nullptr;

	if(num > iter.item)
	{
		return nullptr;
	}

	Object::Ptr obj = iter.objPtr;
	std::string name = obj->name();
	TplId tplId = obj->tplId();
	const uint16_t oldItem = iter.item;
	
	if(num == iter.item)
	{
		iter.objPtr = nullptr;
		iter.item = 0;
		iter.bind = Bind::no;
		iter.modifyType = ModifyType::erase;
	}
	else if(num < iter.item)
	{
		iter.item = SAFE_SUB(iter.item, num);
		iter.modifyType = ModifyType::modify;
	}

	//更新变动的物品  
	updateModifyObj(iter);
	sendModifyObjListToMe(iter);

	LOG_TRACE("背包, 删除物品, 指定格子, 指定数量, ({}), roleId={}, packageType={}, name={}, objId={}, tplId={}, item={}->{}, num={}, cell={}",
			  text, getRoleId(), getPackageType(), name, 
			  iter.objId, tplId, oldItem, iter.item, num, cell);

	if(notify)
	{
		role->sendSysChat(ChannelType::screen_right_down, "失去 {}*{}", obj->name(), num);
	}
	return obj;
}

Object::Ptr Package::getObjByCell(uint16_t cell) const
{
	if(cell >= m_cellVec.size())
		return nullptr;

	return m_cellVec[cell].objPtr;
}

uint16_t Package::getObjNum(TplId tplId) const
{
	if(0 == tplId)
		return 0;

	if(m_cellVec.empty())
		return 0;

	uint16_t totalObjNum = 0;
	for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;

		if(iter->objPtr->tplId() != tplId)
			continue;

        if(iter->fixed >= 1)
            continue;

		totalObjNum += iter->item;
	}

	return totalObjNum;
}

uint16_t Package::getObjNumByCell(uint16_t cell) const
{
	if(cell >= m_cellVec.size())
		return 0;

	const auto iter = m_cellVec[cell]; 
	if(iter.objPtr == nullptr)
		return 0;

    if(iter.fixed >= 1)
        return 0;

	return iter.item;
}


uint16_t Package::getUnlockCellNum() const
{
	return m_unlockCellNum;
}

uint16_t Package::getFirstEmptyCellIndex() const
{
	for(uint16_t i = 0; i < m_cellVec.size(); i++)
	{
		if(m_cellVec[i].objPtr != nullptr)
			continue;

		return i;
	}

	return (uint16_t)-1;
}

TplId Package::getTplIdByCell(uint16_t cell) const
{
	if(cell >= m_cellVec.size())
		return 0;

	if(m_cellVec[cell].objPtr == nullptr)
		return 0;

	return m_cellVec[cell].objPtr->tplId();
}

TplId Package::getTplIdByObjChildType(ObjChildType childType) const
{
	for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
	{
		if(iter->objPtr == nullptr)
			continue;

		if(iter->objPtr->childType() != childType)
			continue;

		return iter->objPtr->tplId();
	}

	return 0;
}

PackageType Package::getPackageType() const
{
	return m_packageType;
}

Bind Package::getBindByCell(uint16_t cell) const
{
	if(cell >= m_cellVec.size())
		return Bind::none;

	if(m_cellVec[cell].objPtr == nullptr)
		return Bind::none;

	return m_cellVec[cell].bind;
}

const std::vector<CellInfo>& Package::getCellVec() const
{
	return m_cellVec;
}

bool Package::isEquipPackage(PackageType packageType)
{
	if(packageType == PackageType::equipOfRole 
	   || packageType == PackageType::equipOfWarrior
	   || packageType == PackageType::equipOfMagician
	   || packageType == PackageType::equipOfTaoist)
	{
		return true;
	}

	return false;
}

bool Package::isStonePackage(PackageType packageType)
{
	if(packageType == PackageType::stoneOfRole
	   || packageType == PackageType::stoneOfWarrior
	   || packageType == PackageType::stoneOfMagician
	   || packageType == PackageType::equipOfTaoist)
	{
		return true;
	}

	return false;
}

uint16_t Package::getEmptyCellNum() const
{
    uint16_t cellNum = 0;
    for(const auto& iter : m_cellVec)
    {
        if(nullptr == iter.objPtr)
            cellNum += 1;
    }
    return cellNum;
}

void Package::updateModifyObj(CellInfo info)
{
	if(0 == info.objId)
		return;

	if(info.modifyType == ModifyType::modify && info.objPtr == nullptr)
		return;

	m_modifyObjVec.push_back(info);
	if(!updateModifyObjListToDB())
		return;

	clearModifyObjVec();

	if(isEquipPackage(getPackageType()))
	{
		if(sceneItemType() == SceneItemType::role)
		{
			RoleEquipPackage::Ptr roleEquipPackagePtr = std::static_pointer_cast<RoleEquipPackage>(shared_from_this()); 
			if(roleEquipPackagePtr == nullptr)
				return;

			roleEquipPackagePtr->sendAllAttributeToMe();
			roleEquipPackagePtr->sendObjListToNine();
			roleEquipPackagePtr->updateAllEquipSkills();
		}
		else if(sceneItemType() == SceneItemType::hero)
		{
			HeroEquipPackage::Ptr heroEquipPackagePtr = std::static_pointer_cast<HeroEquipPackage>(shared_from_this());
			if(heroEquipPackagePtr == nullptr)
				return;

            if(2 == info.cell)
            {
                Role::Ptr role = getRole(getOwner());
                if(nullptr != role)
                {
                    if(info.modifyType == ModifyType::modify)
                        role->m_heroManager.updateDefaultHeroClother(info.objPtr->tplId());
                    else if(info.modifyType == ModifyType::erase)
                        role->m_heroManager.updateDefaultHeroClother(0);
                }
            }

			heroEquipPackagePtr->sendAllAttributeToMe();
			heroEquipPackagePtr->sendObjListToNine();
			heroEquipPackagePtr->updateAllEquipSkills();			
		}

	}
	else if(isStonePackage(getPackageType()))
	{
		StonePackage::Ptr packagePtr = std::static_pointer_cast<StonePackage>(shared_from_this());
		if(packagePtr == nullptr)
			return;

		packagePtr->sendAllAttributeToMe();
	}

	return;
}

void Package::clearModifyObjVec()
{
	m_modifyObjVec.clear();
	return;
}


//将变动的物品更新到数据库
bool Package::updateModifyObjListToDB()
{
	PK::Ptr owner = getOwner();
	if(owner == nullptr)
	{
		LOG_ERROR("背包, PK::WPtr失效, 发送存储背包数据消息失败");
		return false;
	}

	if(m_modifyObjVec.empty())
		return false;

	const RoleId roleId = getRoleId();
	if(0 == roleId)
		return false;

	std::vector<uint8_t> buf;
	buf.reserve(512);
	buf.resize(sizeof(PrivateRaw::RetModifyObjData));

	auto* msg  = reinterpret_cast<PrivateRaw::RetModifyObjData*>(buf.data());
	msg->roleId = roleId;
	msg->objListSize = 0;

	for(auto iter = m_modifyObjVec.begin(); iter != m_modifyObjVec.end(); ++iter)
	{
		buf.resize(buf.size() + sizeof(PrivateRaw::RetModifyObjData::ModifyObj));
		auto* msg  = reinterpret_cast<PrivateRaw::RetModifyObjData*>(buf.data());

		TplId tplId = 0;
		uint32_t skillId = 0;
		uint8_t strongLevel = 0;
		uint8_t luckyLevel = 0;
		if(iter->objPtr != nullptr)
		{
			tplId = iter->objPtr->tplId();
			skillId = iter->objPtr->skillId();
			strongLevel = iter->objPtr->strongLevel();
			luckyLevel = iter->objPtr->luckyLevel();
		}

		msg->data[msg->objListSize].objId = iter->objId;
		msg->data[msg->objListSize].modifyType = iter->modifyType;
		msg->data[msg->objListSize].packageType = m_packageType;
		msg->data[msg->objListSize].cell = iter->cell;
		msg->data[msg->objListSize].tplId = tplId;
		msg->data[msg->objListSize].item = iter->item; 
		msg->data[msg->objListSize].skillId = skillId;
		msg->data[msg->objListSize].bind = iter->bind;
        msg->data[msg->objListSize].sellTime = iter->sellTime;
		msg->data[msg->objListSize].strongLevel = strongLevel;
		msg->data[msg->objListSize].luckyLevel = luckyLevel;

		LOG_TRACE("背包, 将变动的物品更新到数据库, 详细, sceneItem={}, roleId={}, objId={}, modifyType={}, packageType={}, cell={}, tplId={}, item={}, skillId={}, bind={}, strongLevel={}, luckLevel={}, objListSize={}, owner=({}, {})",
				  sceneItemType(), msg->roleId,
				  msg->data[msg->objListSize].objId,
				  msg->data[msg->objListSize].modifyType,
				  msg->data[msg->objListSize].packageType, 
				  msg->data[msg->objListSize].cell, 
				  msg->data[msg->objListSize].tplId,
				  msg->data[msg->objListSize].item,
				  msg->data[msg->objListSize].skillId,
				  msg->data[msg->objListSize].bind,
				  msg->data[msg->objListSize].strongLevel,
				  msg->data[msg->objListSize].luckyLevel,
				  msg->objListSize,
				  owner->name(), owner->id());

		msg->objListSize++;
	}
	
	ProcessIdentity dbcachedId("dbcached", 1);
	const bool ret = World::me().sendToPrivate(dbcachedId, RAWMSG_CODE_PRIVATE(RetModifyObjData), buf.data(), buf.size());

	return ret; 
}


//将变动的物品列表发送给自己
bool Package::sendModifyObjListToMe(CellInfo info, bool newObj/*=false*/)
{
	PK::Ptr owner = getOwner();
	if(owner == nullptr)
		return false;

	PublicRaw::RetPackageModifyObjList send;
	
	TplId tplId = 0;
	uint32_t skillId = 0;
	uint8_t strongLevel = 0;
	uint8_t luckyLevel = 0;
	ObjParentType parentType = (ObjParentType)0;

	if(info.objPtr != nullptr)
	{
		tplId = info.objPtr->tplId();
		skillId = info.objPtr->skillId();
		strongLevel = info.objPtr->strongLevel();
		luckyLevel = info.objPtr->luckyLevel();
		parentType = info.objPtr->parentType();
	}

	send.tplId =  tplId;
	send.item = info.item; 
	send.cell = info.cell;
	send.skillId = skillId;
	send.strongLevel = strongLevel;
	send.luckyLevel = luckyLevel;
	send.bind = info.bind;
	send.fixed = info.fixed;
	send.parentType = parentType;
	send.packageType = getPackageType();
	send.bNewObj = newObj;

	owner->sendToMe(RAWMSG_CODE_PUBLIC(RetPackageModifyObjList), &send, sizeof(send));
	LOG_TRACE("背包, 变动物品, 发送给自己, sceneItem={}, roleId={}, packageType={}, tplId={}, item={}, cell={}, skillId={}, bind={}, strongLevel={}, luckyLevel={}, parentType={}, owner=({}, {}))",
			  sceneItemType(), getRoleId(), send.packageType, send.tplId, send.item, send.cell, 
			  send.skillId, send.bind, send.strongLevel, send.luckyLevel, send.parentType, 
			  send.bNewObj, owner->name(), owner->id());

	return true; 
}


//将物品列表发送给自己
bool Package::sendObjListToMe()
{
	PK::Ptr owner = getOwner();
	if(owner == nullptr)
	{
		LOG_ERROR("背包, PK::WPtr失效, 发送背包物品列表消息失败");
		return false;
	}

	if(m_cellVec.empty())
		return false;

	std::vector<uint8_t> buf;
	fillObjList(&buf);
	
	owner->sendToMe(RAWMSG_CODE_PUBLIC(RetPackageObjList), buf.data(), buf.size());
	return true; 
}

void Package::fillObjList(std::vector<uint8_t>* buf)
{
	PK::Ptr owner = getOwner();
	if(owner == nullptr)
		return;

	buf->clear();
	buf->reserve(4096);
	buf->resize(sizeof(PublicRaw::RetPackageObjList));
	
	auto* msg  = reinterpret_cast<PublicRaw::RetPackageObjList*>(buf->data());
	msg->packageType = m_packageType;
	msg->objListSize = 0;

    if(m_packageType == PackageType::repurchase)
    {
        std::vector<CellInfo> sendVec = m_cellVec;
        struct SellTimeComp
        {
            bool operator() (const CellInfo& lhs, const CellInfo& rhs) const
            {
                return lhs.sellTime < rhs.sellTime;
            } 
        };
        std::sort(sendVec.begin(), sendVec.end(), SellTimeComp());
        for(auto iter = sendVec.begin(); iter != sendVec.end(); ++iter)
        {
            if(iter->objPtr == nullptr)
                continue;

            buf->resize(buf->size() + sizeof(PublicRaw::RetPackageObjList::ObjectData));
			auto* msg  = reinterpret_cast<PublicRaw::RetPackageObjList*>(buf->data());

            msg->data[msg->objListSize].tplId = iter->objPtr->tplId();
            msg->data[msg->objListSize].item = iter->item; 
            msg->data[msg->objListSize].cell = iter->cell;
            msg->data[msg->objListSize].skillId = iter->objPtr->skillId();
            msg->data[msg->objListSize].strongLevel = iter->objPtr->strongLevel();
			msg->data[msg->objListSize].luckyLevel = iter->objPtr->luckyLevel();
			msg->data[msg->objListSize].bind = iter->bind;
            msg->data[msg->objListSize].fixed = iter->fixed;
			msg->data[msg->objListSize].parentType = iter->objPtr->parentType();

            ++msg->objListSize;
        }
    }
	else
	{
		for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
		{
			if(iter->objPtr == nullptr)
				continue;

			buf->resize(buf->size() + sizeof(PublicRaw::RetPackageObjList::ObjectData));
			auto* msg  = reinterpret_cast<PublicRaw::RetPackageObjList*>(buf->data());

			msg->data[msg->objListSize].tplId = iter->objPtr->tplId();
			msg->data[msg->objListSize].item = iter->item; 
			msg->data[msg->objListSize].cell = iter->cell;
			msg->data[msg->objListSize].skillId = iter->objPtr->skillId();
            msg->data[msg->objListSize].strongLevel = iter->objPtr->strongLevel();
			msg->data[msg->objListSize].luckyLevel = iter->objPtr->luckyLevel();
			msg->data[msg->objListSize].bind = iter->bind;
            msg->data[msg->objListSize].fixed = iter->fixed;
			msg->data[msg->objListSize].parentType = iter->objPtr->parentType();

			LOG_TRACE("背包, 指定类型, 获取物品列表, 详细, sceneItem={}, roleId={}, packageType={}, objListSize={}, tplId={}, item={}, cell={}, bind={}, parentType={}, skillId={}, strongLevel={}, luckyLevel={}, owner=({}, {})",
					  sceneItemType(), getRoleId(), 
					  msg->packageType, msg->objListSize,
					  msg->data[msg->objListSize].tplId,
					  msg->data[msg->objListSize].item,
					  msg->data[msg->objListSize].cell, 
					  msg->data[msg->objListSize].bind,
					  msg->data[msg->objListSize].parentType,
					  msg->data[msg->objListSize].skillId,
					  msg->data[msg->objListSize].strongLevel,
					  msg->data[msg->objListSize].luckyLevel,
					  owner->name(), owner->id());

			++msg->objListSize;
		}
		LOG_TRACE("背包, 指定类型, 获取物品列表, 总体, sceneItem={}, roleId={}, packageType={}, objListSize={}, bufSize={}, owner=({}, {})",
				  sceneItemType(), getRoleId(), msg->packageType, msg->objListSize, buf->size(),
				  owner->name(), owner->id());
	}

	return; 
}

//解锁格子
bool Package::unlockCell(uint8_t num) 
{
	if(m_cellVec.empty())
		return false;

	if(m_cellVec.size() >= m_totalCellNum)
		return false;
	
	if(m_cellVec.size() != m_unlockCellNum)
		return false;

	if(m_cellVec.size() + num > m_totalCellNum)
		return false;

	const uint16_t size = m_cellVec.size() + num;
	m_cellVec.resize(size);
	m_unlockCellNum += num;
	
	return true;
}

void Package::sortObj()
{
	if(getPackageType() != PackageType::role && getPackageType() != PackageType::storage)
		return;

	Role::Ptr role = std::static_pointer_cast<Role>(getOwner());
	if(role == nullptr)
		return;

    //摆摊不能整理背包
    if(role->isStall())
        return;

	//遍历背包, 将可叠加的物品进行叠加，预处理
	bool error = true;
	const std::vector<CellInfo>& tempVec = putObjInVector(error);
	if(error)
		return;

	//清空db中的背包数据
	for(auto pos = m_cellVec.begin(); pos != m_cellVec.end(); ++pos)
	{
		if(0 == pos->objId || nullptr == pos->objPtr)
			continue;

		pos->modifyType = ModifyType::erase;
		updateModifyObj(*pos);
	}
	
	//对叠加处理后的m_cellVec进行排序
	m_cellVec = tempVec;
	std::sort(m_cellVec.begin(), m_cellVec.end(), compareGreater);

	for(uint16_t i = 0; i < m_cellVec.size(); i++)
	{
		auto& iter = m_cellVec[i];
		if(0 == iter.objId || nullptr == iter.objPtr)
			continue;

		iter.cell = i;
		iter.modifyType = ModifyType::modify;
		updateModifyObj(iter);
	}

	sendObjListToMe();
}

bool Package::compareGreater(const CellInfo& lsh, const CellInfo& rsh)
{
	if(lsh.objPtr == nullptr || rsh.objPtr == nullptr)
		return lsh.item > rsh.item;

	if(lsh.objPtr->parentType() == rsh.objPtr->parentType())
	{
		if(lsh.objPtr->quality() == rsh.objPtr->quality())
		{
			if(lsh.objPtr->childType() == rsh.objPtr->childType())
				return lsh.item > rsh.item;

			return lsh.objPtr->childType() < rsh.objPtr->childType();
		}
		else
		{
			return lsh.objPtr->quality() > rsh.objPtr->quality();
		}
	}
	else
	{
		return lsh.objPtr->parentType() < rsh.objPtr->parentType();
	}

	return false;
}

//遍历m_cellVec，并返回叠加后的vector
std::vector<CellInfo> Package::putObjInVector(bool& error) const
{
	std::vector<CellInfo> tempVec;
	tempVec.resize(m_cellVec.size());

	for(auto iter = m_cellVec.begin(); iter != m_cellVec.end(); ++iter)
	{
		if(0 == iter->objId || nullptr == iter->objPtr)
			continue;
		
		if(iter->bind == Bind::none)
			continue;

		uint16_t maxStackNum = iter->objPtr->maxStackNum();
		if(0 == maxStackNum)
			continue;

		uint16_t totalPutObjNum = 0; 
		for(uint16_t i = 0; i < tempVec.size(); i++)
		{
			uint16_t needPutObjNum = SAFE_SUB(iter->item, totalPutObjNum);
			if(0 == needPutObjNum)
				break;

			if(tempVec[i].objPtr == nullptr)
			{
				tempVec[i].objPtr = iter->objPtr;
				tempVec[i].item = needPutObjNum;
				tempVec[i].objId = iter->objPtr->objId();
				tempVec[i].cell = i;
				tempVec[i].bind = iter->bind;
				tempVec[i].modifyType = ModifyType::modify;

				totalPutObjNum += needPutObjNum;
				continue;
			}
			else if(tempVec[i].objPtr->tplId() == iter->objPtr->tplId() 
					&& tempVec[i].bind == iter->bind)
			{
				if(tempVec[i].item + needPutObjNum >= maxStackNum)
				{
					needPutObjNum = SAFE_SUB(maxStackNum, tempVec[i].item);
				}
				
				tempVec[i].item  += needPutObjNum;
				tempVec[i].modifyType = ModifyType::modify;

				totalPutObjNum += needPutObjNum;
				continue;
			}
		}

		if(iter->item > totalPutObjNum)
		{
			error = true;
			break;
		}
		else
		{
			error = false;	
		}
	}

	return tempVec;
}


void Package::execCell(std::function<bool (CellInfo& cellInfo)> exec)
{
    for(auto& iter : m_cellVec)
    {
        if(!exec(iter))
            return;
    }
}


void Package::fixCell(uint16_t cell)
{
    if(cell >= m_cellVec.size())
        return;

	if(nullptr == m_cellVec[cell].objPtr)
        return;
    
	m_cellVec[cell].fixed = 1;
	sendModifyObjListToMe(m_cellVec[cell]);	
	return;
}

void Package::cancelFixCell(uint16_t cell)
{
	if(cell >= m_cellVec.size())
		return;

	if(nullptr == m_cellVec[cell].objPtr)
		return;

	m_cellVec[cell].fixed = 0;
	sendModifyObjListToMe(m_cellVec[cell]);	
	return;
}

bool Package::isCellFixed(uint16_t cell) const
{
    if(cell >= m_cellVec.size())
        return true;

    return m_cellVec[cell].fixed >= 1;
}


bool Package::setStrongLevel(uint16_t cell, uint8_t level)
{
	if(!isEquipPackage(getPackageType()))
		return false;

	if(cell >= m_cellVec.size())
		return false;

	Object::Ptr objPtr = m_cellVec[cell].objPtr;
	if(objPtr == nullptr)
		return false;

	uint8_t oldLevel = objPtr->strongLevel();
	objPtr->setStrongLevel(level);
	m_cellVec[cell].modifyType = ModifyType::modify;
	
	updateModifyObj(m_cellVec[cell]);
	sendModifyObjListToMe(m_cellVec[cell]);	
	LOG_TRACE("强化, {}, roleId={}, sceneItem={}, strongLevel={}->{}",
			  level > oldLevel ? "成功" : "失败", getRoleId(), m_sceneItem, oldLevel, level);
	return true;
}

uint8_t Package::getStrongLevelByCell(uint16_t cell) const
{
	if(cell >= m_cellVec.size())
		return (uint8_t)-1;

	if(m_cellVec[cell].objPtr == nullptr)
		return (uint8_t)-1;

	return m_cellVec[cell].objPtr->strongLevel();
}

bool Package::setWeaponLuckyLevel(uint8_t level)
{
	//武器在第0格
	const uint8_t cell = 0;

	if(!isEquipPackage(getPackageType()))
		return false;

	if(cell >= m_cellVec.size())
		return false;

	Object::Ptr objPtr = m_cellVec[cell].objPtr;
	if(objPtr == nullptr)
		return false;

	uint8_t oldLevel = objPtr->luckyLevel();
	objPtr->setLuckyLevel(level);
	m_cellVec[cell].modifyType = ModifyType::modify;
	
	updateModifyObj(m_cellVec[cell]);
	sendModifyObjListToMe(m_cellVec[cell]);	
	LOG_TRACE("武器幸运, {}, roleId={}, sceneItem={}, luckyLevel={}->{}",
			  level > oldLevel ? "成功" : "失败", getRoleId(), m_sceneItem, oldLevel, level);
	return true;
}

uint8_t Package::getWeaponLuckyLevel() const
{
	//武器在第0格
	const uint8_t cell = 0;

	if(cell >= m_cellVec.size())
		return (uint8_t)-1;

	if(m_cellVec[cell].objPtr == nullptr)
		return (uint8_t)-1;

	return m_cellVec[cell].objPtr->luckyLevel();
}


}
