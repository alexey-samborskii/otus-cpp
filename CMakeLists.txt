cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 1)

if(DEFINED PATCH_VERSION)
  set(PROJECT_VERSION_PATCH ${PATCH_VERSION})
endif()

project(async_web_server
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

add_subdirectory(async_web_server)

#include(CTest)
#
#if(BUILD_TESTING)
#  include(FetchContent)
#
#  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
#
#  FetchContent_Declare(
#    googletest
#    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
#    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
#  )
#
#  FetchContent_MakeAvailable(googletest)
#
#  add_subdirectory(tests)
#endif()

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "async_web_server")
set(CPACK_DEBIAN_PACKAGE_NAME "async_web_server")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "otus")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)