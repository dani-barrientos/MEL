#include "future_tests.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <spdlog/spdlog.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <core/Event_mthread.h>


template<class T> ::core::FutureData_Base::EWaitResult waitForFutureMThread( const core::Future<T>& f,unsigned int msecs = ::core::Event_mthread::EVENTMT_WAIT_INFINITE)
{
	using ::core::Event_mthread;
	using ::core::FutureData_Base;

	struct _Receiver
	{		
		_Receiver():mEvent(false,false){}
		FutureData_Base::EWaitResult wait(const core::Future<T>& f,unsigned int msecs)
		{
			//@todo revisar esto, que en el Future uso la seccion critica, pero creo que si lo anterior lo gestiono bien, no hace falta
			// //@todo hacer aqui todo lo de saber el estado y tal. Puede que para eso necesite la seccion critica??			
			FutureData_Base::EWaitResult result;
			//first check if already set
			if ( f.getState() != ::core::EFutureState::NOTAVAILABLE )
			{
				spdlog::debug("Future already available");
				result = ::core::FutureData_Base::FUTURE_WAIT_OK; //means result is aviable, but doesn't men to be valid!!!
			}
			else
			{
				auto id = f.subscribeCallback(
					std::function<::core::ECallbackResult( typename ::Future<T>::ParamType)>([this](typename ::Future<T>::ParamType ) 
					{
						mEvent.set();
	//					spdlog::debug("Event was set");
						return ::core::ECallbackResult::UNSUBSCRIBE; 
					})
				);
				Event_mthread::EWaitCode eventresult;
				//spdlog::debug("Waiting for event");
				eventresult = mEvent.wait(msecs); 
				spdlog::debug("Wait was done");
				switch( eventresult )
				{
				case ::core::Event_mthread::EVENTMT_WAIT_KILL:
					//event was triggered because a kill signal
					result = ::core::FutureData_Base::EWaitResult::FUTURE_RECEIVED_KILL_SIGNAL;
					break;
				case Event_mthread::EVENTMT_WAIT_TIMEOUT:
					result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_TIMEOUT;
					break;
				default:
					result = ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK;
					break;
				}
			}
			return result;	
	
		}
		private:
		::core::Event_mthread mEvent;

	};
	auto receiver = make_unique<_Receiver>();
	auto result =  receiver->wait(f,msecs);	
	return result;
}

template<> ::core::FutureData_Base::EWaitResult waitForFutureMThread<void>( const core::Future<void>& f,unsigned int msecs)
{
	//@todo cuando esté clara la otra
	return ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK; 
}

template <size_t n>
class MyThread : public GenericThread
{
	public:
		MyThread(Thread* producer,std::array<Thread*,n> consumers) : GenericThread(false,true),mConsumers(consumers),mProducer(producer)
		{
		}	
	private:
		std::array<Thread*,n> mConsumers;
		Thread* mProducer;
		std::atomic<int> mResponses;
		void onThreadStart() override
		{
			srand((unsigned)this->getTimer()->getMilliseconds());
			post( ::mpl::makeMemberEncapsulate(&MyThread::_masterTask,this));			
			//pruebas de tareas dummy
			for(auto th:mConsumers)
			{
				// th->post( [](uint64_t t, Process*, ::tasking::EGenericProcessState)
				// {
				// 	return ::tasking::EGenericProcessResult::CONTINUE;
				// },1000000);
										
			}
		}
		::tasking::EGenericProcessResult _masterTask(uint64_t,Process*, ::tasking::EGenericProcessState) 
		{	
			//a common future ("channel") is created so producer will put ther its value and cosumers wait for it
			Future<int> channel;
			mResponses = 0;
			auto prodIdx = rand()%(n+1);
			spdlog::debug("Producer idx {} de {}",prodIdx,n);
			
			for(size_t i = 0; i < n; ++i)
			{
				if ( i == prodIdx)
				{
					mProducer->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MyThread::_producerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
					);
				}
				mConsumers[i]->post(
					mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
					makeMemberEncapsulate(&MyThread::_consumerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
				);		
			}
			if ( prodIdx == n)
			{
				mProducer->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MyThread::_producerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
					);
			}
			//Wait for all threads respond
			int value;
			do
			{
				value = mResponses.load( std::memory_order_relaxed);
				::tasking::Process::wait(100);
			} while(value<n);
			
			return ::tasking::EGenericProcessResult::CONTINUE;
		}
		::tasking::EGenericProcessResult _producerTask(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> output) 
		{			
			//generar el output como sea (tiempo aleatorio, etc..)
			//@note remember C++11 has cool functions for random numbers in <random> header
			constexpr unsigned max = 20;
			auto value = rand()%20;
			spdlog::debug("Genero valor = {}",value);
			output.setValue(value);
			return ::tasking::EGenericProcessResult::KILL;
		}
		::tasking::EGenericProcessResult _consumerTask(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> input ) 
		{
			auto wr = waitForFutureMThread(input);
			if (  wr == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
			{
				auto thId = Thread::getCurrentThread()->getThreadId();
				if (!input.getValid())				
					spdlog::error("Thread {} gets error waiting for input: {}",thId,input.getError()->errorMsg);
				else
					spdlog::debug("Thread {} gets value {}",thId,input.getValue());
			}else
				spdlog::error("Thread {} gets error waiting for input",Thread::getCurrentThread()->getThreadId());
			mResponses.fetch_add(1,::std::memory_order_relaxed);
			return ::tasking::EGenericProcessResult::KILL;
		}
		void onThreadEnd() override{
			//finalizar el resto
			for(auto th:mConsumers)
			{
				th->finish();
				//@todo qué pasa con el join? quiero esperar antes por los demas, ¿virtual?
			}
		}
		void onJoined() override
		{
			for(auto th:mConsumers)
			{
				th->join();
			}
		}
};
int  _testFutures()
{
	int result = 0;
	auto producer = GenericThread::createEmptyThread();
	constexpr size_t n = 2;
	std::array<Thread*,n> consumers;
	for(size_t i=0;i<n;++i)
	{
		consumers[i] = GenericThread::createEmptyThread();
	}
	auto master = new MyThread<n>(producer,consumers);
	master->start();	
			
	Thread::sleep(3000000);
	master->finish();
	master->join();
	return result;
}
