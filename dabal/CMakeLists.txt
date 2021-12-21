# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(DABAL_EXPORTS)
add_library(dabal SHARED "source/core/atomics.cpp" "source/core/CriticalSection_mthread.cpp"
"source/core/CriticalSection_mthread.cpp" "source/core/Event_mthread.cpp"
 "source/core/GenericProcess.cpp" "source/core/Process.cpp" "source/core/Process_X86_MSVC.cpp" "source/core/ProcessScheduler.cpp" "source/core/Runnable.cpp" "source/core/Timer.cpp" "source/core/Type.cpp" "source/core/Thread.cpp" "source/core/GenericThread.cpp" "source/core/TLS.cpp" "source/core/Event_win32.cpp" "include/core/atomics.h" "include/core/CriticalSection_mthread.h" "source/core/Process_X86_GCC.cpp" "include/DabalLibType.h")
target_include_directories(dabal PUBLIC "include")
