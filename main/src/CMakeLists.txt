add_definitions(-DBOOST_LOG_DYN_LINK)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${MyProject_SOURCE_DIR}/lib/include)
link_directories(${MyProject_SOURCE_DIR}/lib/src)

add_executable(main main.cpp)

target_link_libraries(main 
  lib
  Boost::log
  Boost::system
  Boost::thread
  )

