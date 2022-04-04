# CMakeList.txt : Top-level CMake project file
# main executable using tests library for executing tests
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_CLANG_TIDY 
  clang-tidy;
  -header-filter=.;
  -checks=*;)

message("Building for " ${DABAL_OS})
message("Supported features = ${CMAKE_CXX_COMPILE_FEATURES}")
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
add_executable (main "main.cpp")
target_link_libraries(main "dabal")
target_link_libraries(main "tests")

#copy dll on Windows @todo se haraá lo mismo cuando tenga el formato shared
if(DABAL_OS STREQUAL "DABAL_WINDOWS")
add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE_DIR:dabal>"/dabal.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:main>)                 # <--this is out-file path
endif()
#seguramente lo mejor sea tener los test en otro archivo y hacer un include

set (failRegex "Fail")
add_test(NAME RunAll COMMAND main -a)
add_test(NAME TestCallbacks COMMAND main -t callbacks)
add_test(NAME TestExecution.Launch COMMAND main -t execution -n 0)
add_test(NAME TestExecution.For COMMAND main -t execution -n 1)
add_test(NAME TestThreading.SimpleMicrothreads COMMAND main -t threading -n 0)
add_test(NAME TestThreading.LotsTasks COMMAND main -t threading -n 1)
add_test(NAME TestThreading.FutureTests COMMAND main -t threading -n 2)
add_test(NAME TestThreading.LockFree COMMAND main -t threading -n 3)
add_test(NAME TestThreading.Exceptions COMMAND main -t threading -n 4)
set_property(TEST RunAll TestCallbacks TestExecution.Launch TestExecution.For  TestThreading.SimpleMicrothreads 
TestThreading.LotsTasks TestThreading.FutureTests TestThreading.LockFree TestThreading.Exceptions PROPERTY FAIL_REGULAR_EXPRESSION "${failRegex}")

enable_testing() #se supone que no es necesario haciendo luego el include.., pero en algun sitio vi que también lo ponen
include(CTest) 


