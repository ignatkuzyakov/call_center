cmake_minimum_required(VERSION 3.11)

project(call_center
        VERSION 1.0.0
        DESCRIPTION "Call Center Emulator"
        HOMEPAGE_URL https://github.com/ignatkuzyakov/call_center.git
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(TESTING)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest
            GIT_TAG 76bb2afb8b522d24496ad1c757a49784fbfa2e42
)
    FetchContent_MakeAvailable(googletest)
endif()

find_package(Boost COMPONENTS log system thread REQUIRED)

if(NOT Boost_FOUND)
        set(Boost_USE_STATIC_LIBS OFF)
        set(Boost_USE_STATIC_RUNTIME OFF)
        set(Boost_USE_MULTITHREAD ON)
        set(BOOST_ENABLE_CMAKE ON)

        FetchContent_Declare(
                Boost
                GIT_REPOSITORY https://github.com/boostorg/boost.git
                GIT_TAG boost-1.83.0
                GIT_PROGRESS TRUE
                GIT_SHALLOW  TRUE
                USES_TERMINAL_DOWNLOAD TRUE
                CONFIGURE_COMMAND "")
        FetchContent_MakeAvailable(Boost)
endif()

add_subdirectory(main) 
add_subdirectory(lib)

if(TESTING)
    add_subdirectory(tests)
endif()
