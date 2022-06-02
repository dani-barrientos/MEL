#include "test.h"
#include "test_samples.h"
using namespace test_threading;
#include <iostream>
#include <tasking/ThreadRunnable.h>
using mel::tasking::ThreadRunnable;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <text/logger.h>

#include <CommandLine.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/Process.h>
using mel::tasking::Process;
#include "future_tests.h"
#include <tasking/utilities.h>
#include <array>

const std::string TestThreading::TEST_NAME = "threading";

class CustomProcessType : public GenericProcess
{
	public:

	//testing custom operator new. Now do the default but could have my own pool
	static void* operator new( size_t s,Runnable* owner )
	{
		return ::operator new (s);
	}

	static void operator delete( void* ptr )
	{
		::operator delete(ptr);
	}
	CustomProcessType()
	{
		//spdlog::debug("CustomProcessType constructor");
	}
	~CustomProcessType()
	{
		//spdlog::debug("CustomProcessType destructor");
	}
	private:
	int tema;	
};
//custom factory to replace Runnable default factory
class CustomProcessFactory : public mel::tasking::ProcessFactory
{
	public:
		GenericProcess* onCreate(Runnable* owner) const override
		{
			return new (owner)CustomProcessType();
		}
};
//custom factory not inheriting from ProcessFactory
class CustomProcessFactory2
{
	public:
		GenericProcess* create(Runnable* owner) const
		{
			return new (owner)CustomProcessType();
		}
};
static CustomProcessFactory sMyfactory;
static CustomProcessFactory2 sMyfactory2;
//custom allocator for CustomProcessType
struct MyAllocator
{
	static CustomProcessType* allocate(Runnable* _this)
	{
		return static_cast<CustomProcessType*>(sMyfactory2.create(_this));
	}
};

class MyProcess : public Process
{
	public:
		MyProcess(int& var):Process(),mVar(var){}
	private:
		void onUpdate(uint64_t) override
		{
			int aux = mVar;
			++mVar;
			mel::text::debug("MyProcess");
			mel::tasking::Process::wait(70050);
			mVar = aux;
		}	
		int& mVar;

};
class MyTask
{
	public:
	MyTask(Process* target,int& var):mTarget(target),mVar(var){}
	mel::tasking::EGenericProcessResult operator()(uint64_t,Process*)
	{
		mel::text::debug("MyTask");
		++mVar;
		if ( mTarget )
			mTarget->pause();
		mel::tasking::Process::wait(213);
		if ( mTarget )
			mTarget->wakeUp();
		return ::mel::tasking::EGenericProcessResult::CONTINUE;
	}
	private:
	Process* mTarget;
	int& mVar;
};
mel::tasking::EGenericProcessResult staticFuncTask(RUNNABLE_TASK_PARAMS,int& var)
{
	++var;
	mel::text::debug("staticFuncTask");
	return ::mel::tasking::EGenericProcessResult::CONTINUE;
}
static Timer sTimer;
/**
 * objetivo: muchas tareas concurrentes pero que al final un resultado no debe ser modificado, por lo que el test
 * se pasará si el valor sigue manteniendose
 */
uint64_t constexpr TIME_MARGIN = 10;
void CHECK_TIME(uint64_t t0, uint64_t t1, std::string text )
{
	auto elapsed = std::abs((int64_t)t1-(int64_t)t0);
	if ( elapsed > TIME_MARGIN)
		mel::text::warn("Margin time overcome {}. Info: {}",elapsed,text);
}
//@note para pruebas lock free
std::atomic<int> sCount(0);
//test for debuggin stuff
static int _testsDebug(tests::BaseTest* test)
{	
	{
		auto th1 = ThreadRunnable::create();
		th1->fireAndForget( []() noexcept
        {
            auto th = ThreadRunnable::getCurrentThreadRunnable();
            auto fut = th->execute<int>( 
                []()
                {
					//pensar también como encajarái esto con establecer el error de otra forma
					//continuar ejemplo generando otro error distinto que no sea timeout
					//throw std::runtime_error("PRUEBAAAA");
					using namespace std::string_literals;
					throw "prueba string"s;
					mel::tasking::Process::wait(1000);
                    return 6;
                }
            );
			//fut.setError("triquimore"s);
			constexpr bool waitWithException = false;
			
			if ( waitWithException)
			{
				try
				{
					auto res = ::mel::tasking::waitForFutureMThread<::mel::core::WaitErrorAsException>(fut,500); //will wait only for 20 msecs
					mel::text::info("Result = {}",res.value());					
				}catch( mel::core::WaitException& e)
				{
					mel::text::error("Error waiting. Code {}, Reason = {}",(int)e.getCode(),e.what());
				}
				catch(std::exception& e)
				{
					mel::text::error("Error waiting for result. Reason = {}",e.what());
				}
				catch(string& msg)
				{
					mel::text::error("Error waiting for result. Msg = {}",msg);
				}
				catch(...)
				{
					mel::text::error("Error waiting. Unknown Reason");
				}
			}else
			{
				auto res = ::mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(fut,500); //will wait only for 20 msecs
				if ( res.isValid())
					mel::text::info("Result = {}",res.value());
				else
				{
					//@too el problema es que si quiero dar la posibildiad de que el código sea exception-free, esto no me vale, ¿merece la pena?
					try
					{
						std::rethrow_exception(res.error());
					}
					catch(mel::core::WaitException& e)
					{
						mel::text::error("Error waiting for result. Err code = {}, Reason = {}",(int)e.getCode(),e.what());
					}
					catch(std::exception& e)
					{
						mel::text::error("Error waiting for result. Reason = {}",e.what());
					}
					catch(string& msg)
					{
						mel::text::error("Error waiting for result. Msg = {}",msg);
					}
					catch(...)
					{
						mel::text::error("Error waiting for result. Unknown exception");
					}
					
				}				
			}
        }
        ,0,Runnable::killFalse
    );
	}
	{
		//auto th1 = ThreadRunnable::create();
		Future<int> fut1;
		Future<int> fut2;
		Future<int> fut3(fut2);
		Future<int> fut4;
		fut1.subscribeCallback( [](Future<int>::ValueType& vt)
		{
			mel::text::info("Fut1 set! {}",vt.value());
		});
		fut2.subscribeCallback( [](Future<int>::ValueType& vt)
		{
			mel::text::info("Fut2 set! {}",vt.value());
		});
		fut3.subscribeCallback( [](Future<int>::ValueType& vt)
		{
			mel::text::info("Fut3 set! {}",vt.value());
		});
		fut4.subscribeCallback( [](Future<int>::ValueType& vt)
		{
			mel::text::info("Fut4 set! {}",vt.value());
		});
/*		 meditar sobre esto. Funciona bien pero tengo que dejarlo bien hilvanado: igual tengo que ponerle otro nombre, ya que 
		 	viendo eso uno pensaría que sólo cambia el fut2, cuando en realidad cambia el data por lo que afecta a todos
			 POSIBILDIADES:
			  - llamarlo change		*/
		fut1.setValue(6);
 		fut2.assign(fut1);
		mel::text::info("Fut1 value {}",fut1.getValue().value());
		mel::text::info("Fut2 value {}",fut2.getValue().value());
		mel::text::info("Fut3 value {}",fut3.getValue().value());
		fut4 = fut1; 		
		mel::text::info("Fut4 value {}",fut3.getValue().value());
		//asignacion del data
		Thread::sleep(1000);
	}
	{
		//auto th1 = ThreadRunnable::create();
		Future<void> fut1;
		Future<void> fut2;
		Future<void> fut3(fut2);
		fut1.subscribeCallback( [](Future<void>::ValueType& vt)
		{
			mel::text::info("Fut1 set!");
		});
		fut2.subscribeCallback( [](Future<void>::ValueType& vt)
		{
			mel::text::info("Fut2 set!");
		});
		fut3.subscribeCallback( [](Future<void>::ValueType& vt)
		{
			mel::text::info("Fut3 set!");
		});
/*		 meditar sobre esto. Funciona bien pero tengo que dejarlo bien hilvanado: igual tengo que ponerle otro nombre, ya que 
		 	viendo eso uno pensaría que sólo cambia el fut2, cuando en realidad cambia el data por lo que afecta a todos
			 POSIBILDIADES:
			  - llamarlo change		*/
 		fut2.assign(fut1);
		fut1.setValue();
		//asignacion del data
		Thread::sleep(1000);
	}

	mel::text::set_level(mel::text::level::debug);
	{

		auto th1 = ThreadRunnable::create();
		Future<int> fut;
		int cont = 0;;
		th1->post([fut,&cont](RUNNABLE_TASK_PARAMS) mutable
		{
			//auto th = ThreadRunnable::getCurrentThreadRunnable(); 
			if ( cont == 1 )
			{
				mel::text::debug("UNO");
				//tasking::Process::sleep();
				mel::tasking::Process::wait(2500);
				mel::text::debug("DOS");
				mel::tasking::Process::wait(5000);
				mel::text::debug("TRES");
				cont = 0;
			}else
			{
				++cont;
				mel::text::debug("normal");
			}

			//fut.setValue(10);
			fut.setError(std::runtime_error("Prueba error"));;
		//	fut.setError( "error infame");
			//return ::mel::tasking::EGenericProcessResult::KILL;
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},Runnable::killTrue,1000);
		th1->post([](RUNNABLE_TASK_PARAMS)
		{
			mel::tasking::Process::wait(100);
			mel::text::debug("CUATRO");
			mel::tasking::Process::wait(2200);
			mel::text::debug("CINCO");
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},Runnable::killTrue,700);
/*		try
		{
			auto res = mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(fut);
			mel::text::info("Valor = {}",si
		{
			mel::text::error("unknown exception");
		}
		*/
		auto res = mel::core::waitForFutureThread<::mel::core::WaitErrorNoException>(fut);
		if ( res.isValid())
			mel::text::info("Valor = {}",res.value());
		else
		{
			try
			{
				std::rethrow_exception(res.error());
			}catch(std::exception& e)
			{
				mel::text::error("exception: {}",e.what());;
			}catch(...)
			{
				mel::text::error("unknown exception");
			}
		}
		Thread::sleep(11000);		
		mel::text::info("HECHO");
	}
	return 0;
}
static int _testMicroThreadingMonoThread(tests::BaseTest* test)
{
	mel::text::set_level(mel::text::level::debug);
	auto lvl = mel::text::get_level();
	using namespace std::string_literals;
	size_t s1 = sizeof(Process);
	size_t s2 = sizeof(GenericProcess);
	size_t s3 = sizeof(mel::tasking::MThreadAttributtes);
	mel::text::info("Process size {} ; GenericProcess size {}; MThreadAttributes {} ",s1,s2,s3);

	{
		auto th1 = ThreadRunnable::create();
		mel::text::debug("Request execution");
		auto fut = th1->execute<int>([th = th1.get()]{
			mel::text::debug("Start function execution...");
			if ( ::mel::tasking::Process::wait(2000) != mel::tasking::Process::ESwitchResult::ESWITCH_OK )
			{
				mel::text::error("Task killed!!");
				Timer t;				
				auto t0 = t.getMilliseconds();
				mel::text::debug("La espera siguiente no deberia tener efecto");
				//::tasking::Process::wait(2000);
				::mel::tasking::Process::sleep();
				auto t1 = t.getMilliseconds();
				if ( t1 != t0 )
					mel::text::error("Wait shoudn't have been done");
				else
					mel::text::info("Wait wasn't done, OK");
				//now, try to execute "something" in same thread. Shoouldn do anything				
				//@todo ahora está impidiendo la espera correctamente. Lo que tengo que meditar es si debo impedir el execute como tal, ya que se está haciendo
				//igual no tiene sentido porque el execute realmente se hace desde cualqueir sitio, no un mthread forzosamente
				auto fut = th->execute<float>([]{
					mel::text::error("This execution shouldn''t be done");
					return 1.5f;
					});
				try
				{
					auto fr = ::mel::tasking::waitForFutureMThread(fut);
					mel::text::info("OK, execution was't done because process is killed");
				}catch(...)
				{
					mel::text::error("Execution should have recieved a kill signal");
				}				
				/*
				if ( !fr.isValid() && fr.error().error == ::mel::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL)
					text::info("OK, execution was't done because process is killed");
				else
					text::error("Execution should have recieved a kill signal");
				*/

			}
			mel::text::debug("Return result");
			return 6;},Runnable::killTrue);
			
		mel::text::debug("Start Waiting for result");
		// auto fr = ::mel::core::waitForFutureThread(fut,2000);
		// mel::text::debug("Wait done");
		// if ( !fr.isValid())
		// 	text::error(fr.error().errorMsg);
		// else
		// 	text::debug("Result value = {}",fr.value());
	}
	return 0;
	int result = 0;
	int sharedVar = 0;
	auto th1 = ThreadRunnable::create();
	auto th2 = ThreadRunnable::create(true);

	
	th1->post( [th2](RUNNABLE_TASK_PARAMS)
	{
		auto& autokill = Runnable::killTrue;
		std::shared_ptr<Process> p1=th2->post([th2](uint64_t t,Process* p)
		{
			static bool firstTime = true;
			mel::text::debug("Ejecuto p1");
			if ( firstTime )
			{
				firstTime = false;

				auto fut = th2->execute<int>(
					[]()
					{
						::mel::tasking::Process::wait(20000);
						return 6;
					}
				);

				try
				{
					auto fr = ::mel::tasking::waitForFutureMThread(fut,2000);
					mel::text::debug("{}",fr.value());
				}catch(std::exception& e)
				{
					mel::text::error(e.what());
				}
			
				// if ( fr != ::mel::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK)
				// {
				// 	spdlog::error(fut.getValue().error().errorMsg);
				// }
				mel::text::debug("espero en p1");
				auto wr = mel::tasking::Process::wait(10000);			
				mel::text::debug("vuelvo a esperar en p1");

				// esto tengo que arreglarlo para que, si soy autokill, no vuelva a esperar si está en trying_tokill
				// meditar sobre estos temas, no me convence demasiado la forma en que está planteado
				//wr = mel::tasking::Process::wait(10000);
				wr = mel::tasking::Process::switchProcess(true);
				wr = mel::tasking::Process::sleep();
				mel::text::debug("fin espera en p1");				 
			
			}
			mel::text::debug("Continuo");
			return ::mel::tasking::EGenericProcessResult::CONTINUE;
		},autokill,2000,000);
		th2->fireAndForget(
			[p1]()
			{
				mel::text::debug("Ejecuto p2");
				mel::tasking::Process::wait(4000);
				// spdlog::debug("Pauso proceso");
				//  if ( p1 )
				//  	p1->pause();
			//	tasking::Process::wait(1000);
				mel::text::debug("Mato proceso");
				p1->kill();
				// spdlog::debug("Vuelvo a pausar proceso");
				// if ( p1 )
				// 	p1->pause();
				mel::tasking::Process::wait(25000);
				mel::text::debug("Despierto proceso");
				if ( p1 )
				 	p1->wakeUp();
			}
		);

		mel::text::debug("execution done");
		return ::mel::tasking::EGenericProcessResult::KILL;
	}
	,Runnable::killTrue,3000);
	
	/*
	th1->post( [&sharedVar](RUNNABLE_TASK_PARAMS)
	{
		int aux = sharedVar;
		sharedVar++;
		spdlog::debug("Lambda");
		auto t1 = p->getElapsedTime();
		CHECK_TIME(t1,p->getPeriod(),"check period"s);
		auto t0 = sTimer.getMilliseconds();
		unsigned int waittime = 3550;
		::tasking::Process::wait(waittime);
		t1 = sTimer.getMilliseconds();
		CHECK_TIME(t1-t0,waittime,"check wait 1"s);
		waittime = 67;
		::tasking::Process::wait(waittime);
		auto t2 = sTimer.getMilliseconds();
		CHECK_TIME(t2-t1,waittime,"check wait 2"s);
		t0 = sTimer.getMilliseconds();
		::tasking::Process::wait(2000);
		t1 = sTimer.getMilliseconds();
		spdlog::debug("Elapsed {}",t1-t0);
		sharedVar = aux;
		return ::mel::tasking::EGenericProcessResult::CONTINUE;
	},true,2000,000);
	*/
	// th1->post<CustomProcessType,MyAllocator>(
	// 	::mpl::linkFunctor<::mel::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(staticFuncTask,::mpl::_v1,::mpl::_v2,::mpl::_v3,mpl::createRef(sharedVar))
	// 	,true,4200);
	// auto p = make_shared<MyProcess>(sharedVar);
	// p->setPeriod(0);
	// th1->postTask(p);
	// th1->post(MyTask(p.get(),sharedVar),true,1200);

/*
preparar bien el test: quiero que los procesos actúa sobre algún objeto y tenga una salida precedible, por ejemplo:
 - incrementar/dec variable de forma que deba siempre ser isgreaterequal
- 
*/
	mel::text::debug("finish");
	// th1->finish();
	// th1->join();
	
	return 0;
}
//check performance launching a lot of tasks
//@todo habrái que hacerlo con un profiler, un sistema de benchmarking...
int  _testPerformanceLotTasks(tests::BaseTest* test)
{
	int result = 0;
	constexpr int nIterations = 1;
	constexpr int nTasks = 100000;

	Runnable::RunnableCreationOptions opts;
	opts.maxPoolSize = nTasks;
	auto th1 =ThreadRunnable::create(true,opts);
	th1->start();	
	uint64_t t0,t1;
	int count = 0;
	t0 = sTimer.getMilliseconds();
	auto steps = nTasks/th1->getMaxPoolSize();
	
	for(int it = 0; it < nIterations; ++it)
	{
		for(int j = 0;j<steps;++j)
		{
			for(int i = 0; i < th1->getMaxPoolSize(); ++i)
			{
				++count;
				//th1->post<CustomProcessType,MyAllocator>( [count](RUNNABLE_TASK_PARAMS)				
				th1->post<false,MyAllocator>( [count](RUNNABLE_TASK_PARAMS)
				{
					return ::mel::tasking::EGenericProcessResult::KILL;
				},Runnable::killFalse,1000,0);		
			}
			::Thread::sleep(1) ;//to wait for taks
		}
		Thread::sleep(10);
	}
	t1 = sTimer.getMilliseconds();	
	mel::text::info("Time launching {} tasks with global new: {} msecs",nTasks,t1-t0);

	Thread::sleep(2000);
	t0 = sTimer.getMilliseconds();	;
	count = 0;
	for(int it = 0; it < nIterations; ++it)
	{
		for(int j = 0;j<steps;++j)
		{
			for(int i = 0; i < th1->getMaxPoolSize(); ++i)
			{
				++count;
				th1->post( [count](RUNNABLE_TASK_PARAMS)
				{
					//spdlog::debug("Lambda {}",count);
					return ::mel::tasking::EGenericProcessResult::KILL;
				},Runnable::killFalse,1000,0);
			}	
			::Thread::sleep(1) ;//wait for tasks finished
		}
		Thread::sleep(10);
	}
	t1 = sTimer.getMilliseconds();
	mel::text::info("Time launching {} tasks with default allocator: {} msecs",nTasks,t1-t0);
	//Thread::sleep(45000);	
	return result;
}
//test for concurrent post in lock-free scheduler
int _test_concurrent_post( ::tests::BaseTest* test)
{	
	{
		//meter test bueno como si furrulase
		constexpr int NUM_POSTS = 100'000;
		//constexpr int NUM_PRODUCERS = 500;
		constexpr int NUM_PRODUCERS = 7; //in github action increasing this number leads to very high test time
		sCount = 0;
		mel::text::info("Concurrent posts with normal sized buffer. Num producers={}, num_posts={}",NUM_PRODUCERS,NUM_POSTS);

		Runnable::RunnableCreationOptions opts;
		opts.schedulerOpts = ProcessScheduler::LockFreeOptions{};
		{		
			auto consumer =ThreadRunnable::create(true,opts); //en realidad en free lock el maxSize es el chunksize. Cuando l otenga como opciones ya lo haré conherente
			std::array< std::shared_ptr<ThreadRunnable>,NUM_PRODUCERS> producers;
			for(size_t i=0;i<producers.size();++i)
			{
				producers[i] = ThreadRunnable::create(true);
			}
			for(size_t i=0;i<producers.size();++i)
			{		
				producers[i]->post([consumer,NUM_POSTS](uint64_t,Process*)
				{
				//	mel::tasking::Process::wait(50);//to wait for all producer posts done
					for(auto i = 0; i < NUM_POSTS; ++i)
					{
						//@todo así peta. Es algo de destruiccion del hilo, ya que esto hará que tarde mś que la espera a fin
						//if (mel::tasking::Process::wait(1000) == ::mel::tasking::Process::ESwitchResult::ESWITCH_OK)
						{
							consumer->fireAndForget(
								[]()
								{
									++sCount;
								},0,Runnable::killFalse
							);
						}
					//	::mel::tasking::Process::wait(1);
					}
					return mel::tasking::EGenericProcessResult::KILL;
				},Runnable::killFalse);
			}
			mel::text::info("Waiting for tasks finished...");
			//Thread::sleep(10000);
		}
		auto numPosts = NUM_PRODUCERS*NUM_POSTS;
		if ( sCount != numPosts )
		{
			std::stringstream ss;
			ss<<"Test concurrent posts KO!!. Some tasks were not executed. Posts: "<<numPosts<<" Count: "<<sCount.load();
			test->setFailed(ss.str());
		}else
		{
			mel::text::info("Test concurrent posts OK!!");
		}
	}
	{
		constexpr int NUM_POSTS = 100'000;
		//constexpr int NUM_PRODUCERS = 50;
		constexpr int NUM_PRODUCERS = 5;
		sCount = 0;
		mel::text::info("Concurrent posts with very small sized buffer. Num producers={}, num_posts={}",NUM_PRODUCERS,NUM_POSTS);

		Runnable::RunnableCreationOptions opts;
		opts.schedulerOpts = ProcessScheduler::LockFreeOptions{10,2}; //provoca pete
		{		
			auto consumer =ThreadRunnable::create(true,opts); //en realidad en free lock el maxSize es el chunksize. Cuando l otenga como opciones ya lo haré conherente
			std::array< std::shared_ptr<ThreadRunnable>,NUM_PRODUCERS> producers;
			for(size_t i=0;i<producers.size();++i)
			{
				producers[i] = ThreadRunnable::create(true);
			}
			for(size_t i=0;i<producers.size();++i)
			{		
				producers[i]->post([consumer,NUM_POSTS](uint64_t,Process*)
				{
				//	mel::tasking::Process::wait(50);//to wait for all producer posts done
					for(auto i = 0; i < NUM_POSTS; ++i)
					{
						//@todo así peta. Es algo de destruiccion del hilo, ya que esto hará que tarde mś que la espera a fin
						//if (mel::tasking::Process::wait(1000) == ::mel::tasking::Process::ESwitchResult::ESWITCH_OK)
						{
							consumer->fireAndForget(
								[]()
								{
									++sCount;
								},0,Runnable::killFalse
							);
						}
					//	::mel::tasking::Process::wait(1);
					}
					return mel::tasking::EGenericProcessResult::KILL;
				},Runnable::killFalse);
			}
			mel::text::info("Waiting for tasks finished");
			//Thread::sleep(10000);
		}
		auto numPosts = NUM_PRODUCERS*NUM_POSTS;
		if ( sCount != numPosts )
		{
			std::stringstream ss;
			ss<<"Test concurrent posts KO!!. Some tasks were not executed. Posts: "<<numPosts<<" Count: "<<sCount.load();
			test->setFailed(ss.str());
		}else
		{
			mel::text::info("Test concurrent posts OK!!");
		}
	}
	return 0;
}
int _throwExc(int& v)
{
	v = rand();
	throw v;
}
struct MyException
{
	int code;
	string msg;
};
void _throwMyException(int& v)
{
	v = rand();
//	throw v;
	throw MyException{v,"MyException "+std::to_string(v)};
}
auto _doWait(unsigned int msecs)
{
	// auto except = std::current_exception();

	// if (except)
	// {
	// 	try{
	// 		std::rethrow_exception(except);
	// 	}catch(...){}
	// }
	// quisiera que aqui ya no hubiese excepcion
	// except = std::current_exception();
	return mel::tasking::Process::wait(msecs);
}
int _testExceptions( tests::BaseTest* test)
{
	mel::text::info("Test Exceptions");
	auto th1 = ThreadRunnable::create(true);
	th1->post(
		 [test](RUNNABLE_TASK_PARAMS)
		 {
			std::exception_ptr except;
			int val;
			try
			{
				mel::text::info("Task1: Context switch");
				if ( _doWait(200) == Process::ESwitchResult::ESWITCH_OK)
				{
					mel::text::info("Task1: Throw exception");
					// if (rand()%10 < 5)
					// 	_throwMyException(val);
					// else
						_throwExc(val); 
					test->setFailed("Task1: After throw exception. Shouldn't occur");
				}

			}catch(int v)
			{
				if ( v == val)
					mel::text::info("Task1: Captured exception ok");
				else
				{
					stringstream ss;
					ss<<"Task1: Invalid 'int' Captured exception. Thrown "<<val<<" Catched "<<v;
					test->setFailed(ss.str());
				}
				// igual, puedo ahcer algo de esto en el wait, restaurandolo a la vuelta o similar..en este caso tengo que hacer buenas pruebas con excepciones encadenadas
				// aunque no tengo claro como tendría que restaurarla..
				// mirar el uncaught_exceptions 
				// el problema es que no puedo siempre "obligar" a que no se haga u nswitch dentro de un catch, porque se podría llamar a funciones desde aquí
				//auto except = std::current_exception();				
				//_doWait(500);
			}catch(MyException& exc)
			{
				if ( exc.code == val)
				{
					mel::text::info("Task1: Captured exception ok. msg ={}",exc.msg);
				}
				else
				{
					stringstream ss;
					ss<<"Task1: Invalid 'MyException' Captured exception. Thrown "<<val<<" Catched "<<exc.code;
					test->setFailed(ss.str());
//					mel::text::error("Task1:onExecuteAlltests Captured exception Invalid,. Thrown {}, Catched {}",val,exc.code);
				}
			//	mel::tasking::Process::wait(500); //haciendo esto aquí, peta al poco (Windows)
			}			
			
			_doWait(500);
			 return ::mel::tasking::EGenericProcessResult::CONTINUE;
		 },Runnable::killTrue
	);
	th1->post(
		 [test](RUNNABLE_TASK_PARAMS)
		 {
			 int val;
			try
			{

				mel::text::info("Task2: Context switch");
				if (_doWait(300)  == Process::ESwitchResult::ESWITCH_OK)
				{
					mel::text::info("Task2: Throw exception");
					if (rand()%10 < 5)
						_throwExc(val);
					else
						_throwMyException(val);
					test->setFailed("Task2: After throw exception. Shouldn't occur");
				}
			}
			catch(MyException& exc)
			{
				if ( exc.code == val)
				{
					mel::text::info("Task2: Captured exception ok. msg ={}",exc.msg);
				}
				else
				{
					stringstream ss;
					ss<<"Task2: Invalid 'MyException' Captured exception. Thrown "<<val<<" Catched "<<exc.code;
					test->setFailed(ss.str());
				}
			}	
			catch( int v)
			{
				if ( v == val)
					mel::text::info("Task2: Captured exception ok");
				else
				{
					stringstream ss;
					ss<<"Task2: Invalid Captured exception. Thrown "<<val<<" Catched "<<v;
					test->setFailed(ss.str());
				}
			}
			_doWait(1260);			
			
			 return ::mel::tasking::EGenericProcessResult::CONTINUE;
		 },Runnable::killTrue
	);
	th1->post(
		 [test](RUNNABLE_TASK_PARAMS)
		 {
			 int val;
			try
			{

				mel::text::info("Task3: Context switch");
				if (_doWait(500)  == Process::ESwitchResult::ESWITCH_OK)
				{
					mel::text::info("Task3: Throw exception");
					if (rand()%10 < 5)
						_throwExc(val);
					else
						_throwMyException(val);
					test->setFailed("Task3: After throw exception. Shouldn't occur");
				}
			}
			catch(MyException& exc)
			{
				if ( exc.code == val)
				{
					mel::text::info("Task3: Captured exception ok. msg ={}",exc.msg);
				}
				else
				{
					stringstream ss;
					ss<<"Task3: Invalid 'MyException' Captured exception. Thrown "<<val<<" Catched "<<exc.code;
					test->setFailed(ss.str());
				}
			}	
			catch( int v)
			{
				if ( v == val)
					mel::text::info("Task3: Captured exception ok");
				else
				{
					stringstream ss;
					ss<<"Task3: Invalid Captured exception. Thrown "<<val<<" Catched "<<v;
					test->setFailed(ss.str());
				}
			}
			_doWait(750);
			
			 return ::mel::tasking::EGenericProcessResult::CONTINUE;
		 },Runnable::killTrue
	);
	Thread::sleep(30000); //test time
	mel::text::info("Test Exceptions finished ok");
	return 0;
}


int TestThreading::onExecuteTest()
{
	int result = 1;
	typedef int(*TestType)(tests::BaseTest*);
	TestType defaultTest = ::test_threading::test_futures;
	auto opt = tests::CommandLine::getSingleton().getOption("n");
	if ( opt != nullopt)
	{
		try
		{
			auto n = std::stol(opt.value());
			switch(n)
			{
				case 0:
					result = _testMicroThreadingMonoThread( this );
					break;
				case 1:
					result = _testPerformanceLotTasks(this);
					break;
				case 2:
					result = ::test_threading::basicTestFutures( this );
					break;
				case 3:
					result = _test_concurrent_post(this);
					break;
				case 4:
					result = _testExceptions(this);
					break;
				case 5:
					result = ::test_threading::test_futures( this );
					break;					
				case 1000:
					result = _testsDebug(this);
					break;
				case 1001:
					samples();
					break;
				default:;					
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}		
	}else
		result = defaultTest(this); //by default
		
		
	return 0;
}
void TestThreading::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"threading tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",make_unique<TestThreading>());
}
int TestThreading::onExecuteAllTests()
{
	//@todo tengo quitados los testes que todavía no están bien
	//_testMicroThreadingMonoThread( this );
	//_testPerformanceLotTasks(this);
	::test_threading::allTests(this);
	 //_test_concurrent_post(this);	
	 _testExceptions(this);
	 return 0;
}