if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

find_package(Boost REQUIRED COMPONENTS filesystem program_options system)

add_library(bayan_core STATIC
    src/OptionsParser.cpp
    src/FileScanner.cpp
    src/Hasher.cpp
    src/BlockReader.cpp
    src/DuplicateFinder.cpp
    src/OutputPrinter.cpp
)

target_include_directories(bayan_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bayan_core
    PUBLIC
        Boost::filesystem
        Boost::program_options
        Boost::system
)