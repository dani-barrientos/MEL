#pragma once
#include <functional>

/**
* CallbackInterface implementation for C++ function objects and lambdas
*/
#include <core/CallbackInterface.h>
#include <mpl/equal.h>

#undef INCLUDE_PATH
#define INCLUDE_PATH <core/FunctionCallbackInterface_Impl.h>
#include <mpl/VarArgs.h>
