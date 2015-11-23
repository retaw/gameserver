#include "connection_checker.h"

namespace water{
namespace process{

bool ConnectionChecker::ConnectionChecker::exec()
{
    checkConn();
    return true;
}


}}
