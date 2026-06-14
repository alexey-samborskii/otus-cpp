add_executable(async_web_server_tests
  HttpResponseTests.cpp
  HttpRequestHandlerTests.cpp
)

target_link_libraries(async_web_server_tests
  PRIVATE
    async_web_server::async_web_server
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(async_web_server_tests)