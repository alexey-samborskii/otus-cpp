add_executable(ip_filter
    main.cpp
)

target_include_directories(ip_filter 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

install(TARGETS ip_filter RUNTIME DESTINATION bin)


