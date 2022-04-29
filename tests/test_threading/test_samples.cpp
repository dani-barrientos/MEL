#include "test_samples.h"
#include <tasking/ThreadRunnable.h>
#include <tasking/utilities.h>
#include <tasking/CriticalSection_mthread.h>
#include <tasking/Event_mthread.h>
using namespace mel::tasking;
#include <functional>
#include <string>
using std::string;
#include <string.h>
#include <tasking/synchronization_macros.h>
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
            auto fut = th->execute<string>( 
                []() noexcept
                {
                    Process::wait(100);
                    return "Hello";
                }
            );
            try
            {
                auto res = waitForFutureMThread(fut,20); //will wait only for 20 msecs
                result.setValue(res.value());
                mel::text::info("Result = {}",res.value());
            }catch( mel::core::WaitException& e)
            {
                result.setValue(std::current_exception());;
                mel::text::error("Error waiting. Code {}, Reason = {}",(int)e.getCode(),e.what());
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
void _sampleCustomRunnable()
{
    class MyRunnable : public Runnable
    {
        public:
        //create a Runnable with default options
            MyRunnable():Runnable( Runnable::RunnableCreationOptions())
            {

            }
            //needed because Runnable::processTasks is protected
            void processTasks()
            {
                Runnable::processTasks();
            }
    };
    MyRunnable r;
    r.post([](RUNNABLE_TASK_PARAMS)
    {
        mel::text::info("Task 1");
        Process::wait(1000);
        mel::text::info("Task 1 - end");
        return  ::mel::tasking::EGenericProcessResult::CONTINUE;
    });
    r.fireAndForget([]
    {
        mel::text::info("Task 2");
        Process::wait(1000);
        mel::text::info("Task 2 - end");
    });
    while(true)
    {    
        r.processTasks();
    }
}
void _sampleCustomRunnableBad()
{
    class MyRunnable : public Runnable
    {
        public:
        //create a Runnable with default options
            MyRunnable():Runnable( Runnable::RunnableCreationOptions())
            {

            }
            //needed because Runnable::processTasks is protected
            void processTasks()
            {
                Runnable::processTasks();
            }
    };
    MyRunnable r;
    r.post([](RUNNABLE_TASK_PARAMS)
    {
        mel::text::info("Task 1");
        Process::wait(1000);
        mel::text::info("Task 1 - end");
        return  ::mel::tasking::EGenericProcessResult::CONTINUE;
    });
    r.fireAndForget([]
    {
        mel::text::info("Task 2");
        Process::wait(1000);
        mel::text::info("Task 2 - end");
    });
    auto f1 = [&r]
    {
        r.processTasks();
    };
    auto f2 = [&r]
    {
        int a[100];
        r.processTasks();
    };
    while(true)
    {           
        f1();
        f2();
    }
}

std::shared_ptr<Runnable> sRunnable;

/**
 * @brief Test synchronized callables with templates
 * 
 * @details Comparing with macros.
 * Disadvantages:
 *  - more memory footprint: because use internally en function, it has a fixed size more than a simple function pointer
 *  - construction is more complex: you need to constructor the member passing a callable adn a runnable
 * Advantages:
 *  - No need to set runnable in header, which can be disturbing when using macros
 *  - more understable: syntax more clean and (I think) more significant
 */
/*
//PRUEBAS
//TODO EL RUNNABLE NO PUEDE PASARSE POR CONSTRUCTOR, YA QUE CAMBIARÁ. POSIBILIDADES PARA PASAR EL RUNNABLE: 
template <class TRet, class ...TArgs> class SyncCallable
{
    public:
    problema gordo, necesitaría algo que haga referencia a un runnable, no guardar el runnable en sí
        template <class F> 
        SyncCallable(F&& callable,Runnable* runnable):mRunnable(runnable),mCallable(std::forward<F>(callable)){}
        Future<TRet> operator()(TArgs&&...args)
        {
            //return mCallable(std::forward<TArgs>(args)...);
            return mRunnable->execute<TRet>( std::bind(mCallable,std::forward<TArgs>(args)...)); \
        }
    private:
        Runnable* mRunnable;
        std::function< TRet(TArgs...)> mCallable;        
};
template <class TRet, class ...TArgs> class SyncCallable<TRet (TArgs...)>
{

};
*/
string f0_sync(int a,float b)
{
    return "hola";
}
SYNCHRONIZED_STATIC( f1,int,(int),,sRunnable ) ; 


static int f1_sync(int v) 
{
    throw std::runtime_error("ERROR in f1!!!!");
    return v + 5;
}

class MyClass
{
    public:
        //MyClass();
        SYNCHRONIZED_METHOD( f2,string,(int,float),noexcept,sRunnable) ;   
        //SyncCallable<string, int,float> f3; 
    private:
        string f3_sync(int,float);
};
using namespace std::placeholders;
/*MyClass::MyClass():f3(std::bind(&MyClass::f3_sync,this,_1,_2),sRunnable.get())
{

}*/
string MyClass::f2_sync(int v1,float v2) noexcept
{
    return "f2_sync "+std::to_string(v1)+" "+std::to_string(v2);
}
// string MyClass::f3_sync(int v1,float v2) 
// {
//     return "f3_sync "+std::to_string(v1)+" "+std::to_string(v2);
// }
void _sampleSyncMacros()
{
    sRunnable = ThreadRunnable::create(true);
    {
        mel::text::info("Calling static function f1 syncronized with a Runnable");
        auto r = f1(6);
        try
        {
            auto res = mel::core::waitForFutureThread(r);
            mel::text::info("Result = {}",res.value());
        }
        catch(std::exception& e)
        {
            mel::text::error("Exception!!. {}",e.what());
        }
       
    }
    {
        MyClass obj;
        mel::text::info("Calling static function f2 syncronized with a Runnable");
        auto r2 = obj.f2(6,9.1f);
        //auto r2 = obj.f3(6,9.1f);
        //mel::text::info("Sizeof f3 = {}",sizeof(MyClass::f3));
        try
        {
            auto res = mel::core::waitForFutureThread(r2);
            mel::text::info("Result = {}",res.value());
        }
        catch(std::exception& e)
        {
            mel::text::error("Exception!!. {}",e.what());
        }       
    }  
}
void test_threading::samples()
{
    //_sample1();
    _sample2();
    //_sample3();
 //   _sampleTasking1();
    //_sampleTasking2();
    //_sampleTasking3();
    //_sampleTasking4();
    //_sampleTasking4_ok();
    // _sampleTasking5();
    //_sampleTasking6();
 //   _sampleTasking7();
 //   _sampleLimitation();
    //_sampleCustomRunnable();
    //_sampleCustomRunnableBad();
    //_sampleSyncMacros();
}