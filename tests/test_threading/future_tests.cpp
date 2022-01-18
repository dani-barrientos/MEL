#include "future_tests.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <spdlog/spdlog.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/utilities.h>
#include <parallelism/Barrier.h>
using ::parallelism::Barrier;
/*
	class Barrier;
	class BarrierData : private CallbackSubscriptor<::core::NoMultithreadPolicy,const BarrierData&>,
		public std::enable_shared_from_this<BarrierData>
	{
		friend class ::Barrier;		
		typedef CallbackSubscriptor<::core::NoMultithreadPolicy,const BarrierData&> Subscriptor;
	private:
		BarrierData(size_t nWorkers):mActiveWorkers(nWorkers){}		
		void set()
		{
			volatile auto protectMe = shared_from_this();
			::core::Lock lck(mCS);
			if ( --mActiveWorkers == 0 ) 
			{				
				triggerCallbacks(*this);
			}
		}
		inline int getActiveWorkers() const { return mActiveWorkers; }
		template <class F> auto subscribeCallback(F&& f)
		{
			volatile auto protectMe = shared_from_this();
			Lock lck(mCS);
			if (mActiveWorkers==0)
				f(*this);
			return Subscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f)
		{
			Lock lck(mCS);
			return Subscriptor::unsubscribeCallback(std::forward<F>(f));
		}
	protected:
		int	mActiveWorkers;  //para implementacion ingenua
		::core::CriticalSection mCS;

	};
	class Barrier
	{
	public:
		Barrier( size_t nWorkers = 1 ):mData( new BarrierData( nWorkers ) )
		{
		}
		Barrier( const Barrier& o2):mData(o2.mData){ }
		Barrier( Barrier&& o2):mData(std::move(o2.mData)){}
		Barrier& operator=(const Barrier& o2){mData = o2.mData;return *this;}
		Barrier& operator=(Barrier&& o2){mData = std::move(o2.mData);return *this;}
		// inline void addWorkers(size_t nWorkers)
		// {
		// 	mData->addWorkers(nWorkers);
		// }
		inline void set()
		{
			mData->set();
		}
		inline int getActiveWorkers() const
		{ 
			return mData->getActiveWorkers(); 
		}
		template <class F> auto subscribeCallback(F&& f) const
		{
			return const_cast<BarrierData*>(mData.get())->subscribeCallback(std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f) const
		{
			const_cast<BarrierData*>(mData.get())->unsubscribeCallback(std::forward<F>(f));
		}
	private:
		std::shared_ptr<BarrierData>	mData;
	protected:
		Barrier( BarrierData* data );
		
	};
*/
// static ::tasking::Event_mthread::EWaitCode waitForBarrierMThread(const Barrier& b,unsigned int msecs = ::tasking::Event_mthread::EVENTMT_WAIT_INFINITE )
// {
// 	using ::tasking::Event_mthread;

// 	struct _Receiver
// 	{		
// 		_Receiver():mEvent(false,false){}
// 		Event_mthread::EWaitCode wait(const Barrier& barrier,unsigned int msecs)
// 		{
// 			Event_mthread::EWaitCode eventresult;
// 		 	//spdlog::info("Waiting for event");
// 			int evId;
// 			eventresult = mEvent.waitAndDo([this,barrier,&evId]()
// 			{
// 			//   spdlog::debug("waitAndDo was done for Thread {}",threadid);
// 				evId = barrier.subscribeCallback(
// 				std::function<::core::ECallbackResult( const ::parallelism::BarrierData&)>([this](const ::parallelism::BarrierData& ) 
// 				{
// 					 mEvent.set();// el problema de esto es que puede destruir la barrera nates de terminbar:				
// 				    //spdlog::info("Event was set");
// 					return ::core::ECallbackResult::UNSUBSCRIBE; 
// 				}));
// 			},msecs); 
// 			barrier.unsubscribeCallback(evId);
// 			return eventresult;
// 		}
// 		private:
// 		::tasking::Event_mthread mEvent;

// 	};
// 	auto receiver = make_unique<_Receiver>();
// 	return receiver->wait(b,msecs);	
// }


template <size_t n>
class MasterThread : public GenericThread
{
	public:
		MasterThread(Thread* producer,std::array<Thread*,n> consumers) : GenericThread(false,true),mConsumers(consumers),mProducer(producer)
		{
		}	
	private:
		std::array<Thread*,n> mConsumers;
		Thread* mProducer;
		int				 mValueToAdd;
		Barrier			mBarrier;
		uint64_t		mLastDebugTime; //para mosrtar mensaje de debug de que todo va bien
		void onThreadStart() override
		{
			srand((unsigned)this->getTimer()->getMilliseconds());
			post( ::mpl::makeMemberEncapsulate(&MasterThread::_masterTask,this));			
			// //pruebas de tareas dummy
			// for(auto th:mConsumers)
			// {
			// 	th->post( [](uint64_t t, Process*, ::tasking::EGenericProcessState)
			// 	{
			// 		return ::tasking::EGenericProcessResult::CONTINUE;
			// 	},1000000);										
			// }
		}
		::tasking::EGenericProcessResult _masterTask(uint64_t msecs,Process* p, ::tasking::EGenericProcessState) 
		{	
			constexpr auto mthreadProb = 0.7f; //probability of task being a microthread instead blocking thread
			constexpr auto newTaskProb = 0.5f; //probability of launching a new task
			constexpr auto maxTasks = 500;
			//a common future ("channel") is created so producer will put ther its value and cosumers wait for it
			Future<int> channel;            
			int nSinglethreads = 0;
			auto prodIdx = rand()%(n+1);
			//auto prodIdx = -1; //para pruebas
			mValueToAdd = rand()%50;  //value to add to input in consumers
			spdlog::debug("Producer idx {} de {}. Consumers must add {} to their input",prodIdx,n,mValueToAdd);
			int nTasks = 0;

			//pause consumer. They will be started when al ltask posted
			for(auto th:mConsumers)
				th->suspend();
			for(size_t i = 0; i < n; ++i)
			{
				if ( i == prodIdx)
				{
					mProducer->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
					);
				}

				if ( rand() < RAND_MAX*mthreadProb)
				{
					do
					{
						Future<int> result;
						mConsumers[i]->post(
							mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MasterThread::_consumerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel,result,nTasks++)
						);
						++nTasks; //take next task into account
						post( [this,channel,result](uint64_t,Process*, ::tasking::EGenericProcessState)
							{
								if( ::tasking::waitForFutureMThread(result) == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
								{									
									if (result.getValid() )
									{
										auto val = channel.getValue() + mValueToAdd;
										if ( val != result.getValue())
											spdlog::error("Result value is not the expected one!!. Get {}, expected {}",result.getValue(),val);
									}/*else
										spdlog::error("Error waiting for output: {}",result.getError()->errorMsg);*/
								}
								mBarrier.set();	
								return ::tasking::EGenericProcessResult::KILL;
							}
						);
					}while(rand()<RAND_MAX*newTaskProb && nTasks < maxTasks);
				}
				else
				{
					nSinglethreads++;
					mConsumers[i]->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
						makeMemberEncapsulate(&MasterThread::_consumerTaskAsThread,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel,nTasks++)
					);
				}
			}
			if ( prodIdx == n)
			{
				mProducer->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
					);
			}
			spdlog::debug("{} jobs have been launched, from which {} are single threads",nTasks,nSinglethreads);

			mBarrier = Barrier(nTasks);
			for(auto th:mConsumers)
				th->resume();
            constexpr unsigned MAX_TIME = 1000;
			auto r = ::tasking::waitForBarrierMThread(mBarrier,MAX_TIME);
			if ( r != ::tasking::Event_mthread::EVENTMT_WAIT_OK )
			{
				spdlog::error("Wait for responses failed!!!!");
				this->finish();  
				return ::tasking::EGenericProcessResult::KILL; 
			}
			else if ( (msecs - mLastDebugTime) > 5000 )
				{
					mLastDebugTime = msecs;
					spdlog::info("Wait for responses ok!!");
				}
			
			/*t0 = getTimer()->getMilliseconds();
			::tasking::Process::wait(100);
			t1 = getTimer()->getMilliseconds();					
			spdlog::info("Elapsed {}",t1-t0);*/
			return ::tasking::EGenericProcessResult::CONTINUE;
		}
		::tasking::EGenericProcessResult _producerTask(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> output) 
		{			
			//generar el output como sea (tiempo aleatorio, etc..)
			//@note remember C++11 has cool functions for random numbers in <random> header
			constexpr unsigned max = 20;
            constexpr auto errProb = 0.2f;
			auto value = rand()%max;
            if ( value >= max*errProb )
            {
			    spdlog::debug("Genero valor = {}",value);
                output.setValue(value);
            }else
            {
                output.setError(0,"PRUEBA ERROR");
                spdlog::debug("Genero error");
            }			
			return ::tasking::EGenericProcessResult::KILL;
		}
		::tasking::EGenericProcessResult _consumerTask(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> input,Future<int> output,int taskId ) 
		{
			spdlog::debug("Task {} waits for input",taskId);
			auto wr = ::tasking::waitForFutureMThread(input);
			if (  wr == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
			{
				if (!input.getValid())
				{
					spdlog::debug("Task {} gets error waiting for input: {}",taskId,input.getError()->errorMsg);
					output.setError(0,"");
				}
				else
				{
					output.setValue(input.getValue() + mValueToAdd);
					spdlog::debug("Task {} gets value {}",taskId,input.getValue());
				}
                    //spdlog::debug("Thread {} gets value ",thId);
			}else
			{
				spdlog::error("Task {} gets error waiting for input",taskId);
				output.setError(0,"");
			}
			mBarrier.set();
			return ::tasking::EGenericProcessResult::KILL;
		}
		::tasking::EGenericProcessResult _consumerTaskAsThread(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> input,int taskId ) 
		{
			auto wr = ::core::waitForFutureThread(input);
			if (  wr == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
			{
				if (!input.getValid())				
					spdlog::debug("Thread {} gets error waiting for input: {}",taskId,input.getError()->errorMsg);
				else
					spdlog::debug("Thread {} gets value {}",taskId,input.getValue());
                    //spdlog::debug("Thread {} gets value ",thId);
			}else
				spdlog::error("Thread {} gets error waiting for input",taskId);
			mBarrier.set();
			return ::tasking::EGenericProcessResult::KILL;
		}
		void onThreadEnd() override{
			//finalizar el resto
            mProducer->finish();
			for(auto th:mConsumers)
			{
				th->finish();
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
/**
 * @todo 
lanzar n tareas para esperar por respuesta de consumidores para comprar valor con el generado. Por ejemploi, que los conusmidores sumen un valor ty lo dejen en otro
future. Estaría bien usar una berrera para esperar por todos, no el wait que tengo en el master.
 
 */
int test_threading::test_futures()
{
	int result = 0;
	auto producer = GenericThread::createEmptyThread();
    spdlog::set_level(spdlog::level::info); // Set global log level
	constexpr size_t n = 20;
	std::array<Thread*,n> consumers;
	for(size_t i=0;i<n;++i)
	{
		consumers[i] = GenericThread::createEmptyThread();
	}
	auto master = new MasterThread<n>(producer,consumers);
	master->start();	
	Thread::sleep(5*60*60*1000);
    spdlog::info("To finish");
	master->finish();
	master->join();
	return result;
}
