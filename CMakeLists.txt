cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(ip_filter VERSION ${PROJECT_VESRION})

option(WITH_BOOST_TEST "Whether to build Boost test" ON)

configure_file(version.h.in version.h)

#
# 1) library 'version'
#

add_library(version 
    lib.cpp
)

set_target_properties(version 
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(version
    PRIVATE 
        "${CMAKE_BINARY_DIR}"
)

#
# 2) executable 'ip_filter'
#

add_executable(ip_filter
    ip_filter.cpp
)

set_target_properties(ip_filter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(ip_filter
    version
)

#
# 3) executable 'test_version'
#

if(WITH_BOOST_TEST)
    find_package(Boost 
        COMPONENTS 
            unit_test_framework REQUIRED
    )

    add_executable(test_version 
        test_version.cpp
    )

    set_target_properties(test_version 
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )

    set_target_properties(test_version 
        PROPERTIES
            COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
            INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
    )

    target_link_libraries(test_version
        ${Boost_LIBRARIES}
        version
    )
endif()

#
# 4) compile options
#

if (MSVC)
    target_compile_options(version PRIVATE /W4)
    target_compile_options(ip_filter PRIVATE /W4)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE /W4)
    endif()
else ()
    target_compile_options(version PRIVATE -Wall -Wextra -pedantic -Werror)
    target_compile_options(ip_filter PRIVATE -Wall -Wextra -pedantic -Werror)
    if(WITH_BOOST_TEST)
        target_compile_options(test_version PRIVATE -Wall -Wextra -pedantic -Werror)
    endif()
endif()

#
# 5) install
#

install(TARGETS ip_filter RUNTIME DESTINATION bin)

#
# 6) set
#

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)

if(WITH_BOOST_TEST)
    enable_testing()
    add_test(test_version test_version)
endif()

#
# 7) copy file to '${CMAKE_CURRENT_BINARY_DIR}'
#
if (TARGET ip_filter)
    add_custom_command(
        TARGET ip_filter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_LIST_DIR}/ip_filter.tsv"
            "$<TARGET_FILE_DIR:ip_filter>/ip_filter.tsv"
        VERBATIM
    )
endif()