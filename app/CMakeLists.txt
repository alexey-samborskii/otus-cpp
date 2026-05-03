
add_executable(bayan
    main.cpp
)

target_link_libraries(bayan
    PRIVATE
        bayan_core
)

install(TARGETS bayan RUNTIME DESTINATION bin)