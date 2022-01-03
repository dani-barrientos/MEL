# CMakeList.txt : CMake project for sample, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
add_compile_definitions(DABAL_IMPORTS)
add_executable (sample "sample.cpp" "sample.h")

#add_link_options()
target_link_libraries(sample "dabal")
if(DABAL_API STREQUAL "DABAL_POSIX")
    target_link_libraries(sample "pthread")
endif()
#copy dll on Windows @todo se haraá lo mismo cuando tenga el formato shared
if(DABAL_OS STREQUAL "DABAL_WINDOWS")
add_custom_command(TARGET sample POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE_DIR:dabal>"/dabal.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:sample>)                 # <--this is out-file path
endif()
# TODO: Add tests and install targets if needed.

