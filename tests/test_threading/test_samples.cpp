#include "test_samples.h"
#include <tasking/ThreadRunnable.h>
#include <tasking/utilities.h>
#include <tasking/CriticalSection_mthread.h>
#include <tasking/Event_mthread.h>
using namespace tasking;
#include <functional>
#include <string>
using std::string;

void _sample1()
{
    auto th1 = ThreadRunnable::create();
    th1->fireAndForget( []()
        {
            auto th = ThreadRunnable::getCurrentThreadRunnable();
            Future<int> fut = th->execute<int>( 
                [](){
                    Process::wait(100);
                    return 10;
                }
            );
            try
            {
                auto res = waitForFutureMThread(fut);
                text::info("Result = {}",res.value());
            }catch( core::WaitException& e)
            {
                text::error("Error waiting. Reason = {}",e.what());
            }
            catch(...)
            {
                text::error("Error waiting. Unknown Reason");
            }

        }
        ,0,Runnable::killFalse
    );
    
}

void _sample2()
{
    auto th1 = ThreadRunnable::create();
    Future<string> result;
    th1->fireAndForget( [result]() mutable
        {
            auto th = ThreadRunnable::getCurrentThreadRunnable();
        //@todo explicar bien esto de n oasignar el future de enrtada!!!
        //de hecho..¿tendrái sentido gestionarlo de alguna forma??
            auto fut = th->execute<string>( 
                []() noexcept
                {
                    Process::wait(100);
                    return "Hello";
                }
            );
            try
            {
                auto res = waitForFutureMThread(fut,20);
                result.setValue(res.value());
                text::info("Result = {}",res.value());
            }catch( core::WaitException& e)
            {
                result.setValue(std::current_exception());;
                text::error("Error waiting. Reason = {}",e.what());
            }
            catch(...)
            {
                result.setValue(std::current_exception());;
                text::error("Error waiting. Unknown Reason");
            }
        }
        ,0,Runnable::killFalse
    );
    //pause current thread waiting for future ready
    try
    {
        auto res = core::waitForFutureThread(result);
        text::info("Result after waiting in currrent thread= {}",res.value());
    }catch( core::WaitException& e)
    {
        text::error("Error waiting in currrent thread. Reason = {}",e.what());
    }
    catch(...)
    {
        text::error("Error waiting in currrent thread. Unknown Reason");
    }    
    
}
void _sample3()
{
    Event_mthread<::tasking::EventNoMTThreadSafePolicy> event;  
    CriticalSection_mthread<false> cs;
    
    auto th1 = ThreadRunnable::create(); //order is important here to respect reverse order of destruction
    th1->fireAndForget( [&cs,&event]() mutable
        {
            ::text::info("Task1 Waiting before entering critical section");
            ::tasking::Process::wait(1000);
            Lock_mthread lck(cs);
            ::text::info("Task1 Entering critical section and set event");
            event.set();    
        }
        ,0,Runnable::killFalse
    );
    th1->fireAndForget([&cs]() mutable
        {
            Lock_mthread lck(cs);
            ::text::info("Task2 Waiting inside critical section");
            ::tasking::Process::wait(5000);
            ::text::info("Task2 is going to release critical section");
        }
        ,0,Runnable::killFalse
    );
    th1->fireAndForget( [&event]() mutable
        {
            text::info("Task3 Waiting for event..");
            event.wait();
            text::info("Task3 Event was signaled!");
        }
        ,0,Runnable::killFalse
    );
}

void test_threading::samples()
{
    //_sample1();
    //_sample2();
    _sample3();
}