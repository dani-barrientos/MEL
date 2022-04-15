#include "future_tests.h"
#include <tasking/ThreadRunnable.h>
using mel::tasking::ThreadRunnable;
using namespace std;
#include <text/logger.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/utilities.h>
#include <parallelism/Barrier.h>
using ::mel::parallelism::Barrier;
#include <array>
#include <CommandLine.h>
#include <vector>
using std::vector;
using namespace mel;
//Test with custom error
struct MyErrorInfo 
{
	MyErrorInfo(int code,string msg):error(code),errorMsg(std::move(msg))
	{
		text::debug("MyErrorInfo");
	}
	MyErrorInfo(MyErrorInfo&& ei):error(ei.error),errorMsg(std::move(ei.errorMsg)){}
	MyErrorInfo(const MyErrorInfo& ei):error(ei.error),errorMsg(ei.errorMsg){}
	MyErrorInfo& operator = (MyErrorInfo&& info)
	{
		error = info.error;
		errorMsg = std::move(info.errorMsg);
		return *this;
	}
	MyErrorInfo& operator = (const MyErrorInfo& info)
	{
		error = info.error;
		errorMsg = info.errorMsg;
		return *this;
	}
	int error;
	string errorMsg;
};

struct _Stack
{
	vector<uint8_t> _stack;
	_Stack(volatile uint8_t* end,volatile uint8_t* start)
	{
		volatile uint8_t* _start;
		volatile uint8_t* _end;
		if ( end < start )
		{
			_start = end;
			_end = start;

		}else
		{
			_start = start;
			_end = end;
		}
		while(_start!=_end)
		{
			uint8_t val = *_start;
			_stack.push_back(val);
			++_start;
		}
	}
	bool compare(volatile uint8_t* end,volatile uint8_t* start)
	{
		volatile uint8_t* _start;
		volatile uint8_t* _end;
		if ( end < start )
		{
			_start = end;
			_end = start;

		}else
		{
			_start = start;
			_end = end;
		}
		
		for(auto v:_stack)
		{
			if ( v != *_start++)
				return false;
		}
		return true;
	}
};
#define CHECK_STACK_SAVE(end,start) \
	_Stack _stck(end,start);


#define CHECK_STACK_VERIFY(end,start,test) \
	if ( !_stck.compare(end,start) ){ \
		test->setFailed();/*finish();*/} 

template <size_t nConsumers>
class MasterThread : public ThreadRunnable
{
	public:
		MasterThread(std::shared_ptr<ThreadRunnable> producer,std::array<std::shared_ptr<ThreadRunnable>,nConsumers> consumers,unsigned int maxTime,::tests::BaseTest* test) : 
			ThreadRunnable(),mConsumers(consumers),mProducer(producer),mMaxTime(maxTime),mTest(test)
		{	
		}	
		~MasterThread()
		{

		}
	private:
		typedef Future<int> FutureType;

		std::array<std::shared_ptr<ThreadRunnable>,nConsumers> mConsumers;
		std::shared_ptr<ThreadRunnable> mProducer;
		int				 mValueToAdd;
		Barrier			mBarrier;
		uint64_t		mLastDebugTime; //para mosrtar mensaje de debug de que todo va bien
		uint64_t 		mStartTime;
		unsigned int	mMaxTime; //msecs to do test
		::tests::BaseTest* mTest;
		void onStart() override
		{
			auto msecs = this->getTimer()->getMilliseconds();
			mStartTime = msecs;
			srand((unsigned)msecs);
			post(::mel::mpl::makeMemberEncapsulate(&MasterThread::_masterTask,this));					
		}
		::tasking::EGenericProcessResult _masterTask(uint64_t msecs,Process* p) 
		{	
			constexpr auto mthreadProb = 0.7f; //probability of task being a microthread instead blocking thread
			//constexpr auto mthreadProb = 1.0f; //probability of task being a microthread instead blocking thread
			constexpr auto newTaskProb = 0.5f; //probability of launching a new task
			//constexpr auto newTaskProb = 0.0f; //probability of launching a new task
			constexpr auto maxTasks = 500;
			//constexpr auto maxTasks = 1;
			//a common future ("channel") is created so producer will put ther its value and cosumers wait for it
			FutureType channel;            
			int nSinglethreads = 0;
			auto prodIdx = rand()%(nConsumers+1);
			mValueToAdd = rand()%50;  //value to add to input in consumers			
			text::debug("Producer idx {} de {}. Consumers must add {} to their input",prodIdx,nConsumers,mValueToAdd);		
			int nTasks = 0;

			//pause consumer. They will be started when al ltask posted
			for(auto th:mConsumers)
				th->suspend();
			for(size_t i = 0; i < nConsumers; ++i)
			{
				if ( i == prodIdx)
				{
					mProducer->fireAndForget(
						mpl::linkFunctor<void,TYPELIST()>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),channel)
					);
				}

				if ( rand() < (int)RAND_MAX*mthreadProb)
				{
					do
					{
						text::debug("launch task {}",nTasks);
						FutureType result;
						//Select different stakc sizes
						auto idx = rand()%4;
						switch (idx)
						{
						case 0:
							mConsumers[i]->post(
								mpl::linkFunctor<::mel::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(
								makeMemberEncapsulate(&MasterThread::_consumerTask<10>,this),::mpl::_v1,::mpl::_v2,channel,result,nTasks++)
							);
							break;
						case 1:
							mConsumers[i]->post(
								mpl::linkFunctor<::mel::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(
								makeMemberEncapsulate(&MasterThread::_consumerTask<100>,this),::mpl::_v1,::mpl::_v2,channel,result,nTasks++)
							);
							break;
						case 2:
							mConsumers[i]->post(
								mpl::linkFunctor<::mel::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(
								makeMemberEncapsulate(&MasterThread::_consumerTask<1000>,this),::mpl::_v1,::mpl::_v2,channel,result,nTasks++)
							);
							break;							
						case 3:
							mConsumers[i]->post(
								mpl::linkFunctor<::mel::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(
								makeMemberEncapsulate(&MasterThread::_consumerTask<2000>,this),::mpl::_v1,::mpl::_v2,channel,result,nTasks++)
							);
							break;
						case 4:
							mConsumers[i]->post(
								mpl::linkFunctor<::mel::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(
								makeMemberEncapsulate(&MasterThread::_consumerTask<64>,this),::mpl::_v1,::mpl::_v2,channel,result,nTasks++)
							);
							break;
						default:
							break;
						}
						
// el problema que tengo ahora es que el argumento del Callback no es const. 
// Necesito que sea const para subscripciones const, y n oconst las otras

						++nTasks; //take next task into account
						post( [this,channel,result](uint64_t,Process*) mutable
							{
								try
								{
									auto wr = ::mel::tasking::waitForFutureMThread(result);
									auto val = channel.getValue().value() + mValueToAdd;									
									if ( val != wr.value())
										text::error("Result value is not the expected one!!. Get {}, expected {}",result.getValue().value(),val);
								}catch(...){}
							
								mBarrier.set();	
								return ::mel::tasking::EGenericProcessResult::KILL;
							}
						);
					}while(rand()< (int)(RAND_MAX*newTaskProb) && nTasks < maxTasks);
				}
				else
				{
					nSinglethreads++;
					mConsumers[i]->fireAndForget(
						mpl::linkFunctor<void,TYPELIST()>(
						makeMemberEncapsulate(&MasterThread::_consumerTaskAsThread,this),channel,nTasks++)
					);
				}
			}
			if ( prodIdx == nConsumers)
			{
				mProducer->fireAndForget(
						mpl::linkFunctor<void,TYPELIST()>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),channel)
					);
			
			}
			text::debug("{} jobs have been launched, from which {} are single threads",nTasks,nSinglethreads);

			mBarrier = Barrier(nTasks);
			for(auto th:mConsumers)
				th->resume();
			auto t0 = getTimer()->getMilliseconds();
            constexpr unsigned MAX_TIME = 5500;
			auto r = ::mel::tasking::waitForBarrierMThread(mBarrier,MAX_TIME);
			if ( r != ::mel::tasking::EEventMTWaitCode::EVENTMT_WAIT_OK )
			{
				text::error("Wait for responses failed!!!!. {} workers remaining",mBarrier.getActiveWorkers());
				this->terminate();  
				return ::mel::tasking::EGenericProcessResult::KILL; 
			}
			else if ( (msecs - mLastDebugTime) >= 5000 )
				{
					mLastDebugTime = msecs;
					auto t1 = getTimer()->getMilliseconds();
					text::info("Wait for responses ok. Time waiting: {} msecs",t1-t0);
				}
			
			if ( msecs - mStartTime > mMaxTime)
			{
				text::info("Maximum test time reached. Finishing");
				this->terminate();  
				return ::mel::tasking::EGenericProcessResult::KILL; 
			}else			
				return ::mel::tasking::EGenericProcessResult::CONTINUE;
		}
		void _producerTask(FutureType output) 
		{			
			//generar el output como sea (tiempo aleatorio, etc..)
			//@note remember C++11 has cool functions for random numbers in <random> header
			constexpr unsigned max = 20;
            constexpr auto errProb = 0.2;
			//constexpr auto errProb = 0.0;
			auto value = rand()%max;
			unsigned int waitTime = rand()%1000;
			Process::wait(waitTime);
            if ( value >= max*errProb )
            {
			    mel::text::debug("Genero valor = {}",value);
                output.setValue(value);
            }else
            {
                output.setError(MyErrorInfo(0,"PRUEBA ERROR"));
                mel::text::debug("Genero error");
            }			
		}
		template <int N> ::mel::tasking::EGenericProcessResult _consumerTask(uint64_t,Process*, FutureType input,FutureType output,int taskId ) 
		{						
			int arr[N];	
			//fill arr with random numbers
			for( size_t i= 0; i < N;++i )
			{
				arr[i] = rand();
			//	como lo chequeo?->guardar pila en algun sitio, con indice o similar
			}
			unsigned int waitTime = rand()%150;			
			text::debug("Task {} waits for input",taskId);
			CHECK_STACK_SAVE((uint8_t*)&(arr[0]),(uint8_t*)&(arr[N-1]))
			//CHECK_STACK_SAVE((uint8_t*)&(arr[N-4]),(uint8_t*)&(arr[0]))
			Process::wait(waitTime); //random wait
			try
			{
				auto wr = ::mel::tasking::waitForFutureMThread(input);
					output.setValue(wr.value() + mValueToAdd);
				text::debug("Task {} gets value {}",taskId,input.getValue().value());
			}catch(std::exception& e)
			{
				text::debug("Task {} gets error waiting for input: {}",taskId,e.what());
				output.setError( std::current_exception());
				
			}catch(...)
			{
				text::debug("Task {} gets error waiting for input. unknown error",taskId);
				output.setError( std::current_exception());
			}

			/*
			if ( wr.isValid() )
			{
				output.setValue(wr.value() + mValueToAdd);
				text::debug("Task {} gets value {}",taskId,input.getValue().value());
			}else
			{
				text::debug("Task {} gets error waiting for input: {}",taskId,input.getValue().error().errorMsg);
					output.setError( MyErrorInfo(0,""));
			}
			*/
			
			mBarrier.set();
			CHECK_STACK_VERIFY((uint8_t*)&(arr[0]),(uint8_t*)&(arr[N-1]),mTest)
			//CHECK_STACK_VERIFY((uint8_t*)&(arr[N-4]),(uint8_t*)&(arr[0]),mTest)
			return ::mel::tasking::EGenericProcessResult::KILL;
		}
		void _consumerTaskAsThread(FutureType input,int taskId ) 
		{
			unsigned int waitTime = rand()%150;
			Process::wait(waitTime); //random wait
			try
			{
				auto wr = ::mel::core::waitForFutureThread(input);
				text::debug("Thread {} gets value {}",taskId,wr.value());		
			}
			catch(const std::exception& e)
			{
				text::debug("Thread {} gets error waiting for input: {}",taskId,e.what());
			}			
			catch(MyErrorInfo& e)  //prueba captura custom exception
			{
				text::debug("Thread {} gets error waiting for input: {}",taskId,e.errorMsg);
			}
			catch(...)
			{
				text::debug("Thread {} gets error waiting for input. UNKNOWN");
			}
//			esto no funciona-. En mi ejemplo compiler explorer si, será una boabda
			
			/*
			if ( wr.isValid() )
			{				
				text::debug("Thread {} gets value {}",taskId,wr.value());		
			}else
			{
				text::debug("Thread {} gets error waiting for input: {}",taskId,input.getValue().error().errorMsg);
			}*/		
			mBarrier.set();
		}
		void onThreadEnd() override{
			//finalizar el resto
            mProducer->terminate();
			for(auto th:mConsumers)
			{
				th->terminate();				
				//@todo qué pasa con el join? quiero esperar antes por los demas, ¿virtual?
			}			
		}		
		void onJoined() override
		{
            mProducer->join();
			for(auto th:mConsumers)
			{
				th->join();
			}
		}
};
//helper for mkasterthread destroy
struct ThreadRunnableProxy
{
	std::shared_ptr<ThreadRunnable> mPtr;
	ThreadRunnableProxy(std::shared_ptr<ThreadRunnable> ptr ):mPtr(ptr){}
	~ThreadRunnableProxy()
	{
		mPtr->terminate();
		mPtr->join(); 
	}

};
int test_threading::test_futures( tests::BaseTest* test)
{
	int result = 0;
	//@todo hasta que no haga bien lo del autodestroy, esto no está bien del todo. Deberia pasar los consumidores como shared_ptr
	
	auto producer = ThreadRunnable::create(true);
	
	text::set_level(text::level::info);

	constexpr size_t nConsumers = 10;
	constexpr unsigned int DEFAULT_TESTTIME = 60*1000;
	unsigned int testTime;
	//get test time (seconds)
	auto opt = tests::CommandLine::getSingleton().getOption("tt");
	if ( opt != nullopt)
	{
		testTime = std::stoi(opt.value())*1000;
	}else
		testTime = DEFAULT_TESTTIME;
	text::info("Test time {}",testTime);
	std::array< std::shared_ptr<ThreadRunnable>,nConsumers> consumers;
	for(size_t i=0;i<nConsumers;++i)
	{
		consumers[i] = ThreadRunnable::create(true);
	}
	auto master = make_shared<MasterThread<nConsumers>>(producer,consumers,testTime,test);
	ThreadRunnableProxy proxy(master);
	master->start();	
		
	return result;
}
