ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
include_directories(${call_center_SOURCE_DIR}/lib/include)
file(GLOB sources  ${call_center_SOURCE_DIR}/lib/src/*.cpp)
add_library(lib ${sources})
