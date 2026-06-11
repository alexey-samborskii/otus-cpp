find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_library(async_web_server_lib
  src/HttpRequest.cpp
  src/HttpRequestHandler.cpp
  src/HttpResponse.cpp
  src/HttpSession.cpp
  src/HttpWebServer.cpp
  src/HttpsSession.cpp
  src/HttpsWebServer.cpp
  src/Router.cpp
  src/SslContext.cpp
  src/StaticFileHandler.cpp
)

target_include_directories(async_web_server_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(async_web_server_lib
  PUBLIC
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

target_compile_features(async_web_server_lib
  PUBLIC
    cxx_std_20
)

add_executable(async_web_server
  src/main.cpp
)

target_link_libraries(async_web_server
  PRIVATE
    async_web_server_lib
)

install(TARGETS async_web_server
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

file(COPY
  ${CMAKE_CURRENT_SOURCE_DIR}/public
  DESTINATION
  ${CMAKE_CURRENT_BINARY_DIR}
)