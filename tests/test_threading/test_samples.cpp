#include "test_samples.h"
#include <tasking/ThreadRunnable.h>
#include <tasking/utilities.h>
using namespace tasking;
#include <functional>
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
void test_threading::samples()
{
    _sample1();
}