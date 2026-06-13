file(GLOB_RECURSE APP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB_RECURSE APP_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(async_task_web_server)

target_sources(async_task_web_server
    PRIVATE
        ${APP_SOURCES}
        ${APP_HEADERS})

target_include_directories(async_task_web_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(async_task_web_server
    PRIVATE
        cxx_std_20)

set_target_properties(async_task_web_server
    PROPERTIES
        CXX_EXTENSIONS OFF)

target_compile_options(async_task_web_server
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic)

target_link_libraries(async_task_web_server
    PRIVATE
        async_web_server::async_web_server
        task_system::task_system)

add_custom_command(
    TARGET async_task_web_server
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/public"
        "$<TARGET_FILE_DIR:async_task_web_server>/public"
    COMMENT
        "Copying web interface to the binary directory")