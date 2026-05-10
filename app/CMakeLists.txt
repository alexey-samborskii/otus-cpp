add_executable(async_demo
    main.cpp
)

target_link_libraries(async_demo
    PRIVATE
        async
)

install(
    TARGETS async_demo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)