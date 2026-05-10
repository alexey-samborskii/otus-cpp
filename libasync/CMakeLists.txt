find_package(Threads REQUIRED)

add_library(async SHARED
    src/async.cpp
    src/AsyncContext.cpp
    src/AsyncCommandBlockDispatcher.cpp
    src/CommandBlockProcessor.cpp
    src/CompositeHandler.cpp
    src/ConsoleHandler.cpp
    src/FileHandler.cpp
    src/Formatter.cpp
)

target_include_directories(async
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(async
    PRIVATE
        Threads::Threads
)

set_target_properties(async PROPERTIES
    OUTPUT_NAME async
)

install(
    TARGETS async
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)