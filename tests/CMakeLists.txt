add_executable(bayan_tests
    DuplicateFinderTests.cpp
)

target_link_libraries(bayan_tests
    PRIVATE
        bayan_core
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(bayan_tests)