cmake_minimum_required(VERSION 3.14)
project(adder
        VERSION 1.0.0
        DESCRIPTION "Cross Platfrom Multithreading library"
        HOMEPAGE_URL "https://github.com/chernetskyi/cpp-template"
        LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Options
option(WARNINGS_AS_ERRORS   "Treat compiler warnings as errors."    ON)
option(ENABLE_CONAN         "Use Conan as a package manager."       OFF)
option(ENABLE_PVS_STUDIO    "Use PVS-Studio static code analyzer."  OFF) # Option for the local usage only. PVS-Studio isn't installed on GitHub machines.
option(ENABLE_SANITIZERS    "Use sanitizers to detect errors."      OFF) # Option for the test builds. Do not use it in the production builds.

# Project source compilation
include_directories(inc)
add_library(multithreading STATIC
        src/multithreading/EventClass.cpp
        inc/multithreading/EventClass.hpp
        src/multithreading/Thread.cpp
        inc/multithreading/Thread.hpp
        src/multithreading/MutexClass.cpp
        inc/multithreading/MutexClass.hpp
        src/multithreading/ThreadPool.cpp
        inc/multithreading/ThreadPool.h)
add_executable(add src/main.cpp inc/multithreading/ThreadPool.h)
target_link_libraries(add multithreading)

# Add external packages
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.71.0 COMPONENTS program_options REQUIRED)
find_package (Threads)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(add Boost::program_options ${CMAKE_THREAD_LIBS_INIT})

# Define ALL_TARGETS variable to use in some directives
set(ALL_TARGETS multithreading add)

# Include default CMake configuration
include(cmake/Config.cmake)
