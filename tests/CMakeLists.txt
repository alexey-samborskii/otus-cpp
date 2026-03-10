add_executable(alllocator_tests
    alllocator_tests.cpp
)

target_link_libraries(alllocator_tests
    GTest::gtest_main
)

target_include_directories(alllocator_tests 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(alllocator_tests) 