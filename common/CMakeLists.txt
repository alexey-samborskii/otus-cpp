add_library(common STATIC
    src/common/Logger.cpp)

add_library(common::common ALIAS common)

target_include_directories(common
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(common
    PUBLIC
        cxx_std_20)

target_compile_options(common
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic)

set_target_properties(common
    PROPERTIES
        OUTPUT_NAME common)
