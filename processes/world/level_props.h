
#ifndef WORLD_ROLE_LEVEL_HPP
#define WORLD_ROLE_LEVEL_HPP


#include "pkdef.h"
#include "attribute.h"

#include "water/common/roledef.h"


namespace world{

class LevelProps : public Attribute
{
public:
    explicit LevelProps(const Job& job, const SceneItemType& sceneItem);
    ~LevelProps() = default;

public:
    void setLevel(uint32_t level);
    uint32_t level() const;

private:
    void calcAttr();

private:
    const Job m_job;
    const SceneItemType m_sceneItem;
    uint32_t m_level;
};


}


#endif
