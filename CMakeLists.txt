cmake_minimum_required(VERSION 3.11)
project(call_center)

set(CMAKE_CXX_STANDARD 14)

include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG 76bb2afb8b522d24496ad1c757a49784fbfa2e42
)
FetchContent_MakeAvailable(googletest)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
find_package(Boost COMPONENTS thread system log REQUIRED)

add_subdirectory(main) 
add_subdirectory(lib)
add_subdirectory(tests)
