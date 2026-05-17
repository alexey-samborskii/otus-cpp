add_executable(print_ip_tests
    print_ip_tests.cpp
)

target_link_libraries(print_ip_tests
    GTest::gtest_main
)

target_include_directories(print_ip_tests 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(print_ip_tests) 