#include "string_kit.h"

namespace water{
namespace componet{


std::string toString(const datetime::TimePoint& tp)
{
    return datetime::timePointToString(tp);
}

template<>
datetime::TimePoint fromString<datetime::TimePoint>(const std::string& str)
{
    return datetime::stringToTimePoint(str);
}

}}
