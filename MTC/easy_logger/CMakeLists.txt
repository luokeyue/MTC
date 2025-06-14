cmake_minimum_required (VERSION 3.0)

#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/easy_logger/include/)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

add_executable(easy_log_test_main test_main.cc)
target_link_libraries(easy_log_test_main  pthread)