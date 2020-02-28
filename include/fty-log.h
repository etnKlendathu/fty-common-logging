#pragma once
#include "include/fty_logger.h"
#include <functional>
#include <memory>
#include <sstream>
#include <string>

// ===========================================================================================================

template <class...>
constexpr std::false_type always_false{};

template <typename... T>
struct is_cont_helper
{
};

template <typename T, typename = void>
struct is_list : std::false_type
{
};

template <typename T, typename = void>
struct is_map : std::false_type
{
};

// clang-format off
template <typename T>
struct is_list<T, std::conditional_t<false,
    is_cont_helper<
        typename T::value_type,
        typename T::size_type,
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())
    >,
    void>> : public std::true_type
{
};

template <typename T>
struct is_map<T, std::conditional_t<false,
    is_cont_helper<
        typename T::key_type,
        typename T::value_type,
        typename T::size_type,
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())
    >,
    void>> : public std::true_type
{
};

class Logger;


template <typename T, typename = decltype(std::declval<T>().operator<<(std::declval<Logger&>()))>
struct has_stream_op : public std::true_type
{
};

// clang-format on


// ===========================================================================================================

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
    Logger& operator<<(const T& val)
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
            addText(val ? "true" : "false");
        } else if constexpr (std::is_integral_v<T>) {
            addText(std::to_string(val));
        } else if constexpr (std::is_floating_point_v<T>) {
            // In case of floats std::to_string gives not pretty results. So, just use stringstream here.
            std::stringstream ss;
            ss << val;
            addText(ss.str());
        } else if constexpr (std::is_pointer_v<T>) {
            std::stringstream ss;
            ss << std::hex << val;
            addText(ss.str());
        } else if constexpr (is_map<T>::value) {
            std::stringstream ss;
            ss << "{";
            bool first = true;
            for (const auto& it : val) {
                ss << (!first ? ", " : "") << "{" << it.first << " : " << it.second << "}";
                first = false;
            }
            ss << "}";
            addText(ss.str());
        } else if constexpr (is_list<T>::value) {
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
        } else if constexpr (has_stream_op<T>::value)  {
            val.operator<<(*this);
        } else {
            static_assert(always_false<T>, "Unsupported type");
        }
        return *this;
    }

    static LoggerCallback& callBackFunction();
    static void            setCallback(LoggerCallback&& callback);
    static bool            isSupports(Ftylog::Level level);

public:
    struct nowhitespace
    {};
public:
    struct Void
    {
        // This has to be an operator with a precedence lower than << but higher than ?:
        void operator&(Logger&)
        {
        }
    };

private:
    Log m_log;
    bool m_inswhite = true;
};

// ===========================================================================================================

#define _log(level, condition)                                                                               \
    !Logger::isSupports(level) || !(condition)                                                               \
        ? void(0)                                                                                            \
        : Logger::Void() & Logger(level, __FILE__, __LINE__, __func__)

#define logIf(level, condition) _log(level, condition)

#define logDbg()              _log(Ftylog::Level::Debug, true)
#define logInfo()             _log(Ftylog::Level::Info, true)
#define logFatal()            _log(Ftylog::Level::Fatal, true)
#define logError()            _log(Ftylog::Level::Error, true)
#define logWarn()             _log(Ftylog::Level::Warn, true)
#define logTrace()            _log(Ftylog::Level::Trace, true)
#define logDbgIf(condition)   _log(Ftylog::Level::Debug, condition)
#define logInfoIf(condition)  _log(Ftylog::Level::Info, condition)
#define logFatalIf(condition) _log(Ftylog::Level::Fatal, condition)
#define logErrorIf(condition) _log(Ftylog::Level::Error, condition)
#define logWarnIf(condition)  _log(Ftylog::Level::Warn, condition)
#define logTraceIf(condition) _log(Ftylog::Level::Trace, condition)

// ===========================================================================================================
