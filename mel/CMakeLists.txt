# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_definitions(MEL_EXPORTS)
if ( MSVC )
    add_compile_options(/permissive- /Zc:lambda)
endif()
#cambiado SHARED a STATIC por temas de emscripten
add_library(mel STATIC  "source/tasking/CriticalSection_mthread.cpp"
"source/tasking/Event_mthread.cpp" "source/tasking/Process.cpp"
"source/tasking/GenericProcess.cpp"  "source/tasking/ProcessScheduler.cpp" "source/tasking/Runnable.cpp" "source/tasking/ThreadRunnable.cpp" 
"source/core/Timer.cpp" "source/core/Type.cpp" "source/core/Thread.cpp"  "source/core/TLS.cpp" 
"source/core/Event_win32.cpp" "source/core/Event_posix.cpp" "include/tasking/CriticalSection_mthread.h"  "include/MelLibType.h"
 "source/parallelism/Barrier.cpp" "source/tasking/utilities.cpp" "source/text/logger.cpp"
"source/parallelism/ThreadPool.cpp" "include/parallelism/For.h" "include/execution/Executor.h"  "include/execution/RunnableExecutor.h"
)
#set_target_properties(mel PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

if(MEL_OS STREQUAL "MEL_WINDOWS")
    #target_compile_options(mel PUBLIC "/permissive-")
    if(MEL_ARQ STREQUAL "MEL_X86_MSVC")
        target_sources(mel PRIVATE "source/tasking/Process_X86_MSVC.cpp")
    else()
        enable_language(ASM_MASM)
        SET(ASM_DIALECT "-MASM")
        #set(CMAKE_ASM_NASM_FLAGS "-f bin")
        SET(ASM_OPTIONS "-x assembler-with-cpp")
        #SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )
        target_sources(mel PRIVATE "source/tasking/Process_X64_MSVC.cpp")
        target_sources(mel PRIVATE "source/tasking/Process_X64_MSVC_asm.asm")
    endif()
elseif(MEL_OS STREQUAL "MEL_MACOSX")
    target_sources(mel PRIVATE "source/tasking/Process_X64_MAC.cpp")
elseif(MEL_OS STREQUAL "MEL_LINUX" AND (MEL_ARQ STREQUAL "MEL_X64_CLANG" OR MEL_ARQ STREQUAL "MEL_X64_GCC") )
    target_sources(mel PRIVATE "source/tasking/Process_X64_GCC.cpp")
elseif(MEL_OS STREQUAL "MEL_ANDROID")
    target_sources(mel PRIVATE "source/tasking/Process_ARM_Android.cpp")
elseif(MEL_OS STREQUAL "MEL_EMSCRIPTEN")
        message("Building mel for emscripten")
        target_sources(mel PRIVATE "source/tasking/Process_emsdk.cpp")
elseif(MEL_ARQ STREQUAL "MEL_X86_CLANG" OR MEL_ARQ STREQUAL "MEL_X86_GCC")
    message("Building mel 32 bits")
   # target_sources(mel PRIVATE "source/tasking/Process_X86_GCC.cpp")
    #set_target_properties(mel PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    target_compile_options(mel PUBLIC "-m32")
    target_link_options(mel PUBLIC "-m32")
endif()


target_include_directories(mel PUBLIC "include")
if (USE_SPDLOG)
    message ("MEL is using spdlog")
    find_package(spdlog CONFIG REQUIRED)
    target_link_libraries(mel spdlog::spdlog spdlog::spdlog_header_only)
else()
    message ("MEL is NOT using spdlog")
endif()



#parece que no hay version 32 bits de spdlog meter un define -> hacer funcion cmake para devovler true si arquitectura 32 bits
#tratar de unificar con el find_packe
