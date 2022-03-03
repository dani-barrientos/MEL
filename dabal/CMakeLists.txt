# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(DABAL_EXPORTS)
add_library(dabal SHARED  "source/tasking/CriticalSection_mthread.cpp"
"source/tasking/Event_mthread.cpp" "source/tasking/Process.cpp"
"source/tasking/GenericProcess.cpp"  "source/tasking/ProcessScheduler.cpp" "source/tasking/Runnable.cpp" "source/core/Timer.cpp"
 "source/core/Type.cpp" "source/core/Thread.cpp" "source/core/ThreadRunnable.cpp" "source/core/TLS.cpp" 
"source/core/Event_win32.cpp" "source/core/Event_posix.cpp" "include/tasking/CriticalSection_mthread.h"  "include/DabalLibType.h"
 "source/parallelism/Barrier.cpp" "source/tasking/utilities.cpp" "source/text/logger.cpp"
"source/parallelism/ThreadPool.cpp" "include/parallelism/For.h" "include/execution/Executor.h" "include/execution/Continuation.h" "include/execution/RunnableExecutor.h"
)
#set_target_properties(dabal PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

if(DABAL_OS STREQUAL "DABAL_WINDOWS")
    if(DABAL_ARQ STREQUAL "DABAL_X86_MSVC")
        target_sources(dabal PRIVATE "source/tasking/Process_X86_MSVC.cpp")
    else()
        enable_language(ASM_MASM)
        SET(ASM_DIALECT "-MASM")
        #set(CMAKE_ASM_NASM_FLAGS "-f bin")
        SET(ASM_OPTIONS "-x assembler-with-cpp")
        #SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )
        target_sources(dabal PRIVATE "source/tasking/Process_X64_MSVC.cpp")
        target_sources(dabal PRIVATE "source/tasking/Process_X64_MSVC_asm.asm")
    endif()
elseif(DABAL_OS STREQUAL "DABAL_MACOSX")
    target_sources(dabal PRIVATE "source/tasking/Process_X64_MAC.cpp")
elseif(DABAL_OS STREQUAL "DABAL_LINUX" AND (DABAL_ARQ STREQUAL "DABAL_X64_CLANG" OR DABAL_ARQ STREQUAL "DABAL_X64_GCC") )
    target_sources(dabal PRIVATE "source/tasking/Process_X64_GCC.cpp")
elseif(DABAL_OS STREQUAL "DABAL_ANDROID")
    target_sources(dabal PRIVATE "source/tasking/Process_ARM_Android.cpp")
elseif(DABAL_ARQ STREQUAL "DABAL_X86_CLANG" OR DABAL_ARQ STREQUAL "DABAL_X86_GCC")
    message("Building dabal 32 bits")
   # target_sources(dabal PRIVATE "source/tasking/Process_X86_GCC.cpp")
    #set_target_properties(dabal PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    target_compile_options(dabal PUBLIC "-m32")
    target_link_options(dabal PUBLIC "-m32")
endif()


target_include_directories(dabal PUBLIC "include")
if (USE_SPDLOG)
    message ("Dabal is using spdlog")
    find_package(spdlog CONFIG REQUIRED)
    target_link_libraries(dabal spdlog::spdlog spdlog::spdlog_header_only)
else()
    message ("Dabal is NOT using spdlog")
endif()



#parece que no hay version 32 bits de spdlog meter un define -> hacer funcion cmake para devovler true si arquitectura 32 bits
#tratar de unificar con el find_packe
