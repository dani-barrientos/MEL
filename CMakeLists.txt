# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

if(NOT DEFINED ENV{VCPKG_ROOT})
  message("vcpkg root not defined!.Missing?")
endif()
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message("Using vcpkg")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")    
endif()
message( "Toolchain " ${CMAKE_TOOLCHAIN_FILE})

project ("dabal")

message("MENSAJE DE PRUEBA " ${DABAL_OS})

#add_compile_definitions(PROCESSSCHEDULER_USE_LOCK_FREE)
add_compile_definitions(${DABAL_OS})
add_compile_definitions(${DABAL_ARQ})
# Include sub-projects.
add_subdirectory("dabal")
add_subdirectory ("tests")

#seguramente lo mejor sea tener los test en otro archivo y hacer un include
#enable_testing()

