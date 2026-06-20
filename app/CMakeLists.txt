include(GNUInstallDirs)

file(GLOB_RECURSE APP_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB_RECURSE APP_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

if(IS_ABSOLUTE "${CMAKE_INSTALL_BINDIR}")
    message(FATAL_ERROR
        "CMAKE_INSTALL_BINDIR must be a relative path")
endif()

if(IS_ABSOLUTE "${CMAKE_INSTALL_DATADIR}")
    message(FATAL_ERROR
        "CMAKE_INSTALL_DATADIR must be a relative path")
endif()

set(APP_GENERATED_INCLUDE_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/AppConfig.hpp.in"
    "${APP_GENERATED_INCLUDE_DIR}/AppConfig.hpp"
    @ONLY)

add_executable(async_task_web_server)

target_sources(async_task_web_server
    PRIVATE
        ${APP_SOURCES}
        ${APP_HEADERS}
        "${APP_GENERATED_INCLUDE_DIR}/AppConfig.hpp")

target_include_directories(async_task_web_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${APP_GENERATED_INCLUDE_DIR})

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
        common::common
        task_system::task_system)

add_custom_command(
    TARGET async_task_web_server
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/public"
            "$<TARGET_FILE_DIR:async_task_web_server>/public"
    COMMENT
        "Copying web interface to the binary directory")

add_custom_command(
    TARGET async_task_web_server
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/config
            $<TARGET_FILE_DIR:async_task_web_server>
)

install(
    TARGETS async_task_web_server
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR})

install(
    DIRECTORY
        "${PROJECT_SOURCE_DIR}/public/"
    DESTINATION
        "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/public")

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/config/
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)