find_package(GTest REQUIRED)

add_executable(matrix_tests
    MatrixTests.cpp
)

target_link_libraries(matrix_tests 
    PRIVATE
        GTest::gtest_main
)

target_include_directories(matrix_tests 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

include(GoogleTest)

gtest_discover_tests(matrix_tests)