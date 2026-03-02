add_executable(ip_filter_tests
    ip_filter_tests.cpp
)

target_link_libraries(ip_filter_tests
    GTest::gtest_main
)

target_include_directories(ip_filter_tests 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(ip_filter_tests)

add_custom_command(
    TARGET ip_filter_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_LIST_DIR}/ip_filter.tsv"
        "$<TARGET_FILE_DIR:ip_filter_tests>/ip_filter.tsv"
    VERBATIM
)