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

#include "include/fty_logger.h"
#include <fstream>
#include <log4cplus/appender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/mdc.h>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <thread>
#include <typeinfo>
#include <unistd.h>


class Ftylog::Impl
{
public:
    // Name of the agent/component
    std::string _agentName;
    // Path to the log configuration file if any
    std::string _configFile;
    // Layout pattern for logs
    std::string _layoutPattern;
    // log4cplus object to print logs
    log4cplus::Logger _logger;
    // Thread for watching modification of the log configuration file if any
    std::unique_ptr<log4cplus::ConfigureAndWatchThread> _watchConfigFile;

public:
    ~Impl();

    void init(const std::string& component, const std::string& configFile = {});

    // Set logging subsystem initialization messages' log level with a valid
    // string; log level set to OFF level otherwise
    // used only in loadAppenders() to control its spamminess
    void setLogLevelFromEnv();

    void setPatternFromEnv();

    // Set the console appender
    void setConsoleAppender();

    // Remove instances of log4cplus::ConsoleAppender from a given logger
    static void removeConsoleAppenders(log4cplus::Logger logger);

    // Set log level with level from syslog.h
    // for debug, info, warning, error, fatal or off
    bool setLogLevelFromEnvDefinite(const std::string& level);

    // Set default log level from envvar with a valid string;
    // log level set to trace level otherwise
    void setLogInitLevelFromEnv(const std::string& level);
    void setLogLevelFromEnv(const std::string& level);

    // Load appenders from the config file
    // or set the default console appender if no can't load from the config file
    void loadAppenders();

    // Call log4cplus system to print logs in logger appenders
    void insertLog(
        Level level, const char* file, int line, const char* func, const char* format, va_list args);
    // Switch the logging system to verbose
    void                setVerboseMode();
    void                setConfigFile(const std::string& file);
    bool                isLogLevel(Level level) const;
    log4cplus::LogLevel log4cplusLevel(Level level) const;

    void insertLog(Level level, const char* file, int line, const char* func, const std::string& str);
};

// ===========================================================================================================

Ftylog::Impl::~Impl()
{
    _logger.shutdown();
}

void Ftylog::Impl::init(const std::string& component, const std::string& configFile)
{
    _watchConfigFile.reset();
    _logger.shutdown();
    _agentName     = component;
    _configFile    = configFile;
    _layoutPattern = LOGPATTERN;

    // initialize log4cplus
    log4cplus::initialize();

    // Create logger
    _logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(component));

    // Get log level from bios and set to the logger
    // even if there is a log configuration file
    setLogLevelFromEnv();

    // Get pattern layout from env
    setPatternFromEnv();

    // load appenders
    loadAppenders();
}

// Set logging subsystem initialization messages' log level with a valid
// string; log level set to OFF level otherwise
// used only in loadAppenders() to control its spamminess
void Ftylog::Impl::setLogLevelFromEnv()
{
    // get BIOS_LOG_LEVEL value and set correction logging level
    const char* varEnv = getenv("BIOS_LOG_LEVEL");
    setLogLevelFromEnv(varEnv ? varEnv : "");
}

void Ftylog::Impl::setPatternFromEnv()
{
    // Get BIOS_LOG_PATTERN for a default patternlayout
    const char* varEnv = getenv("BIOS_LOG_PATTERN");
    if (varEnv && !std::string(varEnv).empty()) {
        _layoutPattern = varEnv;
    }
}

// Set the console appender
void Ftylog::Impl::setConsoleAppender()
{
    _logger.removeAllAppenders();
    // create appender
    // Note: the first bool argument controls logging to stderr(true) as output stream
    log4cplus::helpers::SharedObjectPtr<log4cplus::Appender> append(
        new log4cplus::ConsoleAppender(true, true));
    // Create and affect layout
    append->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(_layoutPattern)));
    append.get()->setName(LOG4CPLUS_TEXT("Console" + this->_agentName));

    // Add appender to logger
    _logger.addAppender(append);
}

// Remove instances of log4cplus::ConsoleAppender from a given logger
void Ftylog::Impl::removeConsoleAppenders(log4cplus::Logger logger)
{
    for (log4cplus::SharedAppenderPtr& appenderPtr : logger.getAllAppenders()) {
        log4cplus::Appender& app = *appenderPtr;

        if (typeid(app) == typeid(log4cplus::ConsoleAppender)) {
            // If any, remove it
            logger.removeAppender(appenderPtr);
            break;
        }
    }
}

// Set log level with level from syslog.h
// for debug, info, warning, error, fatal or off
bool Ftylog::Impl::setLogLevelFromEnvDefinite(const std::string& level)
{
    if (level.empty()) {
        // If empty string, set nothing and return false
        return false;
    } else if (level == "LOG_TRACE") {
        _logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
    } else if (level == "LOG_DEBUG") {
        _logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
    } else if (level == "LOG_INFO") {
        _logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
    } else if (level == "LOG_WARNING") {
        _logger.setLogLevel(log4cplus::WARN_LOG_LEVEL);
    } else if (level == "LOG_ERR") {
        _logger.setLogLevel(log4cplus::ERROR_LOG_LEVEL);
    } else if (level == "LOG_CRIT") {
        _logger.setLogLevel(log4cplus::FATAL_LOG_LEVEL);
    } else if (level == "LOG_OFF") {
        _logger.setLogLevel(log4cplus::OFF_LOG_LEVEL);
    } else {
        // If unknown string, set nothing and return false
        return false;
    }
    return true;
}


// Set default log level from envvar with a valid string;
// log level set to trace level otherwise
void Ftylog::Impl::setLogInitLevelFromEnv(const std::string& level)
{
    // If empty or unknown string, set logging subsystem initialization level to OFF
    if (!setLogLevelFromEnvDefinite(level)) {
        _logger.setLogLevel(log4cplus::OFF_LOG_LEVEL);
    }
}

void Ftylog::Impl::setLogLevelFromEnv(const std::string& level)
{
    // If empty or unknown string, set log level to TRACE by default
    if (!setLogLevelFromEnvDefinite(level)) {
        _logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
    }
}

// Load appenders from the config file
// or set the default console appender if no can't load from the config file
void Ftylog::Impl::loadAppenders()
{
    // Get BIOS_LOG_INIT_LEVEL value and set correction logging level
    // before we start processing the rest - perhaps the user does not
    // want to see reports about early logging initialization itself.
    // When we do have some configuration loaded or defaulted, it will
    // later override this setting.
    // Note that each call to loadAppenders() would process this envvar
    // again - this is because the de-initialization below can make
    // some noise, and/or config can be reloaded at run-time.
    const char*         varEnvInit = getenv("BIOS_LOG_INIT_LEVEL");
    log4cplus::LogLevel oldLevel   = log4cplus::NOT_SET_LOG_LEVEL;
    if (nullptr != varEnvInit) {
        // If the caller provided a BIOS_LOG_INIT_LEVEL setting,
        // honor it until we load a config file or have none -
        // and at that point revert to current logging level first.
        // This should allow for quiet tool startups when explicitly
        // desired.

        // Save the loglevel of the logger - e.g. a value set
        // by common BIOS_LOG_LEVEL earlier
        oldLevel = _logger.getLogLevel();
        setLogInitLevelFromEnv(varEnvInit);
    }

    // by default, load console appenders
    setConsoleAppender();

    // If true, load file
    bool loadFile = false;

    // if path to log config file
    if (!_configFile.empty()) {
        // file can be accessed with read rights
        if (FILE* file = fopen(_configFile.c_str(), "r")) {
            fclose(file);
            loadFile = true;
        } else {
            insertLog(Level::Error, __FILE__, __LINE__, __func__,
                "File " + _configFile +
                    " can't be accessed with read rights; this process will not monitor whether it "
                    "becomes available later");
        }
    } else {
        if (!varEnvInit) {
            insertLog(Level::Warn, __FILE__, __LINE__, __func__, "No log configuration file defined");
        }
    }

    // if no file or file not valid, set default ConsoleAppender
    if (loadFile) {
        if (!varEnvInit) {
            insertLog(Level::Info, __FILE__, __LINE__, __func__, "Load Config file " + _configFile);
        }

        // Remove previous appender
        _logger.removeAllAppenders();

        if (log4cplus::NOT_SET_LOG_LEVEL != oldLevel) {
            _logger.setLogLevel(oldLevel);
        }

        // Load the file
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(_configFile));
        // Start the thread watching the modification of the log config file
        _watchConfigFile.reset(new log4cplus::ConfigureAndWatchThread(LOG4CPLUS_TEXT(_configFile), 60000));
    } else {
        if (!varEnvInit) {
            insertLog(Level::Info, __FILE__, __LINE__, __func__,
                "No log configuration file was loaded, will log to stderr by default");
        }

        if (log4cplus::NOT_SET_LOG_LEVEL != oldLevel) {
            _logger.setLogLevel(oldLevel);
        }
    }
}

void Ftylog::Impl::insertLog(
    Level level, const char* file, int line, const char* func, const std::string& str)
{
    log4cplus::detail::macro_forced_log(_logger, log4cplusLevel(level), str.c_str(), file, line, func);
}


// Switch the logging system to verbose
void Ftylog::Impl::setVerboseMode()
{
    // Save the loglevel of the logger
    log4cplus::LogLevel oldLevel = _logger.getLogLevel();
    // set log level of the logger to TRACE
    _logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
    // Search if a console appender already exist in our logger instance or in
    // the root logger (we assume a flat hierarchy with the root logger and
    // specialized instances directly below the root logger)
    removeConsoleAppenders(_logger);
    removeConsoleAppenders(log4cplus::getDefaultHierarchy().getRoot());

    // Set all remaining appenders with the old log level as threshold if not defined
    for (log4cplus::SharedAppenderPtr& appenderPtr : _logger.getAllAppenders()) {
        log4cplus::Appender& app = *appenderPtr;
        if (app.getThreshold() == log4cplus::NOT_SET_LOG_LEVEL) {
            app.setThreshold(oldLevel);
        }
    }

    // create and add the appender
    log4cplus::helpers::SharedObjectPtr<log4cplus::Appender> append(
        new log4cplus::ConsoleAppender(false, true));
    // Create and affect layout
    append->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(_layoutPattern)));
    append.get()->setName(LOG4CPLUS_TEXT("Verbose-" + this->_agentName));
    // Add verbose appender to logger
    _logger.addAppender(append);
}

void Ftylog::Impl::setConfigFile(const std::string& file)
{
    _configFile = file;
    loadAppenders();
}

bool Ftylog::Impl::isLogLevel(Level level) const
{
    return _logger.getLogLevel() <= log4cplusLevel(level);
}

log4cplus::LogLevel Ftylog::Impl::log4cplusLevel(Level level) const
{
    switch (level) {
        case Level::Debug:
            return log4cplus::DEBUG_LOG_LEVEL;
        case Level::Error:
            return log4cplus::ERROR_LOG_LEVEL;
        case Level::Fatal:
            return log4cplus::FATAL_LOG_LEVEL;
        case Level::Info:
            return log4cplus::INFO_LOG_LEVEL;
        case Level::Off:
            return log4cplus::OFF_LOG_LEVEL;
        case Level::Trace:
            return log4cplus::TRACE_LOG_LEVEL;
        case Level::Warn:
            return log4cplus::WARN_LOG_LEVEL;
    }
}

// ===========================================================================================================

Ftylog::Ftylog(const std::string& component, const std::string& configFile)
    : m_impl(new Impl)
{
    m_impl->init(component, configFile);
}

Ftylog::Ftylog()
    : m_impl(new Impl)
{
    std::ostringstream threadId;
    threadId << std::this_thread::get_id();
    std::string name = "log-default-" + threadId.str();
    m_impl->init(name);
}

Ftylog::~Ftylog()
{
}

const std::string& Ftylog::getAgentName() const
{
    return m_impl->_agentName;
}

void Ftylog::setConfigFile(const std::string& file)
{
    m_impl->setConfigFile(file);
}

void Ftylog::change(const std::string& name, const std::string& configFile)
{
    m_impl->init(name, configFile);
}

// Switch the logging system to verbose
void Ftylog::setVeboseMode()
{
    m_impl->setVerboseMode();
}

void Ftylog::setContext(const std::map<std::string, std::string>& contextParam)
{
    log4cplus::getMDC().clear();
    for (auto const& entry : contextParam) {
        log4cplus::getMDC().put(entry.first, entry.second);
    }
}

void Ftylog::clearContext()
{
    log4cplus::getMDC().clear();
}

void Ftylog::setLogLevelTrace()
{
    m_impl->_logger.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
}

void Ftylog::setLogLevelDebug()
{
    m_impl->_logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
}

void Ftylog::setLogLevelInfo()
{
    m_impl->_logger.setLogLevel(log4cplus::INFO_LOG_LEVEL);
}

void Ftylog::setLogLevelWarning()
{
    m_impl->_logger.setLogLevel(log4cplus::WARN_LOG_LEVEL);
}

void Ftylog::setLogLevelError()
{
    m_impl->_logger.setLogLevel(log4cplus::ERROR_LOG_LEVEL);
}

void Ftylog::setLogLevelFatal()
{
    m_impl->_logger.setLogLevel(log4cplus::FATAL_LOG_LEVEL);
}

void Ftylog::setLogLevelOff()
{
    m_impl->_logger.setLogLevel(log4cplus::OFF_LOG_LEVEL);
}

bool Ftylog::isLogTrace() const
{
    return m_impl->isLogLevel(Level::Trace);
}

bool Ftylog::isLogDebug() const
{
    return m_impl->isLogLevel(Level::Debug);
}

bool Ftylog::isLogInfo() const
{
    return m_impl->isLogLevel(Level::Info);
}

bool Ftylog::isLogWarning() const
{
    return m_impl->isLogLevel(Level::Warn);
}

bool Ftylog::isLogError() const
{
    return m_impl->isLogLevel(Level::Error);
}

bool Ftylog::isLogFatal() const
{
    return m_impl->isLogLevel(Level::Fatal);
}

bool Ftylog::isSupports(Level level) const
{
    return m_impl->isLogLevel(level);
}

bool Ftylog::isLogOff() const
{
    return m_impl->_logger.getLogLevel() == log4cplus::OFF_LOG_LEVEL;
}

void Ftylog::insertLog(Level level, const char* file, int line, const char* func, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    va_list args2;
    va_copy(args2, args);
    std::string buff;
    buff.resize(size_t(std::vsnprintf(nullptr, 0, format, args)+1));
    va_end(args);
    std::vsnprintf(buff.data(), buff.size(), format, args2);
    va_end(args2);
    m_impl->insertLog(level, file, line, func, buff);
}

// ===========================================================================================================
// ManageFtyLog section
// ===========================================================================================================

Ftylog ManageFtyLog::_ftylogdefault = Ftylog("ftylog", FTY_COMMON_LOGGING_DEFAULT_CFG);

Ftylog& ManageFtyLog::getInstanceFtylog()
{
    return _ftylogdefault;
}

void ManageFtyLog::setInstanceFtylog(const std::string& componentName, const std::string& logConfigFile)
{
    _ftylogdefault.change(componentName, logConfigFile);
}

// ===========================================================================================================
