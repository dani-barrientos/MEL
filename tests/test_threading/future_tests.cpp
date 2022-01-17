#include "future_tests.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <spdlog/spdlog.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/utilities.h>


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
		int				 mValueToAdd;
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
		::tasking::EGenericProcessResult _masterTask(uint64_t,Process*, ::tasking::EGenericProcessState) 
		{	
			constexpr auto mthreadProb = 0.7f; //probability of task being a microthread instead blocking thread
			constexpr auto newTaskProb = 0.1f; //probability of launching a new task
			//a common future ("channel") is created so producer will put ther its value and cosumers wait for it
			Future<int> channel;
            //Future<void> channel;
			mResponses = 0;			
			int nSinglethreads = 0;
			auto prodIdx = rand()%(n+1);
			mValueToAdd = rand()%50;  //value to add to input in consumers
						spdlog::debug("Producer idx {} de {}. Consumers must add {} to their input",prodIdx,n,mValueToAdd);
			int nTasks = 0;

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
						post( [this,channel,result](uint64_t,Process*, ::tasking::EGenericProcessState)
							{
								if( ::tasking::waitForFutureMThread(result) == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
								{
									if (result.getValid() )
									{
										auto val = channel.getValue() + mValueToAdd;
										if ( val != result.getValue())
											spdlog::error("Result value is not the expected one!!");
									}
								}
								
								return ::tasking::EGenericProcessResult::KILL;
							}
						);
					}while(rand()<RAND_MAX*newTaskProb);
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
			//@todo usar barrera
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
						
			spdlog::debug("Wait for responses ok!!");
			
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
					spdlog::info("Task {} gets error waiting for input: {}",taskId,input.getError()->errorMsg);
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
			mResponses.fetch_add(1,::std::memory_order_relaxed);
			return ::tasking::EGenericProcessResult::KILL;
		}
		::tasking::EGenericProcessResult _consumerTaskAsThread(uint64_t,Process*, ::tasking::EGenericProcessState,Future<int> input,int taskId ) 
		{
			auto wr = ::core::waitForFutureThread(input);
			if (  wr == ::core::FutureData_Base::EWaitResult::FUTURE_WAIT_OK )
			{
				if (!input.getValid())				
					spdlog::info("Thread {} gets error waiting for input: {}",taskId,input.getError()->errorMsg);
				else
					spdlog::debug("Thread {} gets value {}",taskId,input.getValue());
                    //spdlog::debug("Thread {} gets value ",thId);
			}else
				spdlog::error("Thread {} gets error waiting for input",taskId);
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
/**
 * @todo 
lanzar n tareas para esperar por respuesta de consumidores para comprar valor con el generado. Por ejemploi, que los conusmidores sumen un valor ty lo dejen en otro
future. Estaría bien usar una berrera para esperar por todos, no el wait que tengo en el master.
 
 */
int test_threading::test_futures()
{
	int result = 0;
	auto producer = GenericThread::createEmptyThread();
    //spdlog::set_level(spdlog::level::warn); // Set global log level
	constexpr size_t n = 20;
	std::array<Thread*,n> consumers;
	for(size_t i=0;i<n;++i)
	{
		consumers[i] = GenericThread::createEmptyThread();
	}
	auto master = new MasterThread<n>(producer,consumers);
	master->start();	
	Thread::sleep(5*60*60);
    spdlog::info("To finish");
	master->finish();
	master->join();
	return result;
}
