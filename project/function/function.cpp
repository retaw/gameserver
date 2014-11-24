#include "water/process.h"
#include "water/componet/string_kit.h"

using namespace water;

int main(int argc, char* argv[])
{
    std::string configFile = "../config/process.xml";
    int32_t id = 0;

    if(argc > 1)
        configFile = argv[1];
    if(argc > 2)
        id = componet::fromString<int32_t>(argv[2]);

    TcpConnectionManager::Ptr tcm = TcpConnectionManager::create();

    Process process(ProcessType::function, id, configFile, tcm);
    process.start();
}






