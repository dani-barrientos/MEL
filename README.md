[![CMake](https://github.com/dani-barrientos/MEL/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/dani-barrientos/MEL/actions/workflows/cmake.yml)
# MEL
See https://dani-barrientos.github.io/MEL for MEL documentation
# vcpkg
 If CMake variable **USE_SPDLOG** is set to *true*. code is compiled using spdlog as the logging library. In this case vcpkg need to be installed. As a summary:
   - Follow installacion instructions in https://vcpkg.io/en/getting-started.html. It consists basically on cloning the vcpkg github repository and execute a couple of scripts
   - In order for cmake to be able to find packages,set environment variable VCPKG_ROOT pointing to vcpkg root

# Build
Very brief guide on how to build.

*CMakePresets.json* file includes the base configuration for builds in Windows, MacOsX and Linux, but you need to provide your *CMakeUserPresets* for your concrete case. A *CMakeUserPresets_EXAMPLE.json* is included as a guide toi crete your custom configuration.
There are 2 diferent macros that guide compilation based on target platform.
- **MEL_OS** : target operating system. Available OS are: *MEL_WINDOWS*, *MEL_LINUX*,*MEL_MACOSX*,*MEL_ANDROID* (iOS platform is working progress)
- **MEL_ARQ**: CPU arquitecture and compiler, mainly for assembler syntax resolution
Issues/facts:
- CMAKE_BUILD_TYPE hasn't any effect with Visual Studio Generator. It has to be done in a "build preset" with the "configuration" parameter
- MacOSX:
   - I was unable to set VCPKG_ROOT environment variable in my machine. So I set the **CMAKE_TOOLCHAIN_FILE** variable in section *chacheVariables in CMakeUserPresets.json pointing to the vcpkg path (in my case /Users/dani/vcpkg/scripts/buildsystems/vcpkg.cmake)
- Android:
   - Android Studio project in folder [android](/android/main).
   - Because Android Studio is based on cmake, integration is quite straight, but symbolic links to source code need to be because proyect structure requirement in Android Studio
   - If you want to use *spdlog*, concrete android triplet need to be installed (https://vcpkg.readthedocs.io/en/latest/users/android/#android-build-requirements)
- Visual Studio: a bug in lambda processing with /Zc:lambda option (which is neccesary) for MSVC previous to 19.31 (Visual Studio 2022 17.1) generate error in some cases with nested lambdas. For example:
  
      
      #include <exception>
      auto test_lambda  = [](auto ex) 
      {

         auto g = [](std::exception_ptr err){};	
      };

      void exec()
      {
         test_lambda(1);
      }
      
   At the present moment this error has only be shown with a case as that, using exception_ptr, but could possibly appear in other situations.
  
 # Testing
 If using Visual Studio Code, execute *Run CTest*,or execute ctest from command line in out/build/[required configuration]/tests
 Tests runs automatically when build action runs (whenever a change in source code is done) and sent to: https://my.cdash.org/index.php?project=mel
 
 
