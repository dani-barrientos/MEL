# CMakeList.txt : CMake project for tests, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Add source to this project's executable.
add_compile_definitions(DABAL_IMPORTS)


#add_executable (tests "main.cpp")
add_library(tests STATIC  "tests_main.cpp" "common/source/CommandLine.cpp" "common/source/BaseTest.cpp" "common/source/TestManager.cpp"  "test_callbacks/test.cpp" "test_threading/test.cpp"
"test_threading/future_tests.cpp" "test_parallelism/test.cpp" "test_execution/test.cpp" )
target_include_directories(tests PUBLIC "common/include")
#[[target_sources(tests PRIVATE  "common/source/CommandLine.cpp" "common/source/TestManager.cpp"  "test_callbacks/test.cpp" "test_threading/test.cpp"
"test_threading/future_tests.cpp" "test_parallelism/test.cpp" "test_execution/test.cpp"
)]]

#add_link_options()
target_link_libraries(tests "dabal")
if(DABAL_OS STREQUAL "DABAL_LINUX")
    target_link_libraries(tests "pthread")
endif()

if (USE_SPDLOG)    
    message ("Tests is using spdlog")
    find_package(spdlog CONFIG REQUIRED)
    target_link_libraries(tests spdlog::spdlog spdlog::spdlog_header_only)
endif()


