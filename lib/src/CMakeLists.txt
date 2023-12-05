add_definitions(-DBOOST_LOG_DYN_LINK)
add_definitions(-DBOOST_ALLOW_DEPRECATED_HEADERS)
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

include_directories(${call_center_SOURCE_DIR}/lib/include)
file(GLOB sources  ${call_center_SOURCE_DIR}/lib/src/*.cpp)

add_library(lib ${sources})
