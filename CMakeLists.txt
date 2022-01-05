# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("dabal")
add_compile_definitions(${DABAL_OS})
add_compile_definitions(${DABAL_API})
add_compile_definitions(${DABAL_ARQ})
# Include sub-projects.
add_subdirectory("dabal")
add_subdirectory ("tests")

#seguramente lo mejor sea tener los test en otro archivo y hacer un include
#enable_testing()

