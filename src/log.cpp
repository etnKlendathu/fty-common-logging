#include "fty/fty-log.h"
#include <iostream>

namespace fty {

Logger::Logger(Ftylog::Level level, const std::string& file, int line, const std::string& func)
    : m_log({level, file, line, func, {}})
{
}

Logger::~Logger()
{
    callBackFunction()(m_log);
}

Logger::LoggerCallback& Logger::callBackFunction()
{
    static LoggerCallback func = [](const Log& log) {
        fty::ManageFtyLog::getInstanceFtylog().insertLog(
            log.level, log.file.c_str(), log.line, log.func.c_str(), log.content.c_str());
    };
    return func;
}

void Logger::setCallback(LoggerCallback&& callback)
{
    callBackFunction() = callback;
}

bool Logger::isSupports(Ftylog::Level level)
{
    return ManageFtyLog::getInstanceFtylog().isSupports(level);
}

}
