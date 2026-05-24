find_package(Boost REQUIRED COMPONENTS system)

add_executable(bulk_server
    main.cpp
    Server.cpp
    Session.cpp
)

target_link_libraries(bulk_server
    PRIVATE
        async
        Boost::system
)

install(
    TARGETS bulk_server
    RUNTIME DESTINATION bin
)