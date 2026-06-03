file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(fashion_mnist
    ${SOURCES}
)

target_compile_options(fashion_mnist PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(
    TARGETS
        fashion_mnist
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
