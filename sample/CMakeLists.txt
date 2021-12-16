# CMakeList.txt : CMake project for sample, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (sample "sample.cpp" "sample.h")

target_link_libraries(sample "dabal")

# TODO: Add tests and install targets if needed.
