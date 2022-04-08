#pragma once
/*
 * SPDX-FileCopyrightText: 2017,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <functional>

/**
* CallbackInterface implementation for C++ function objects and lambdas
*/
#include <core/CallbackInterface.h>
#include <mpl/equal.h>

#undef INCLUDE_PATH
#define INCLUDE_PATH <core/FunctionCallbackInterface_Impl.h>
#include <mpl/VarArgs.h>
