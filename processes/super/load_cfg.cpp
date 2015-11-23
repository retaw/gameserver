/*
 * Author: LiZhaojia
 *
 * Created: 2015-07-14 17:45 +0800
 *
 * Modified: 2015-07-14 17:45 +0800
 *
 * Description: 
 */



#include "super.h"

#include "zone_manager.h"

namespace super{


void Super::loadConfig()
{
    ZoneManager::me().loadConfig(m_cfgDir);
}


}
