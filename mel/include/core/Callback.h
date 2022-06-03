#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos
 * <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <core/FunctionCallbackInterface.h>
#include <core/FunctorCallbackInterface.h>
#include <mpl/Conversion.h>
#include <mpl/TypeTraits.h>
#include <stdexcept>
#undef INCLUDE_PATH
#define INCLUDE_PATH <core/Callback_Impl.h>
#include <mpl/VarArgs.h>
