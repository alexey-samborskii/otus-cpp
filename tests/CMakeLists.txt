add_executable(bulk_tests
    BulkProcessorTests.cpp
)

target_link_libraries(bulk_tests
    PRIVATE
        bulk_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(bulk_tests)