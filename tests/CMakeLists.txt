ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
include_directories(${call_center_SOURCE_DIR}/lib/include)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${call_center_SOURCE_DIR}/lib/src)

enable_testing()

add_executable(tests tests_ts_queue.cpp)

target_link_libraries(tests PRIVATE ${Boost_LIBRARIES} gtest_main lib)
include(GoogleTest)
gtest_discover_tests(tests)
