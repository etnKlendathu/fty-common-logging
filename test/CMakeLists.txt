find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)

raven_target(exe ${PROJECT_NAME}-test
    SOURCES
        main.cpp
        log.cpp
    CONFIGS
        conf/*
    USES
        ${PROJECT_NAME}
        Catch2::Catch2
)

catch_discover_tests(${PROJECT_NAME}-test)
