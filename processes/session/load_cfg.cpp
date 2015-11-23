#include "session.h"

#include "scene_dispenser.h"
#include "roles_and_scenes.h"

namespace session{

void Session::loadConfig()
{
    SceneDispenser::me().loadConfig(m_cfgDir);
    RolesAndScenes::me().loadConfig(m_cfgDir);
}

}
