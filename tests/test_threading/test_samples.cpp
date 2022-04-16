#include "test_samples.h"
#include <tasking/ThreadRunnable.h>
#include <tasking/utilities.h>
#include <tasking/CriticalSection_mthread.h>
#include <tasking/Event_mthread.h>
using namespace mel::tasking;
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
                mel::text::info("Result = {}",res.value());
            }catch( mel::core::WaitException& e)
            {
                mel::text::error("Error waiting. Reason = {}",e.what());
            }
            catch(...)
            {
                mel::text::error("Error waiting. Unknown Reason");
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
                mel::text::info("Result = {}",res.value());
            }catch( mel::core::WaitException& e)
            {
                result.setValue(std::current_exception());;
                mel::text::error("Error waiting. Reason = {}",e.what());
            }
            catch(...)
            {
                result.setValue(std::current_exception());;
                mel::text::error("Error waiting. Unknown Reason");
            }
        }
        ,0,Runnable::killFalse
    );
    //pause current thread waiting for future ready
    try
    {
        auto res = mel::core::waitForFutureThread(result);
        mel::text::info("Result after waiting in currrent thread= {}",res.value());
    }catch( mel::core::WaitException& e)
    {
        mel::text::error("Error waiting in currrent thread. Reason = {}",e.what());
    }
    catch(...)
    {
        mel::text::error("Error waiting in currrent thread. Unknown Reason");
    }    
    
}
void _sample3()
{
    Event_mthread<::mel::tasking::EventNoMTThreadSafePolicy> event;  
    CriticalSection_mthread<false> cs;
    
    auto th1 = ThreadRunnable::create(); //order is important here to respect reverse order of destruction
    th1->fireAndForget( [&cs,&event]() mutable
        {
            ::mel::text::info("Task1 Waiting before entering critical section");
            ::mel::tasking::Process::wait(1000);
            Lock_mthread lck(cs);
            ::mel::text::info("Task1 Entering critical section and set event");
            event.set();    
        }
        ,0,Runnable::killFalse
    );
    th1->fireAndForget([&cs]() mutable
        {
            Lock_mthread lck(cs);
            ::mel::text::info("Task2 Waiting inside critical section");
            ::mel::tasking::Process::wait(5000);
            ::mel::text::info("Task2 is going to release critical section");
        }
        ,0,Runnable::killFalse
    );
    th1->fireAndForget( [&event]() mutable
        {
            mel::text::info("Task3 Waiting for event..");
            event.wait();
            mel::text::info("Task3 Event was signaled!");
        }
        ,0,Runnable::killFalse
    );
}
void _sampleTasking1()
{
    auto th1 = ThreadRunnable::create(true);
    constexpr unsigned int period = 450; //how often, msecs, the tasks is executed
    auto& killPolicy = Runnable::killTrue;
    std::shared_ptr<mel::tasking::Process> task = th1->post([](uint64_t msecs,Process* p)
		{
            mel::text::info( "execute task. Time={}",msecs);
			//return ::mel::tasking::EGenericProcessResult::KILL;
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},killPolicy,period);
    ::mel::core::Thread::sleep(5000);
}
void _sampleTasking2()
{
    auto th1 = ThreadRunnable::create(true);
    auto& killPolicy = Runnable::killTrue;
    auto t1 = th1->fireAndForget([]
		{
            mel::text::info( "Task 1, Step1" );
            ::mel::tasking::Process::switchProcess(true);
            mel::text::info( "Task 1, Step2" );
            ::mel::tasking::Process::wait(2000);
            mel::text::info( "Task 1, Step3. Going to sleep.." );
            ::mel::tasking::Process::sleep();
            mel::text::info( "Task 1, Awaken!!" );
            

		},0,Runnable::killFalse);
    th1->fireAndForget([t1]
		{
            mel::text::info( "Task 2, Step1" );
            ::mel::tasking::Process::wait(2500);
            mel::text::info( "Task 2, Step2" );
            ::mel::tasking::Process::wait(5000);
            mel::text::info( "Task 2, Going to wake up task1" );
            t1->wakeUp();
		},0,Runnable::killFalse);
}
//test emscripten
void _testEMS()
{
    auto th1 = ThreadRunnable::create(true);
    auto& killPolicy = Runnable::killTrue;
    auto t1 = th1->post([](RUNNABLE_TASK_PARAMS)
		{
            mel::text::info( "Task 1, Step1" );            
            ::mel::tasking::Process::switchProcess(true);            
            mel::text::info( "Task 1, Step2" );
        /*    ::mel::tasking::Process::wait(2000);
            mel::text::info( "Task 1, Step3. Going to sleep.." );
            ::mel::tasking::Process::sleep();
            mel::text::info( "Task 1, Awaken!!" );
            */
           return mel::tasking::EGenericProcessResult::KILL;

		},Runnable::killFalse,2000);
    th1->post([t1](RUNNABLE_TASK_PARAMS)
		{
            mel::text::info( "Task 2, Step1" );
          /*  ::mel::tasking::Process::wait(2500);
            mel::text::info( "Task 2, Step2" );
            ::mel::tasking::Process::wait(5000);
            mel::text::info( "Task 2, Going to wake up task1" );
            t1->wakeUp();*/
            return mel::tasking::EGenericProcessResult::CONTINUE;
		},Runnable::killFalse,4000);
    Thread::sleep(10000); //sin espera, el microhilismo peta en JS
}
void test_threading::samples()
{
    mel::text::set_level(mel::text::level::ELevel::debug);
    //_sample1();
    //_sample2();
    //_sample3();
    //_sampleTasking1();
    //_sampleTasking2();
    _testEMS();
}