cmake_minimum_required(VERSION 3.11)
project(call_center)

set(CMAKE_CXX_STANDARD 11)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
find_package(Boost COMPONENTS thread system log REQUIRED)
#find_package(GTest REQUIRED)

set(SOURCE_FILES 
    configure.cpp
    configure.hpp
    call_center.cpp
    call_center.hpp
    current_calls.cpp
    current_calls.hpp
    logger.hpp
    listener.cpp
    listener.hpp
    notify_interface.hpp
    notify_interface.cpp
    session.cpp
    session.hpp
    ts_queue.hpp
    main.cpp)

#enable_testing()

add_executable(call_center ${SOURCE_FILES})

include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(call_center PRIVATE ${Boost_LIBRARIES})

#target_link_libraries(call_center PRIVATE ${Boost_LIBRARIES} ${GTEST_BOTH_LIBRARIES})
#target_include_directories(call_center PRIVATE ${GTEST_INCLUDE_DIRS})

#gtest_discover_tests(call_center)
