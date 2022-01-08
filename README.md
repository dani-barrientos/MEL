# dabal


# notes
Preprocessor macros for arquitecture, OS, and API
There are 3 diferent macros that guide compilation based on target platform.
- DABAL_OS : target operating system. Available OS are: **DABAL_WINDOWS**, **DABAL_LINUX**,**DABAL_MACOS**,**DABAL_IOS**,**DABAL_ANDROID**
- DABAL_API: API to use. It can be **DABAL_POSIX**. Else, API based on platform is used.
- DABAL_ARQ: CPU arquitecture and compiler, fo assembler syntax resolution

# vcpkg
 vcpkg need to be installed. Follow instructions, among many other sites, https://vcpkg.io/en/getting-started.html. An environment variable VCPKG_ROOT pointing to vcpkg root
 need to be set. @todo algo no entiendo, esta variable parece que la usa el propio vcpkg si está definida, de forma que si la tengo en las variables de entorno, al menos en mi instalación kubuntu, da error al ejecutarlo porque usa ese varaible. por ejemplo, si se hace ./vcpkg install <lo que sea> da error de que "No such file or directory". Si se ejecuta con --debug se ven más logs y se ve que concatena el path actual con el VCPKG_ROOT.
 Por tanto, tengo que "undefinir" esa variable de entorno al ir a instalar un paquete o usarlo..qué raro
 Mirar https://github.com/microsoft/vcpkg/blob/master/docs/users/config-environment.md
 Resulta que si defino esa variable con export desde terminal, va bien. El problema es definiendola en el /etc/environment
 # testing
 execute ctest in out/build/[required configuration]/tests
 