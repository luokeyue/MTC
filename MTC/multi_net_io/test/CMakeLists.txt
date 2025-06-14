# test CMakeLists.list
enable_testing()

set(TEST_SOURCE_DIR "${CMAKE_SOURCE_DIR}/multi_net_io/test/")
configure_file("test_config.h.in" "test_config.h")


# test functions
function(compile_and_test exec_file code_file arg result)
    add_executable(${exec_file} ${code_file})

    add_test(NAME test_target_${exec_file} COMMAND ${exec_file} ${arg})
    set_tests_properties(test_target_${exec_file} PROPERTIES PASS_REGULAR_EXPRESSION ${result})
endfunction(compile_and_test)

# test all "_test" file
aux_source_directory(. TEST_SUFFIX_FILE)
#file(GLOB TEST_SUFFIX_FILE ./*_test.cpp)
foreach(f ${TEST_SUFFIX_FILE})
    string(REGEX REPLACE "_test.cc$" "_test_target" exec_name ${f})
    string(REGEX REPLACE "^.*/" "" exec_name ${exec_name})
    #MESSAGE( STATUS "target name = ${exec_name}")
    #MESSAGE(STATUS "TOP DIR = ${PROJECT_SOURCE_DIR}")
    compile_and_test(${exec_name} ${f} "" "TEST SUCCESS")
    #target_link_libraries(${exec_name} common_net_lib)
   # target_link_libraries(${exec_name} event)
    target_link_libraries(${exec_name} multi_net_io)
endforeach(f)

