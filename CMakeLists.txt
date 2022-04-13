# CMakeList.txt : Top-level CMake project file
# main executable using tests library for executing tests
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#[[
tengo que mirar bien la doc del clang-tidy porque sale demasiada mierda
set(CMAKE_CXX_CLANG_TIDY 
  clang-tidy;
  -header-filter=.;
  -checks=*;)
]]
message("Building for " ${MEL_OS})
message("Supported features = ${CMAKE_CXX_COMPILE_FEATURES}")
if(NOT DEFINED ENV{VCPKG_ROOT})
  message("vcpkg root not defined!.Missing?")
endif()
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message("Using vcpkg")
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")    
endif()
message( "Toolchain " ${CMAKE_TOOLCHAIN_FILE})

project ("mel")

add_compile_definitions(${MEL_OS})
add_compile_definitions(${MEL_ARQ})
if(USE_SPDLOG)
    add_compile_definitions(USE_SPDLOG)
endif()
# Include sub-projects.
add_subdirectory("mel")
add_subdirectory ("tests")
add_executable (main "main.cpp")
target_link_libraries(main "mel")
target_link_libraries(main "tests")
if(MEL_OS STREQUAL "MEL_LINUX")
    target_link_libraries(main "pthread")
endif()
if(MEL_OS STREQUAL "MEL_EMSCRIPTEN")

  if(JS_ONLY)
    message(STATUS "Setting compilation target to native JavaScript")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set_target_properties(main PROPERTIES LINK_FLAGS "-s WASM=0 -s EXPORTED_FUNCTIONS='[_main]' -s NO_DISABLE_EXCEPTION_CATCHING -s USE_PTHREADS")
  else()
    message(STATUS "Setting compilation target to WASM")
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm.js")
    #set_target_properties(main PROPERTIES LINK_FLAGS "-s WASM=1 -s EXPORTED_FUNCTIONS='[_main]' -s NO_DISABLE_EXCEPTION_CATCHING -s USE_PTHREADS")
    set_target_properties(main PROPERTIES LINK_FLAGS "-s WASM=1 -s EXPORTED_FUNCTIONS='[_main]' -s NO_DISABLE_EXCEPTION_CATCHING -s USE_PTHREADS -s PTHREAD_POOL_SIZE=10 -s ASYNCIFY")
  endif()
  #I think that this options are superseded using -s USE_PTHREADS
  #target_compile_options(main PRIVATE "-pthread")    
  #target_link_options(main PRIVATE "-pthread")    
endif()

#copy dll on Windows @todo se haraá lo mismo cuando tenga el formato shared
if(MEL_OS STREQUAL "MEL_WINDOWS")
add_custom_command(TARGET main POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE_DIR:mel>"/mel.dll"      # <--this is in-file
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


