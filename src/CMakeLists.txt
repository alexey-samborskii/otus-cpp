add_executable(editor
    main.cpp
)

target_include_directories(editor
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src
)

install(TARGETS editor RUNTIME DESTINATION bin)


