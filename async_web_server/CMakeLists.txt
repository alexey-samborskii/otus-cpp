find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(async_web_server
    ${SOURCES}
)

add_custom_command(
    TARGET async_web_server
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/public"
        "$<TARGET_FILE_DIR:async_web_server>/public"
)

target_include_directories(async_web_server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(async_web_server
    PRIVATE
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

target_compile_options(async_web_server
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

install(
    TARGETS
        async_web_server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)