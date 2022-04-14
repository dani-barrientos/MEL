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
void _sampleTasking3()
{
    auto th1 = ThreadRunnable::create(true);
    constexpr unsigned int period = 450; //how often, msecs, the tasks is executed
    auto& killPolicy = Runnable::killFalse;
    std::shared_ptr<mel::tasking::Process> task = th1->post([](uint64_t msecs,Process* p)
		{            
            mel::text::info( "First Wait" );
            Process::wait(1000);
            mel::text::info( "Context switch");
			Process::switchProcess(true);
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},killPolicy,period);
    Thread::sleep(5000);
    mel::text::info( "Sleep done");
}

void _sampleTasking4()
{
    struct MyString
    {
        MyString()
        {
            data = new char[10];
            strcpy(data,"Hello!");
        }
        ~MyString()
        {
            mel::text::info("Destroying MyString");
            delete []data;
        }
        char* data;
    };
    auto th1 = ThreadRunnable::create(true);
    constexpr unsigned int period = 450; //how often, msecs, the tasks is executed
    auto& killPolicy = Runnable::killTrue;
    MyString str;
    std::shared_ptr<mel::tasking::Process> task = th1->post([&str](uint64_t msecs,Process* p)
		{            
            mel::text::info( "Before Wait. {}",str.data );
            Process::wait(10000);
            mel::text::info( "After Wait. {}",str.data );            
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},killPolicy,period);
    Thread::sleep(5000);
    mel::text::info( "Sleep done");
}
void _sampleTasking4_ok()
{
    struct MyString
    {
        MyString()
        {
            data = new char[10];
            strcpy(data,"Hello!");
        }
        ~MyString()
        {
            mel::text::info("Destroying MyString");
            delete []data;
        }
        char* data;
    };
    auto th1 = ThreadRunnable::create(true);
    constexpr unsigned int period = 450; //how often, msecs, the tasks is executed
    auto& killPolicy = Runnable::killTrue;
    MyString str;
    std::shared_ptr<mel::tasking::Process> task = th1->post([&str](uint64_t msecs,Process* p)
		{            
            mel::text::info( "Before Wait. {}",str.data );
            if ( Process::wait(10000) == Process::ESwitchResult::ESWITCH_OK)
                mel::text::info( "After Wait. {}",str.data );            
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},killPolicy,period);
    Thread::sleep(5000);
    mel::text::info( "Sleep done");
}
//Process Specialization
void _sampleTasking5()
{
    struct MyProcess : public Process
    {
        void onInit(uint64_t msecs) override
        {
            mel::text::info("MyProcess::oninit");
            count = 0;
        };
        void onUpdate(uint64_t msecs) override
        {
            mel::text::info("MyProcess::onUpdate");
            ++count;
        }
        virtual bool onKill() override
        {
            return count == 10;
        }
        private:
            int count;

    };
    auto th1 = ThreadRunnable::create(true);
    constexpr unsigned int period = 450; //how often, msecs, the tasks is executed
    auto& killPolicy = Runnable::killFalse;
    auto task = std::make_shared<MyProcess>();
    task->setPeriod(1000);
    th1->postTask(task);
}
//changing default runnable factory
void _sampleTasking6()
{
    class CustomProcessType : public GenericProcess
    {
        public:	
        CustomProcessType()
        {
            ::mel::text::info("CustomProcessType constructor");
        }
    };
    //custom factory to replace Runnable default factory
    class CustomProcessFactory : public mel::tasking::ProcessFactory
    {
        public:
            GenericProcess* onCreate(Runnable* owner) const override
            {
                return new CustomProcessType();
            }
    };
    auto th1 = ThreadRunnable::create(true);
    th1->setDefaultFactory( new CustomProcessFactory );
    th1->fireAndForget( []
    {
        mel::text::info("execute");
    },0,Runnable::killTrue);
}
//custom factory 
class CustomProcessFactory
{
    public:
        GenericProcess* create(Runnable* owner) const
        {
            return new GenericProcess();
        }
};
//setting factory in post
struct MyAllocator
{
    static GenericProcess* allocate(Runnable* _this)
    {
        return factory.create(_this);
    }
    static CustomProcessFactory factory;
};
CustomProcessFactory MyAllocator::factory;
void _sampleTasking7()
{
    //custom allocator which will use out CustomProcessFactory, not inheriting from ProcessFactory, so best performance
    auto th1 = ThreadRunnable::create(true);
    th1->fireAndForget<MyAllocator>( []
    {
        mel::text::info("execute");
    },0,Runnable::killFalse);
}
//sample showing limitation on local variables
void _sampleLimitation()
{
    auto th1 = ThreadRunnable::create(true);
    th1->fireAndForget([th = th1.get()]
		{
            int a = 5;
            mel::text::info("Set 'a' = {}",a);
            th->fireAndForget([&a]
                {
                    mel::text::info("'a' = {}",a);
                    a = 10;
                }
            ,0,Runnable::killFalse);
            Process::wait(5000);
            mel::text::info("At end 'a' = {}",a);
		},0,Runnable::killFalse);    
}

void test_threading::samples()
{
    //_sample1();
    //_sample2();
    //_sample3();
 //   _sampleTasking1();
    //_sampleTasking2();
    //_sampleTasking3();
    //_sampleTasking4();
    //_sampleTasking4_ok();
    // _sampleTasking5();
    //_sampleTasking6();
 //   _sampleTasking7();
    _sampleLimitation();
}