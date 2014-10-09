#include "string_kit.h"

namespace water{
namespace componet{


std::string toString(const datetime::TimePoint tp)
{
    return datetime::timePointToString(tp);
}


}}
