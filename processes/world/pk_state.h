/*
 * Author: LiZhaojia
 *
 * Created: 2015-04-09 19:50 +0800
 *
 * Modified: 2015-04-09 19:50 +0800
 *
 * Description: pk对象状态类
 */

#ifndef PROCESS_WORLD_PK_STATE_H
#define PROCESS_WORLD_PK_STATE_H

#include "pkdef.h"

#include "water/common/roledef.h"

#include <vector>


namespace world{


class PK;
class PKState final
{
public:
    explicit PKState(PK& me);
    ~PKState() = default;


public:
    uint32_t pkstatus() const;
    void loadStatus(uint32_t status);
    void loadStatus(visual_status status);
    bool issetStatus(visual_status status) const;
    bool issetStatus(uint32_t status) const;
    bool setStatus(visual_status status);
    bool setStatus(uint32_t status);
    bool clearStatus(visual_status status);
    bool clearStatus(uint32_t status);

private:
    void showStatusTo9(uint32_t status) const;
    void unshowStatusTo9(uint32_t status) const;

private:
    PK& m_owner; //状态拥有者
    uint32_t m_status; //按位拼接
};

}


#endif

