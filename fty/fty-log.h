#pragma once
#include "fty/convert.h"
#include "fty/fty_logger.h"
#include "fty/traits.h"
#include <functional>
#include <memory>
#include <sstream>
#include <string>

// ===========================================================================================================

#define _log(level, condition)                                                                               \
    !fty::Logger::isSupports(level) || !(condition)                                                          \
        ? void(0)                                                                                            \
        : fty::Logger::Void() & fty::Logger(level, __FILE__, __LINE__, __func__)

#define logIf(level, condition) _log(level, condition)

#define logDbg()              _log(fty::Ftylog::Level::Debug, true)
#define logInfo()             _log(fty::Ftylog::Level::Info, true)
#define logFatal()            _log(fty::Ftylog::Level::Fatal, true)
#define logError()            _log(fty::Ftylog::Level::Error, true)
#define logWarn()             _log(fty::Ftylog::Level::Warn, true)
#define logTrace()            _log(fty::Ftylog::Level::Trace, true)
#define logDbgIf(condition)   _log(fty::Ftylog::Level::Debug, condition)
#define logInfoIf(condition)  _log(fty::Ftylog::Level::Info, condition)
#define logFatalIf(condition) _log(fty::Ftylog::Level::Fatal, condition)
#define logErrorIf(condition) _log(fty::Ftylog::Level::Error, condition)
#define logWarnIf(condition)  _log(fty::Ftylog::Level::Warn, condition)
#define logTraceIf(condition) _log(fty::Ftylog::Level::Trace, condition)


// ===========================================================================================================
namespace fty {

class Logger;

template<typename T, typename = void>
struct has_stream_op : public std::false_type
{
};

template <typename T>
struct has_stream_op<T, std::void_t<decltype(std::declval<T>().operator<<(std::declval<Logger&>()))>> : public std::true_type
{
};

class Logger
{
public:
    struct Log
    {
        Ftylog::Level level;
        std::string   file;
        int           line;
        std::string   func;
        std::string   content;
    };
    using LoggerCallback = std::function<void(const Log&)>;

public:
    Logger(Ftylog::Level level, const std::string& file, int line, const std::string& func);
    ~Logger();

    Logger(const Logger&) = delete;
    Logger(Logger&&)      = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    template <typename T>
    Logger& operator<<(const T& val);

    static LoggerCallback& callBackFunction();
    static void            setCallback(LoggerCallback&& callback);
    static bool            isSupports(Ftylog::Level level);

public:
    struct nowhitespace
    {
    };

public:
    struct Void
    {
        // This has to be an operator with a precedence lower than << but higher than ?:
        void operator&(Logger&)
        {
        }
    };

private:
    Log  m_log;
    bool m_inswhite = true;
};

// ===========================================================================================================

template <typename T>
Logger& Logger::operator<<(const T& val)
{
    auto addText = [&](const std::string& str) {
        if (m_inswhite && !m_log.content.empty()) {
            m_log.content += " ";
        }
        m_log.content += str;
    };

    if constexpr (std::is_constructible_v<std::string, T>) {
        addText(val);
    } else if constexpr (std::is_same_v<bool, T>) {
        addText(fty::convert<std::string>(val));
    } else if constexpr (std::is_integral_v<T>) {
        addText(fty::convert<std::string>(val));
    } else if constexpr (std::is_floating_point_v<T>) {
        addText(fty::convert<std::string>(val));
    } else if constexpr (std::is_pointer_v<T>) {
        std::stringstream ss;
        ss << std::hex << val;
        addText(ss.str());
    } else if constexpr (fty::is_map<T>::value) {
        std::stringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& it : val) {
            ss << (!first ? ", " : "") << "{" << it.first << " : " << it.second << "}";
            first = false;
        }
        ss << "}";
        addText(ss.str());
    } else if constexpr (fty::is_list<T>::value) {
        std::stringstream ss;
        ss << "[";
        bool first = true;
        for (const auto& it : val) {
            ss << (!first ? ", " : "") << it;
            first = false;
        }
        ss << "]";
        addText(ss.str());
    } else if constexpr (std::is_same_v<T, nowhitespace>) {
        m_inswhite = false;
    } else if constexpr (has_stream_op<T>::value) {
        val.operator<<(*this);
    } else {
        addText(fty::convert<std::string>(val));
    }
    return *this;
}

} // namespace fty
