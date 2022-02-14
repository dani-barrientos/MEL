# dabal


# notes
Preprocessor macros for arquitecture, OS, and API
There are 3 diferent macros that guide compilation based on target platform.
- DABAL_OS : target operating system. Available OS are: **DABAL_WINDOWS**, **DABAL_LINUX**,**DABAL_MACOSX**,**DABAL_IOS**,**DABAL_ANDROID**
- DABAL_ARQ: CPU arquitecture and compiler, fo assembler syntax resolution
- by default, installing gcc o clang in a 64bit platform will only install 64 bit libraries. You need to installa all, using for example target_compile_options
- CMAKE_BUILD_TYPE hasn't any effect with Visual Studio Generator. It has to be done in a "build preset" with the "configuration" parameter
- Cosas en Mac:
    - si se añade un preset y se seleccion, no funciona, y no da info. Parece ser que el problema es que falta el "generator". Parece que tanto "Ninja" como "Unix Makefiles" vale
    - tampoco fui capaz de establcer la variable de entorno para el VCPKG_ROOT, por lo que tuve que meterla en los settings de cmaketools en la opción Cmake:environtment->pero resulta que cuando empecé a poder usar los presets ya no vale, así que definí la variable"CMAKE_TOOLCHAIN_FILE" en al sección "cacheVariables" apuntando a la ruta (en mi caso /Users/dani/vcpkg/scripts/buildsystems/vcpkg.cmake)
# vcpkg
 vcpkg need to be installed. Follow instructions, among many other sites, https://vcpkg.io/en/getting-started.html. An environment variable VCPKG_ROOT pointing to vcpkg root
 need to be set. @todo algo no entiendo, esta variable parece que la usa el propio vcpkg si está definida, de forma que si la tengo en las variables de entorno, al menos en mi instalación kubuntu, da error al ejecutarlo porque usa ese varaible. por ejemplo, si se hace ./vcpkg install <lo que sea> da error de que "No such file or directory". Si se ejecuta con --debug se ven más logs y se ve que concatena el path actual con el VCPKG_ROOT.
 Por tanto, tengo que "undefinir" esa variable de entorno al ir a instalar un paquete o usarlo..qué raro
 Mirar https://github.com/microsoft/vcpkg/blob/master/docs/users/config-environment.md
 Resulta que si defino esa variable con export desde terminal, va bien. El problema es definiendola en el /etc/environment
 Definiendola en el $HOME/.profile con export, funciona todo ya bien
 # testing
 execute ctest in out/build/[required configuration]/tests
 