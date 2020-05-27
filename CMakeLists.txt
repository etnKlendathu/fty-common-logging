cmake_minimum_required(VERSION 3.13)
cmake_policy(VERSION 3.13)


##############################################################################################################
find_package(fty-cmake PATHS ${CMAKE_BINARY_DIR})
##############################################################################################################

project(fty-common-logging 
    VERSION 1.0.0
)

##############################################################################################################

etn_target(shared ${PROJECT_NAME}
    PUBLIC
        fty/fty_logger.h
        fty/fty-log.h
    SOURCES
        src/log.cpp
        src/logger.cpp
    USES_PUBLIC
        fty-utils
    USES
        log4cplus
)


if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
