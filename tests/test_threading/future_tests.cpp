#include "future_tests.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <spdlog/spdlog.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <core/Event_mthread.h>


template<class T> ::core::FutureData_Base::EWaitResult waitForFutureMThread( int threadid,const core::Future<T>& f,unsigned int msecs = ::core::Event_mthread::EVENTMT_WAIT_INFINITE)
{
	using ::core::Event_mthread;
	using ::core::FutureData_Base;

	struct _Receiver
	{		
		_Receiver():mEvent(false,false){}
		FutureData_Base::EWaitResult wait(int threadid,const core::Future<T>& f,unsigned int msecs)
		{
            FutureData_Base::EWaitResult result;            
            Event_mthread::EWaitCode eventresult;
           // spdlog::debug("Waiting for event in Thread {}",threadid);
            eventresult = mEvent.waitAndDo([this,f,threadid]()
            {
             //   spdlog::debug("waitAndDo was done for Thread {}",threadid);
                f.subscribeCallback(
                std::function<::core::ECallbackResult( typename ::Future<T>::ParamType)>([this,threadid](typename ::Future<T>::ParamType ) 
                {
                    mEvent.set();
                 //   spdlog::debug("Event was set for Thread {}",threadid);
                    return ::core::ECallbackResult::UNSUBSCRIBE; 
                }));
            },msecs); 
          //  spdlog::debug("Wait was done in Thread {}",threadid);
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
			return result;	
	
		}
		private:
		::core::Event_mthread mEvent;

	};
	auto receiver = make_unique<_Receiver>();
	auto result =  receiver->wait(threadid,f,msecs);	
	return result;
}

template<> ::core::FutureData_Base::EWaitResult waitForFutureMThread<void>( int thId,const core::Future<void>& f,unsigned int msecs)
{
	//@todo cuando esté clara la otra
	return ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK; 
}

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
		std::atomic<int> mResponses;
		void onThreadStart() override
		{
			srand((unsigned)this->getTimer()->getMilliseconds());
			post( ::mpl::makeMemberEncapsulate(&MasterThread::_masterTask,this));			
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
            spdlog:
			
			for(size_t i = 0; i < n; ++i)
			{
				if ( i == prodIdx)
				{
					mProducer->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
					);
				}
				mConsumers[i]->post(
					mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
					makeMemberEncapsulate(&MasterThread::_consumerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel,i)
				);		
			}
			if ( prodIdx == n)
			{
				mProducer->post(
						mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*,::tasking::EGenericProcessState)>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),::mpl::_v1,::mpl::_v2,::mpl::_v3,channel)
					);
			}
			//Wait for all threads respond
			int value;
            unsigned time = 0;
            constexpr unsigned MAX_TIME = 2000;
			do
			{
				value = mResponses.load( std::memory_order_relaxed);
				::tasking::Process::wait(50);
                time+=50;
                if ( time >= MAX_TIME)
                {
                    //test error
                    spdlog::error("Time waiting for consumers response exceeeded maximum time!!");
                    this->finish();
                    return ::tasking::EGenericProcessResult::KILL; 
                }
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
		::tasking::EGenericProcessResult _consumerTask(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> input,int thId ) 
		{
			auto wr = waitForFutureMThread(thId,input);
			if (  wr == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
			{
//				auto thId = Thread::getCurrentThread()->getThreadId();
				if (!input.getValid())				
					spdlog::error("Thread {} gets error waiting for input: {}",thId,input.getError()->errorMsg);
				else
					spdlog::debug("Thread {} gets value {}",thId,input.getValue());
			}else
				spdlog::error("Thread {} gets error waiting for input",thId);
			mResponses.fetch_add(1,::std::memory_order_relaxed);
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
int test_threading::test_futures()
{
	int result = 0;
	auto producer = GenericThread::createEmptyThread();
    spdlog::set_level(spdlog::level::warn); // Set global log level to err
	constexpr size_t n = 10;
	std::array<Thread*,n> consumers;
	for(size_t i=0;i<n;++i)
	{
		consumers[i] = GenericThread::createEmptyThread();
	}
	auto master = new MasterThread<n>(producer,consumers);
	master->start();	
			
	Thread::sleep(3000000);
	master->finish();
	master->join();
	return result;
}
