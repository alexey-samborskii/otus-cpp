set(PROJECT_INSTALL_DATA_DIR
    "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/project_config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/project_config.hpp
    @ONLY
)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(fashion_mnist
    ${SOURCES}
)

target_include_directories(fashion_mnist
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
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

install(
    FILES
        ${CMAKE_SOURCE_DIR}/input/logreg_coef.txt
        ${CMAKE_SOURCE_DIR}/input/test.csv
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/input
)