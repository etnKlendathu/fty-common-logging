#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "fty/fty_logger.h"

TEST_CASE("Logger testing")
{
    fty::ManageFtyLog::getInstanceFtylog().setLogLevelTrace();
    log_trace("This is a simple %s log with default logger", "trace");
    log_debug("This is a simple %s log with default logger", "debug");
    log_info("This is a simple %s log with default logger", "info");
    log_warning("This is a simple %s log with default logger", "warning");
    log_error("This is a simple %s log with default logger", "error");
    log_fatal("This is a simple %s log with default logger", "fatal");


    fty::ManageFtyLog::setInstanceFtylog("fty-log-agent");
    fty::Ftylog& test = fty::ManageFtyLog::getInstanceFtylog();

    SECTION("Check trace level")
    {
        test.setLogLevelTrace();
        log_trace_log(test, "This is a simple trace log");
        log_trace_log(test, "This is a %s log test number %d", "trace", 1);
        REQUIRE(test.isLogTrace());
        REQUIRE(test.isLogDebug());
        REQUIRE(test.isLogInfo());
        REQUIRE(test.isLogWarning());
        REQUIRE(test.isLogError());
        REQUIRE(test.isLogFatal());
    }

    SECTION("Check debug level")
    {
        test.setLogLevelDebug();
        log_debug_log(test, "This is a simple debug log");
        log_debug_log(test, "This is a %s log test number %d", "debug", 1);
        REQUIRE(!test.isLogTrace());
        REQUIRE(test.isLogDebug());
        REQUIRE(test.isLogInfo());
        REQUIRE(test.isLogWarning());
        REQUIRE(test.isLogError());
        REQUIRE(test.isLogFatal());
    }

    SECTION("Check info level")
    {
        test.setLogLevelInfo();
        log_info_log(test, "This is a simple info log");
        log_info_log(test, "This is a %s log test number %d", "info", 1);
        REQUIRE(!test.isLogTrace());
        REQUIRE(!test.isLogDebug());
        REQUIRE(test.isLogInfo());
        REQUIRE(test.isLogWarning());
        REQUIRE(test.isLogError());
        REQUIRE(test.isLogFatal());
    }

    SECTION("Check warning level")
    {
        test.setLogLevelWarning();
        log_warning_log(test, "This is a simple warning log");
        log_warning_log(test, "This is a %s log test number %d", "warning", 1);
        REQUIRE(!test.isLogTrace());
        REQUIRE(!test.isLogDebug());
        REQUIRE(!test.isLogInfo());
        REQUIRE(test.isLogWarning());
        REQUIRE(test.isLogError());
        REQUIRE(test.isLogFatal());
    }

    SECTION("Check errror level")
    {
        test.setLogLevelError();
        log_error_log(test, "This is a simple error log");
        log_error_log(test, "This is a %s log test number %d", "error", 1);
        REQUIRE(!test.isLogTrace());
        REQUIRE(!test.isLogDebug());
        REQUIRE(!test.isLogInfo());
        REQUIRE(!test.isLogWarning());
        REQUIRE(test.isLogError());
        REQUIRE(test.isLogFatal());
    }

    SECTION("Check fatal level")
    {
        test.setLogLevelFatal();
        log_fatal_log(test, "This is a simple fatal log");
        log_fatal_log(test, "This is a %s log test number %d", "fatal", 1);
        REQUIRE(!test.isLogTrace());
        REQUIRE(!test.isLogDebug());
        REQUIRE(!test.isLogInfo());
        REQUIRE(!test.isLogWarning());
        REQUIRE(!test.isLogError());
        REQUIRE(test.isLogFatal());
    }

    SECTION("Configuration test")
    {
        REQUIRE(access("conf/test-config.conf", F_OK) != -1);

        test.setLogLevelTrace();
        test.setConfigFile("conf/not-a-valid-file.conf");
        test.setConfigFile("conf/test-config.conf");

        log_info_log(test, "This is a simple info log");
        log_info_log(test, "This is a %s log test number %d", "info", 1);

        // file is created
        REQUIRE(access("/tmp/logging.txt", F_OK) != -1);

        // this size is not 0
        std::ifstream file("/tmp/logging.txt", std::ifstream::in | std::ifstream::binary);
        file.seekg(0, std::ios::end);
        int64_t fileSize = file.tellg();

        REQUIRE(0 != fileSize);
        test.setVeboseMode();
        log_trace_log(test, "This is a verbose trace log");

        remove("/tmp/logging.txt");
    }
}

