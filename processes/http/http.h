/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server 的进程定义
 */

#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#include "water/process/process.h"
#include "water/process/http_connection_manager.h"

namespace http{

using namespace water;
using namespace process;

class Http: public Process
{
public:
    Http(int32_t num, const std::string& configDir, const std::string& logDir);
	virtual void extendInit();
public:
	void regHttpCallBack();
	void dealPacket(HttpConnectionManager::ConnectionHolder::Ptr connHolder, net::Packet::CPtr packet);

};

}

#endif
