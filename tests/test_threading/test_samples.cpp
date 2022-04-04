#include "test_samples.h"
#include <tasking/ThreadRunnable.h>
#include <tasking/utilities.h>
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
        @todo explicar bien esto de n oasignar el future de enrtada!!!
        de hecho..¿tendrái sentido gestionarlo de alguna forma??
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

void test_threading::samples()
{
    //_sample1();
    _sample2();
}