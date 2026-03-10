add_executable(alllocator
    main.cpp
)

target_include_directories(alllocator 
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

install(TARGETS alllocator RUNTIME DESTINATION bin)


