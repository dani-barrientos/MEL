#pragma once
/*
 * SPDX-FileCopyrightText: 2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
namespace mel
{
    namespace execution
    {
        struct VoidType{};
        //helper type to get a VoidType when template parameter is void
        template <class T> struct WrapperType
        {
            using type = T;         
        };
        template <> struct WrapperType<void>
        {
            using type = VoidType;
        };
    }
}