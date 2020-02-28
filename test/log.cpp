#include <catch2/catch.hpp>
#include "include/fty-log.h"
#include <iostream>

struct MyStruct
{
    std::string val;
    int num;

    Logger& operator<<(Logger& log) const
    {
        log << Logger::nowhitespace() << "MyStruct{val = " << val << "; num = " << num << "}";
        return log;
    }
};

TEST_CASE("Log test")
{
    Logger::Log currentLog;
    Logger::setCallback([&](const Logger::Log& log){
        currentLog = log;
    });

    SECTION("Test string")
    {
        logDbg() << "Dead Parrot";

        REQUIRE(Ftylog::Level::Debug == currentLog.level);
        REQUIRE("Dead Parrot" == currentLog.content);
        REQUIRE(__FILE__ == currentLog.file);
    }

    SECTION("Test whitespace")
    {
        logDbg() << "Norwegian" << "Blue";

        REQUIRE("Norwegian Blue" == currentLog.content);
    }

    SECTION("Test integral")
    {
        logDbg() << 42;

        REQUIRE("42" == currentLog.content);
    }

    SECTION("Test float")
    {
        logDbg() << 42.1;

        REQUIRE("42.1" == currentLog.content);
    }

    SECTION("Test bool")
    {
        logDbg() << "Is dead?" << true;

        REQUIRE("Is dead? true" == currentLog.content);

        logDbg() << "Is live?" << false;

        REQUIRE("Is live? false" == currentLog.content);
    }

    SECTION("Test ptr")
    {
        logDbg() << &currentLog;
        std::stringstream ss;
        ss << std::hex << &currentLog;

        REQUIRE(Ftylog::Level::Debug == currentLog.level);
        REQUIRE(ss.str() == currentLog.content);
        REQUIRE(__FILE__ == currentLog.file);
    }

    SECTION("Test condition")
    {
        bool runned = false;
        auto caller = [&](){
            runned = true;
            return "It's dead, that's what's wrong with it.";
        };

        logDbgIf(false) << caller();

        REQUIRE(!runned);
        REQUIRE("" == currentLog.content);

        logDbgIf(true) << caller();

        REQUIRE(runned);
        REQUIRE("It's dead, that's what's wrong with it." == currentLog.content);
    }

    SECTION("Test vector")
    {
        std::vector<std::string> lst = {"this", "is", "an", "ex-parrot"};
        logDbg() << lst;
        REQUIRE("[this, is, an, ex-parrot]" == currentLog.content);
    }

    SECTION("Test map")
    {
        std::map<std::string, std::string> map = {{"bereft", "of life"}, {"it rests", "in peace"}};
        logDbg() << map;
        REQUIRE("{{bereft : of life}, {it rests : in peace}}" == currentLog.content);
    }

    SECTION("Test struct")
    {
        logDbg() << MyStruct{"is no more", 42};
        REQUIRE("MyStruct{val = is no more; num = 42}" == currentLog.content);
    }
}
