#pragma once

namespace mel
{
    namespace mpl
    {
        /**
        * check if both types are same
        **/
        template <class T1, class T2>
    struct isSame
    {
        enum{ result = false};
    };
    template <class T>
    struct isSame<T,T>
    {
        enum{ result = true};
    };

    }
}