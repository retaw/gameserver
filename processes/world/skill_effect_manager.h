#ifndef PROCESS_WORLD_SKILL_EFFECT_MGR_H
#define PROCESS_WORLD_SKILL_EFFECT_MGR_H

#include "skill_effect_element.h"

#include <memory>
#include <unordered_map>



namespace world{

#define MAX_SKILL_LOGIC_NUM 28


class SkillEffectMgr final
{
public:
    explicit SkillEffectMgr(PK& me);
    ~SkillEffectMgr() = default;


private:
    void initFunc();


public:
    //执行一个技能效果
    void addSkillEffect(SkillEffectEle& see);

    //
    logic_retcode run(SkillEffectEle& see);

    //定时器
    void timerExec();

    void clear();

private:
    //技能效果函数列表
    struct{
        logic_retcode (* func)(PK& owner, SkillEffectEle&);
    }m_logicList[MAX_SKILL_LOGIC_NUM];


    PK& m_owner;

    //持续技能效果id列表
    std::unordered_map<uint32_t, SkillEffectEle> m_timerElements;
    bool clearFlag;
    bool inited;
    uint32_t tick;
};



}


#endif

