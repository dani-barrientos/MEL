# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.8)

# Declares and names the project.
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
message( "Toolchain2 " ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE})
project("main")

add_compile_definitions(${DABAL_OS})
add_compile_definitions(${DABAL_ARQ})
if(USE_SPDLOG)
    add_compile_definitions(USE_SPDLOG)
endif()

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_subdirectory("dabal")
add_subdirectory("tests")

add_library( # Sets the name of the library.
        main

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)
target_link_libraries(main "dabal")
target_link_libraries(main "tests")
# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
#[[
find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)
]]
# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
#[[
target_link_libraries( # Specifies the target library.
        myapplication

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
]]