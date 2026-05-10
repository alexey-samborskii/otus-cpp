add_executable(bulk_tests
    BulkProcessorTests.cpp
)

target_include_directories(bulk_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/libasync/src
)

target_link_libraries(bulk_tests
    PRIVATE
        async
        gtest
        gtest_main
)

include(GoogleTest)

gtest_discover_tests(bulk_tests)