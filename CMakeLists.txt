cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)
find_package(raven-cmake PATHS ${CMAKE_BINARY_DIR})

##############################################################################################################

project(fty-common-logging VERSION 1.0.0)

##############################################################################################################

raven_target(shared ${PROJECT_NAME}
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

##############################################################################################################

raven_define_component(${PROJECT_NAME}-dev
    TARGETS ${PROJECT_NAME}
)

raven_define_component(${PROJECT_NAME}-runtime
    SHLIBDEPS
    TARGETS ${PROJECT_NAME}
)

raven_pack(
    PKG_NAME        "fty-common-logging"
    CONTACT         "zjesclean@gmail.com"
    COMPONENTS      ${PROJECT_NAME}-dev ${PROJECT_NAME}-runtime
    DESCRIPTION     "Loggin library"
    LD_LIBRARY_PATH "${RUNTIME_PREFIX}/lib"
)

##############################################################################################################

add_subdirectory(test)
