cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

IF(debug)
    message(STATUS " Libevent DEBUG!!!!!")
    SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
    SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
    ADD_SUBDIRECTORY(libevent)
    include_directories(${CMAKE_BINARY_DIR}/multi_net_io/libevent/include)
    include_directories(libevent/include)
ENDIF()

include_directories(./include)
include_directories(./include/rapidjson)

aux_source_directory(./src DIR_LIB_SRCS)
add_library(multi_net_io ${DIR_LIB_SRCS})

target_link_libraries(multi_net_io event)
target_link_libraries(multi_net_io pthread)

IF(debug)
## Test
    enable_testing()
    add_subdirectory(test)
ENDIF()