# CMakeList.txt : CMake project for tests, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Add source to this project's executable.
add_compile_definitions(MEL_IMPORTS)
if ( MSVC )
#with /permissive- should be sufficient, but since version  19.28 to 19.31 is neccesary to add /Zx:lambda 
#see https://docs.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance?view=msvc-170 for details
#/bigobj options is needed when there is a strong use of templates with lead to big .obj
    add_compile_options(/permissive- /Zc:lambda /bigobj)
    
endif()

#add_executable (tests "main.cpp")
add_library(tests STATIC  "tests_main.cpp" "common/source/CommandLine.cpp" "common/source/BaseTest.cpp" "common/source/TestManager.cpp"  "test_callbacks/test.cpp" 
"test_threading/test.cpp" "test_threading/test_samples.cpp"
"test_threading/future_tests.cpp" "test_parallelism/test.cpp" "test_execution/test.cpp" "test_execution/test_samples.cpp")

target_include_directories(tests PUBLIC "common/include")
if(MEL_OS STREQUAL "MEL_WINDOWS")
    # target_compile_options(tests PRIVATE /permissive-)    
endif()

target_link_libraries(tests "mel")
if(MEL_OS STREQUAL "MEL_LINUX")
    target_link_libraries(tests "pthread")
endif()

if (USE_SPDLOG)    
    message ("Tests is using spdlog")
    find_package(spdlog CONFIG REQUIRED)
    target_link_libraries(tests spdlog::spdlog spdlog::spdlog_header_only)
endif()



