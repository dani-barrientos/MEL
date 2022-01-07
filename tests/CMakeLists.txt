# CMakeList.txt : CMake project for tests, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
add_compile_definitions(DABAL_IMPORTS)
add_executable (tests "main.cpp")
target_include_directories(tests PUBLIC "common/include")
target_sources(tests PRIVATE  "common/source/CommandLine.cpp" "common/source/TestManager.cpp"  "test_callbacks/test.cpp" "test_threading/test.cpp")

#add_link_options()
target_link_libraries(tests "dabal")
if(DABAL_API STREQUAL "DABAL_POSIX")
    target_link_libraries(tests "pthread")
endif()
#copy dll on Windows @todo se haraá lo mismo cuando tenga el formato shared
if(DABAL_OS STREQUAL "DABAL_WINDOWS")
add_custom_command(TARGET tests POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE_DIR:dabal>"/dabal.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:tests>)                 # <--this is out-file path
endif()

enable_testing()
add_test(NAME Run1 COMMAND tests -t callbacks)
add_test(NAME Run2 COMMAND tests -t callbacks)


