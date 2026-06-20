add_library(async_web_server STATIC 
    src/server/HttpResponse.cpp 
    src/server/HttpSession.cpp 
    src/server/HttpSessionBase.cpp 
    src/server/HttpsSession.cpp 
    src/server/HttpWebServer.cpp 
    src/server/HttpsWebServer.cpp 
    src/server/StaticFileHandler.cpp)

add_library(
    async_web_server::async_web_server
    ALIAS
    async_web_server)

target_sources(async_web_server
    PRIVATE
        ${WEB_SERVER_SOURCES}
        ${WEB_SERVER_HEADERS})

target_include_directories(async_web_server
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

target_compile_features(async_web_server
    PUBLIC
        cxx_std_20)

set_target_properties(async_web_server
    PROPERTIES
        CXX_EXTENSIONS OFF)

target_compile_options(async_web_server
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -Wall
            -Wextra
            -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:
            /W4>)

target_link_libraries(async_web_server
    PUBLIC
        Boost::headers
        OpenSSL::SSL
        Threads::Threads
    PRIVATE
        common::common)