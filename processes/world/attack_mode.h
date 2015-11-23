#ifndef PROCESS_WORLD_ATTACK_MODE_H
#define PROCESS_WORLD_ATTACK_MODE_H

#include "pkdef.h"
#include <memory>

namespace world{

class PK;
class Role;
class Npc;
class AttackMode final
{
public:
    explicit AttackMode(Role& owner, uint8_t mode, uint16_t evil);
    ~AttackMode() = default;

public:
    void setMode(attack_mode mode, bool force=false);
    //当前用户所处攻击模式
    attack_mode mode() const;
    //需要保存到db的攻击模式
    attack_mode saveMode() const;
    //攻击模式pk中判断的接口
    bool isEnemy(std::shared_ptr<PK> atk) const;

    void login();
    void initNameColor();
    uint16_t evilVal() const;
    void addEvil();
    void subEvil(uint16_t evil = 1);
    void judgeAndSetNameColor(bool clearGreyname=false);


private:
    bool isEnemy(std::shared_ptr<Role> atkRole) const;
    bool isEnemy(std::shared_ptr<Npc> atkNpc) const;
    void notifyAttackMode(attack_mode) const;
    void refreshPKValue() const;

private:
    attack_mode m_mode; 
    attack_mode m_mapAttackMode; //地图强制攻击模式
    Role& m_owner;
    uint16_t m_evilVal; //罪恶值
};

}

#endif

