#pragma once
#include <MelLibType.h>
#include <mutex>

namespace mel
{
    namespace core
    {
        /**
         * policy for thread-safe access to singleton
         * @todo review, no complete
         */
        template <class T> class Singleton_Multithread_Policy
        {
          private:
            template <class U> class Locker
            {
              public:
                Locker() { mCs.lock(); }
                ~Locker() { mCs.unlock(); }

              private:
                static std::mutex mCs;
            };

          public:
            typedef T VolatileType;
            typedef Locker<T> Lock;
        };
        template <class T>
        template <class U>
        std::mutex Singleton_Multithread_Policy<T>::Locker<U>::mCs;
    } // namespace core
} // namespace mel