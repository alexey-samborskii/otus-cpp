add_library(bulk_lib
    Formatter.cpp
    CommandBlockProcessor.cpp
    CompositeHandler.cpp
    ConsoleHandler.cpp
    FileHandler.cpp
)

target_include_directories(bulk_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

add_executable(bulk
    main.cpp
)

target_link_libraries(bulk
    PRIVATE
        bulk_lib
)

install(TARGETS bulk RUNTIME DESTINATION bin)