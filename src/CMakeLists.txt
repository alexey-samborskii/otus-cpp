add_executable(matrix
    main.cpp
)

target_include_directories(matrix
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

install(TARGETS matrix RUNTIME DESTINATION bin)


