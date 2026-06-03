file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(final_project_work
    ${SOURCES}
)

target_compile_options(final_project_work PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(
    TARGETS
        final_project_work
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
