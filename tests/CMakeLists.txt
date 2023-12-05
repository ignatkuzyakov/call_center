add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS)
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories(${call_center_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${call_center_SOURCE_DIR}/lib/src)

add_executable(tests_functional tests_functional.cpp)
target_link_libraries(tests_functional PRIVATE 
  ${CMAKE_THREAD_LIBS_INIT}
  gtest_main
  )

enable_testing()
add_executable(tests tests_ts_queue.cpp
                     tests_current_calls.cpp)

target_link_libraries(tests PRIVATE 
  ${CMAKE_THREAD_LIBS_INIT}
  Boost::system
  Boost::log
  Boost::thread
  gtest_main 
  lib)

include(GoogleTest)
gtest_discover_tests(tests)
