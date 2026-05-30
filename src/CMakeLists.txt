if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost CONFIG REQUIRED COMPONENTS system)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(join_server
    ${SOURCES}
)

target_link_libraries(join_server
    PRIVATE
        Boost::system
)

install(
    TARGETS join_server
    RUNTIME DESTINATION bin
)