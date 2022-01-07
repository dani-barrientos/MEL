# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(DABAL_EXPORTS)



add_library(dabal SHARED "source/core/atomics.cpp" "source/core/CriticalSection_mthread.cpp"
"source/core/Event_mthread.cpp" "source/core/Process.cpp"
"source/core/GenericProcess.cpp"  "source/core/ProcessScheduler.cpp" "source/core/Runnable.cpp" "source/core/Timer.cpp" "source/core/Type.cpp" "source/core/Thread.cpp" "source/core/GenericThread.cpp" "source/core/TLS.cpp" 
"source/core/Event_win32.cpp" "source/core/Event_posix.cpp" "include/core/atomics.h" "include/core/CriticalSection_mthread.h"  "include/DabalLibType.h"
"source/core/Process_X64_GCC.cpp")

 #[[
 add_library(dabal SHARED "source/core/atomics.cpp" "source/core/Type.cpp" "source/core/TLS.cpp" 
 "source/core/Event_win32.cpp" "source/core/Event_posix.cpp" "include/core/atomics.h"   "include/DabalLibType.h"
)
]]

if(DABAL_ARQ STREQUAL "DABAL_X64_CLANG" OR DABAL_ARQ STREQUAL "DABAL_X64_GCC" )
    target_sources(dabal PRIVATE "source/core/Process_X64_GCC.cpp")
elseif(DABAL_ARQ STREQUAL "DABAL_X86_CLANG" OR DABAL_ARQ STREQUAL "DABAL_X86_GCC")
target_sources(dabal PRIVATE "source/core/Process_X86_GCC.cpp")
else()  #estas condiciones no son correctas, me falta estructurar bien segun plataforma, etc..No tengo define todavái para MSVC
    target_sources(dabal PRIVATE "source/core/Process_X86_MSVC.cpp")
endif()

target_include_directories(dabal PUBLIC "include")
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(dabal PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
