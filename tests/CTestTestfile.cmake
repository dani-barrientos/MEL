# CMake generated Testfile for 
# Source directory: /home/dani/trabajo/proyectos/dabal/tests
# Build directory: /home/dani/trabajo/proyectos/dabal/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(Run1 "/home/dani/trabajo/proyectos/dabal/tests/tests" "-t" "callbacks")
set_tests_properties(Run1 PROPERTIES  _BACKTRACE_TRIPLES "/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;30;add_test;/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;0;")
add_test(Run2 "/home/dani/trabajo/proyectos/dabal/tests/tests" "-t" "threading" "-n" "0")
set_tests_properties(Run2 PROPERTIES  _BACKTRACE_TRIPLES "/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;31;add_test;/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;0;")
add_test(Run3 "/home/dani/trabajo/proyectos/dabal/tests/tests" "-t" "threading" "-n" "1")
set_tests_properties(Run3 PROPERTIES  _BACKTRACE_TRIPLES "/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;32;add_test;/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;0;")
add_test(Run4 "/home/dani/trabajo/proyectos/dabal/tests/tests" "-t" "threading" "-n" "2")
set_tests_properties(Run4 PROPERTIES  _BACKTRACE_TRIPLES "/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;33;add_test;/home/dani/trabajo/proyectos/dabal/tests/CMakeLists.txt;0;")
