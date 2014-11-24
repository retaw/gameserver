/*
 * Author: LiZhaojia 
 *
 * Last modified: 2014-10-09 17:42 +0800
 *
 * Description: 
 */

#ifndef WATER_COMPONET_LOG_HPP
#define WATER_COMPONET_LOG_HPP

#include "log/logger.h"
#include "log/file_writer.h"

#include <atomic>
#include <mutex>

namespace water{
namespace componet{

class GlobalLogger : public Logger
{
public:
    ~GlobalLogger();
    static GlobalLogger* getInstance();
private:
    GlobalLogger();
};


}}

#define LOG_DEBUG(args...) water::componet::GlobalLogger::getInstance()->debug(args);
#define LOG_TRACE(args...) water::componet::GlobalLogger::getInstance()->trace(args);
#define LOG_ERROR(args...) water::componet::GlobalLogger::getInstance()->error(args);

#endif
