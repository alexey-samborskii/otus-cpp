add_library(task_system STATIC
    src/tasks/Task.cpp
    src/tasks/TaskRepository.cpp
    src/tasks/TaskScheduler.cpp
    src/tasks/TaskService.cpp)

add_library(task_system::task_system ALIAS task_system)

target_include_directories(task_system
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${Boost_INCLUDE_DIRS})

target_compile_features(task_system
    PUBLIC
        cxx_std_20)

target_compile_definitions(task_system
    PUBLIC
        BOOST_ASIO_HAS_CO_AWAIT
        BOOST_ERROR_CODE_HEADER_ONLY)

target_compile_options(task_system
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic)

target_link_libraries(task_system
    PUBLIC
        Threads::Threads
    PRIVATE
        common::common
        SQLite::SQLite3)

set_target_properties(task_system
    PROPERTIES
        OUTPUT_NAME task_system)
