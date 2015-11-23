/**
 **
 ** buff模块
 **
 **
 **/

#ifndef PROCESS_BUFF_MANAGER_H
#define PROCESS_BUFF_MANAGER_H

#include <memory>
#include <unordered_map>
#include <string.h>
#include "water/componet/datetime.h"
#include "water/common/roledef.h"
#include "config_table_manager.h"
#include "attribute.h"

namespace world{

class PK;

//buff单元
struct BuffElement : public BuffData
{
    BuffElement()
    {
        std::memset(this, 0, sizeof(*this));
    }

    uint32_t        tick;   //滴答计时器
    BuffBase::Ptr   buffPtr;
};


class BuffManager final
{
public:
    explicit BuffManager(PK& me);
    ~BuffManager() = default;


public:
    void sendBuffListToMe() const;
    //所有buffsize
    uint32_t buffSize() const;

    //施加一个buff
    bool showBuff(TplId buffId);
    //关闭
    void unshowBuff(TplId buffId);
    //是否有某个buff
    bool isShowBuff(TplId buffId) const;
    //获取属性接口
    uint32_t getBuffProps(PropertyType type, int32_t baseVal) const;
    uint32_t getHpMpBuffConstProp(PropertyType type, int32_t baseVal) const;
    uint32_t getHpMpBuffPercentProp(PropertyType type, int32_t baseVal) const;
    //主角经验加成
    uint16_t roleExpAddPercent() const;
    //英雄经验加成
    uint16_t heroExpAddPercent() const;

    void loadFromDB(std::vector<BuffData>& data);
    void timerExec(const water::componet::TimePoint& now);
    void processDeath();
    void processOffline();

    //返回选中的buff列表
    void retBuffList(std::shared_ptr<PK> asker);
    void retBuffTips(std::shared_ptr<PK> asker, TplId buffId);


private:
    //重新计算汇总属性
    void calcAttr();
    //回血回蓝, 掉血掉蓝
    void actionOnHpMp(BuffBase::Ptr buffPtr);

    void updateToDB(TplId buffId, ModifyType type);
    void updateToDB(std::vector<TplId>& buffIDs, ModifyType type);


public:
    //清除魔法盾
    void clearShield();
    //清除隐身
    void unHide();
    //增加buff时间
    void addTime(TplId buffId, uint16_t sec);

private:
    PK& m_owner;
    std::unordered_map<TplId, BuffElement> m_buffList;

    Attribute   m_constBuffProps;  //buff固定值属性
    Attribute   m_percentBuffProps;//百分比属性
};

}


#endif


