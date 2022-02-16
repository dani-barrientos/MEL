# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
message("Building for " ${DABAL_OS})
if(NOT DEFINED ENV{VCPKG_ROOT})
  message("vcpkg root not defined!.Missing?")
endif()
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message("Using vcpkg")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")    
endif()
message( "Toolchain " ${CMAKE_TOOLCHAIN_FILE})

project ("dabal")



#add_compile_definitions(PROCESSSCHEDULER_USE_LOCK_FREE)
add_compile_definitions(${DABAL_OS})
add_compile_definitions(${DABAL_ARQ})
if(USE_SPDLOG)
    add_compile_definitions(USE_SPDLOG)
endif()
# Include sub-projects.
add_subdirectory("dabal")
add_subdirectory ("tests")
add_executable (main "tests/main.cpp")
target_link_libraries(main "dabal")
target_link_libraries(main "tests")
target_include_directories(main PUBLIC "tests/common/include")

#seguramente lo mejor sea tener los test en otro archivo y hacer un include
enable_testing()
add_test(NAME Run1 COMMAND tests -t callbacks)
add_test(NAME Run2 COMMAND tests -t threading -n 0)
add_test(NAME Run3 COMMAND tests -t threading -n 1)
add_test(NAME Run4 COMMAND tests -t threading -n 2)


