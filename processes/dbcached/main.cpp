/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-12-03 10:46 +0800
 *
 * Description:  game server启动入口
 */

#include "dbcached.h"
#include "water/process/shell_arg_parser.h"
#include "protocol/protobuf/proto_manager.h"

#include <csignal>

//#include "object_manager.h"
//#include "dbadaptcher/dbconnection_pool.h"
int main(int argc, char* argv[])
{
    using namespace dbcached;

    water::process::ShellArgParser arg(argc, argv);

    protocol::protobuf::ProtoManager::me().loadConfig(arg.configDir());

    dbcached::DbCached::init(arg.num(), arg.configDir(), arg.logDir());
    dbcached::DbCached::me().start();
    /*
    using namespace water;
    using namespace dbadaptcher;
    MysqlConnectionPool::me().init("/home/centos/server/config/process.xml");
    PrivateRaw::RetModifyObjData::ModifyObj modifyObj{ModifyType::erase,1000,PackageType::hero,0,0,1};
    RoleId roleId = 4294967297;
    dbcached::ObjectManager::me().modify(modifyObj,roleId);
    */
/*
    protocol::protobuf::Message::getMe().loadConfig("/home/water/server/config/");

    PrivateProto::Test_RetNum proto1;
    proto1.set_num(12);
    proto1.set_msg("hello");

    auto bin = proto1.SerializeAsString();

    auto proto = protocol::protobuf::Message::getMe().createMessage(2004);
    proto->ParseFromString(bin);

    auto proto2 = std::static_pointer_cast<PrivateProto::Test_RetNum>(proto);

    std::cout << proto2->num() << " " << proto2->msg() << std::endl;
*/
}
