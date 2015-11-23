#include "level_props.h"
#include "config_table_manager.h"


namespace world{


LevelProps::LevelProps(const Job& job, const SceneItemType& sceneItem)
  :m_job(job)
  ,m_sceneItem(sceneItem)
  ,m_level(1)
{
}


void LevelProps::setLevel(uint32_t level)
{
    m_level = level;
    calcAttr();
}


uint32_t LevelProps::level() const
{
    return m_level;
}


void LevelProps::calcAttr()
{
    uint32_t job = static_cast<uint8_t>(m_job);
    uint32_t sceneItem = static_cast<uint8_t>(m_sceneItem);
    uint32_t hash_key = rlp_hash(job, sceneItem, m_level);
    LevelPropsBase::Ptr rlp_ptr = levelPropCT.get(hash_key);
    if(nullptr == rlp_ptr)
    {
        LOG_ERROR("角色属性值找不到, hash_key={}, job={}, type={}, level={}",
				  hash_key, m_job, m_sceneItem, m_level);
        return;
    }

    setAttribute(rlp_ptr->level_props);
}

}
