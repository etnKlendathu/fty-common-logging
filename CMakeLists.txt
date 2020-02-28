cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
find_package(raven-cmake PATHS ${CMAKE_BINARY_DIR})

##############################################################################################################

project(fty-common-logging VERSION 1.0.0)

##############################################################################################################

raven_target(shared ${PROJECT_NAME}
    PUBLIC
        include/fty_logger.h
        include/fty-log.h

    SOURCES
        src/log.cpp
        src/logger.cpp

    USES
        log4cplus
)

##############################################################################################################

add_subdirectory(test)
