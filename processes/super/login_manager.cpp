#include "login_manager.h"

#include "water/dbadaptcher/dbconnection_pool.h"

namespace super{


LoginManager LoginManager::me()
{
    static LoginManager me;
    return me;
}


void LoginManager::clientConnReady(LoginId loginId)
{
}

void LoginManager:clientmsg_UserSignUp(const ProtoMsgPtr& proto, const ProtoMsgPtr& proto, LoginId loginId)
{
    Platform
}

void LoginManager:clientmsg_UserLogIn(const ProtoMsgPtr& proto, const ProtoMsgPtr& proto, LoginId loginId)
{
}



}
