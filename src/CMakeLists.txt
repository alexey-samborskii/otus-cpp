add_executable(print_ip
    main.cpp
)

target_include_directories(print_ip 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

install(TARGETS print_ip RUNTIME DESTINATION bin)