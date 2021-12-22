# dabal


# notes
Preprocessor macros for arquitecture, OS, and API
There are 3 diferent macros that guide compilation based on target platform.
- DABAL_OS : target operating system. Available OS are: **DABAL_WINDOWS**, **DABAL_LINUX**,**DABAL_MACOS**,**DABAL_IOS**,**DABAL_ANDROID**
- DABAL_API: API to use. It can be **DABAL_POSIX**. Else, API based onj platform is used.
- DABAL_ARQ: CPU arquitecture and compiler, for assembler syntax resolution