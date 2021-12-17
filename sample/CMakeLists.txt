# CMakeList.txt : CMake project for sample, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_compile_definitions(DABAL_IMPORTS)
add_executable (sample "sample.cpp" "sample.h")

target_link_libraries(sample "dabal")
add_custom_command(TARGET sample POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE_DIR:dabal>"/dabal.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:sample>)                 # <--this is out-file path

# TODO: Add tests and install targets if needed.
