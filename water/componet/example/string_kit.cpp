#include "test.h"

#include "../exception.h"
#include "../../net/endpoint.h"
//#include "process_id.h"
#include "../logger.h"                                                                                     
#include "../xmlparse.h"
#include "../format.h"                                                                                  
#include "../string_kit.h"

using namespace water;
using namespace componet;
using namespace test;

int main()
{
    const std::vector<std::string>& vec = splitString("", " ");
    std::cout << vec.size() << ":" << vec << std::endl;
}
