# Чтобы бинарники складывались в ./bin,
# как ожидает run_locally.sh из заготовки.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

#-------------------------------------------------------------------------------

add_executable(mapper_mean
    mapper_mean.cpp
)

target_compile_options(mapper_mean PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

#-------------------------------------------------------------------------------

add_executable(reducer_mean
    reducer_mean.cpp
)

target_compile_options(reducer_mean PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

#-------------------------------------------------------------------------------

add_executable(mapper_variance
    mapper_variance.cpp
)

target_compile_options(mapper_variance PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

#-------------------------------------------------------------------------------

add_executable(reducer_variance
    reducer_variance.cpp
)

target_compile_options(reducer_variance PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

#-------------------------------------------------------------------------------

add_custom_target(run_locally
    COMMAND sh ${CMAKE_SOURCE_DIR}/test/run_locally.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS
        mapper_mean
        reducer_mean
        mapper_variance
        reducer_variance
)

#-------------------------------------------------------------------------------

install(
    TARGETS
        mapper_mean
        reducer_mean
        mapper_variance
        reducer_variance
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    PROGRAMS
        ${CMAKE_SOURCE_DIR}/test/run_locally.sh
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES
        ${CMAKE_SOURCE_DIR}/input/AB_NYC_2019.csv
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/input
)