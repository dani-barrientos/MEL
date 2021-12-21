#pragma once
#undef INCLUDE_PATH
#include <mpl/Conversion.h>

//TODO quitar los Int2Type(false) y true para indicar constante y usar estructura propia

#define INCLUDE_PATH <mpl/MemberEncapsulateImpl.h>
#include <mpl/Int2Type.h>
using mpl::Int2Type;
#include <mpl/_If.h>
#include <mpl/equal.h>

#include <mpl/VarArgs.h>
