/*  =========================================================================
    fty_log - Log management

    Copyright (C) 2014 - 2018 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
 */

#pragma once
#include <map>
#include <memory>
#include <string>

// Trick to avoid conflict with CXXTOOLS logger, currently the BIOS code
// prefers OUR logger macros
#if defined(LOG_CXXTOOLS_H) || defined(CXXTOOLS_LOG_CXXTOOLS_H)
#undef log_trace
#undef log_error
#undef log_debug
#undef log_info
#undef log_fatal
#undef log_warn
#undef log_define
#define log_define(category)
#else
#define LOG_CXXTOOLS_H
#define CXXTOOLS_LOG_CXXTOOLS_H
#endif

#ifdef log_macro
// Old logger is used
#undef log_macro
#undef log_trace_log
#undef log_debug_log
#undef log_info_log
#undef log_warning_log
#undef log_error_log
#undef log_fatal_log
#undef log_trace
#undef log_debug
#undef log_info
#undef log_warning
#undef log_error
#undef log_fatal
#undef FTY_COMMON_LOGGING_DEFAULT_CFG
#endif

#define log_macro(level, ftylogger, ...)                                                                     \
    do {                                                                                                     \
        ftylogger.insertLog((level), __FILE__, __LINE__, __func__, __VA_ARGS__);                             \
    } while (0)

/// Logging with explicit logger
/// Prints message with TRACE level.
#define log_trace_log(ftylogger, ...) log_macro(fty::Ftylog::Level::Trace, ftylogger, __VA_ARGS__)

/// Prints message with DEBUG level.
#define log_debug_log(ftylogger, ...) log_macro(fty::Ftylog::Level::Debug, ftylogger, __VA_ARGS__)

/// Prints message with INFO level.
#define log_info_log(ftylogger, ...) log_macro(fty::Ftylog::Level::Info, ftylogger, __VA_ARGS__)

/// Prints message with WARNING level.
#define log_warning_log(ftylogger, ...) log_macro(fty::Ftylog::Level::Warn, ftylogger, __VA_ARGS__)

/// Prints message with ERROR level.
#define log_error_log(ftylogger, ...) log_macro(fty::Ftylog::Level::Error, ftylogger, __VA_ARGS__)

/// Prints message with FATAL level.
#define log_fatal_log(ftylogger, ...) log_macro(fty::Ftylog::Level::Fatal, ftylogger, __VA_ARGS__)

/// Logging with default logger
/// Prints message with TRACE level.
#define log_trace(...)                                                                                       \
    log_macro(fty::Ftylog::Level::Trace, fty::ManageFtyLog::getInstanceFtylog(), __VA_ARGS__)

/// Prints message with DEBUG level.
#define log_debug(...)                                                                                       \
    log_macro(fty::Ftylog::Level::Debug, fty::ManageFtyLog::getInstanceFtylog(), __VA_ARGS__)

/// Prints message with INFO level.
#define log_info(...) log_macro(fty::Ftylog::Level::Info, fty::ManageFtyLog::getInstanceFtylog(), __VA_ARGS__)

/// Prints message with WARNING level.
#define log_warning(...)                                                                                     \
    log_macro(fty::Ftylog::Level::Warn, fty::ManageFtyLog::getInstanceFtylog(), __VA_ARGS__)

/// Prints message with ERROR level.
#define log_error(...)                                                                                       \
    log_macro(fty::Ftylog::Level::Error, fty::ManageFtyLog::getInstanceFtylog(), __VA_ARGS__)

/// Prints message with FATAL level.
#define log_fatal(...)                                                                                       \
    log_macro(fty::Ftylog::Level::Fatal, fty::ManageFtyLog::getInstanceFtylog(), __VA_ARGS__)

#define LOG_START             log_debug("start")
#define LOG_END               log_debug("end::normal")
#define LOG_END_ABNORMAL(exp) log_error("end::abnormal with %s", (exp).what())

// Default layout pattern
#define LOGPATTERN "%c [%t] -%-5p- %M (%l) %m%n"

namespace fty {

class Ftylog
{
public:
    enum class Level
    {
        Off,
        Fatal,
        Error,
        Warn,
        Info,
        Debug,
        Trace,
    };

    // Constructor/destructor
    Ftylog(const std::string& component, const std::string& logConfigFile = {});
    Ftylog();
    ~Ftylog();

    /// getter
    const std::string& getAgentName() const;

    /// setter
    /// Set the path to the log config file
    /// And try to load it
    void setConfigFile(const std::string& file);

    /// Change properties of the Ftylog object
    void change(const std::string& name, const std::string& configFile);

    /// Set the logger to a specific log level
    void setLogLevelTrace();
    void setLogLevelDebug();
    void setLogLevelInfo();
    void setLogLevelWarning();
    void setLogLevelError();
    void setLogLevelFatal();
    void setLogLevelOff();

    /// Check the log level
    bool isLogTrace() const;
    bool isLogDebug() const;
    bool isLogInfo() const;
    bool isLogWarning() const;
    bool isLogError() const;
    bool isLogFatal() const;
    bool isLogOff() const;

    bool isSupports(Level level) const;

    /// \brief insertLog
    /// An internal logging function, use specific log_error, log_debug  macros!
    /// \param level - level for message, see \ref log4cplus::logLevel
    /// \param file - name of file issued print, usually content of __FILE__ macro
    /// \param line - number of line, usually content of __LINE__ macro
    /// \param func - name of function issued log, usually content of __func__ macro
    /// \param format - printf-like format string
    void insertLog(Level level, const char* file, int line, const char* func, const char* format, ...);

    /// Load a specific appender if verbose mode is set to true :
    /// -Save the logger logging level and set it to TRACE logging level
    /// -Remove an already existing ConsoleAppender
    /// -For the other appender, set the threshold parameter to the old logger log level
    ///    if no loglevel defined for this appender
    /// -Add a new console appender
    void setVeboseMode();

    /// Set a context for a mapped diagnostic context (MDC)
    /// @param contextParam The context params mapped.
    static void setContext(const std::map<std::string, std::string>& contextParam);

    /// Clear the mapped diagnostic context.
    static void clearContext();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

/// Singleton for logger managment
class ManageFtyLog
{
private:
    /// Avoid use of the following procedures/functions
    ManageFtyLog(const ManageFtyLog&) = delete;
    ManageFtyLog& operator=(const ManageFtyLog&) = delete;
    static Ftylog _ftylogdefault;

public:
    static constexpr const char* FTY_COMMON_LOGGING_DEFAULT_CFG = "/etc/fty/ftylog.cfg";

public:
    /// Returns the Ftylog obect from the instance
    static Ftylog& getInstanceFtylog();
    /// Creates or replaces the Ftylog object in the instance using a new Ftylog object
    static void setInstanceFtylog(const std::string& componentName, const std::string& logConfigFile = {});
};

} // namespace fty
