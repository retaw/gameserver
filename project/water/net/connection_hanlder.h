/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-09-12 21:11 +0800
 *
 * Description: 
 */

#ifndef WATER_NET_CONNECTION_HANLDER
#define WATER_NET_CONNECTION_HANLDER


#include "connection.h"

namespace water{
namespace net{

class MessageHanlder
{
public:
    MessageHanlder();
    ~MessageHanlder();

    void insert();
    void erase();

    void sender();
    void recv();
private:
    std::set<TcpConnection::Ptr> connections;
    

};

}
}

#endif
