cmake_minimum_required(VERSION 3.10)

project(multi_master_tool)

option(debug "cmake build mode" OFF)

IF(debug)
    message(STATUS "DEBUG!!!!!")
    add_subdirectory(easy_logger)
ENDIF()

add_subdirectory(multi_net_io)

add_subdirectory(centralized-lock-table)