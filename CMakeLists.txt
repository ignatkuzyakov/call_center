cmake_minimum_required(VERSION 3.11)
project(call_center)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES 
    configure.cpp
    configure.hpp
    call_center.cpp
    call_center.hpp
    current_calls.cpp
    current_calls.hpp
    listener.cpp
    listener.hpp
    notify_interface.hpp
    notify_interface.cpp
    session.cpp
    session.hpp
    ts_queue.hpp
    main.cpp)

add_executable(call_center ${SOURCE_FILES})

find_package(Boost COMPONENTS thread system REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(call_center ${Boost_LIBRARIES})