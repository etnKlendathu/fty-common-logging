etn_target(exe ${PROJECT_NAME}-test
    SOURCES
        main.cpp
        log.cpp
    CONFIGS
        conf/*
    USES
        ${PROJECT_NAME}
        Catch2::Catch2
)

ParseAndAddCatchTests(${PROJECT_NAME}-test)
