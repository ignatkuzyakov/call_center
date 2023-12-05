add_definitions(-DBOOST_LOG_DYN_LINK)
include_directories(${Boost_INCLUDE_DIR})
include_directories(${MyProject_SOURCE_DIR}/lib/include)
link_directories(${MyProject_SOURCE_DIR}/lib/src)

add_executable(abonent ab.cpp)
add_executable(callCenter cc.cpp)

target_link_libraries(abonent 
  lib
  Boost::system
  Boost::thread
  )
target_link_libraries(callCenter 
  lib
  Boost::log
  Boost::system
  Boost::thread
  )
