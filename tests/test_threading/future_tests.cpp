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
#include <array>

//Test with custom error
struct MyErrorInfo : public ::core::ErrorInfo
{
	MyErrorInfo(int code,string msg):ErrorInfo(code,std::move(msg))
	{
		spdlog::debug("MyErrorInfo");
	}
	MyErrorInfo(MyErrorInfo&& ei):ErrorInfo(ei.error,std::move(ei.errorMsg)){}
	MyErrorInfo(const MyErrorInfo& ei):ErrorInfo(ei.error,ei.errorMsg){}
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
};


template <size_t n>
class MasterThread : public GenericThread
{
	public:
		MasterThread(std::shared_ptr<Thread> producer,std::array<std::shared_ptr<Thread>,n> consumers,unsigned int maxTime) : 
			GenericThread(false,true),mConsumers(consumers),mProducer(producer),mMaxTime(maxTime)
		{
		}	
	private:
		typedef Future<int,MyErrorInfo> FutureType;

		std::array<std::shared_ptr<Thread>,n> mConsumers;
		std::shared_ptr<Thread> mProducer;
		int				 mValueToAdd;
		Barrier			mBarrier;
		uint64_t		mLastDebugTime; //para mosrtar mensaje de debug de que todo va bien
		uint64_t 		mStartTime;
		unsigned int	mMaxTime; //msecs to do test
		void onThreadStart() override
		{			
			auto msecs = this->getTimer()->getMilliseconds();
			mStartTime = msecs;
			srand((unsigned)msecs);
			post( ::mpl::makeMemberEncapsulate(&MasterThread::_masterTask,this));					
		}
		::tasking::EGenericProcessResult _masterTask(uint64_t msecs,Process* p) 
		{	
			constexpr auto mthreadProb = 0.7f; //probability of task being a microthread instead blocking thread
			constexpr auto newTaskProb = 0.5f; //probability of launching a new task
			constexpr auto maxTasks = 500;
			//a common future ("channel") is created so producer will put ther its value and cosumers wait for it
			FutureType channel;            
			int nSinglethreads = 0;
			auto prodIdx = rand()%(n+1);
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
					mProducer->fireAndForget(
						mpl::linkFunctor<void,TYPELIST()>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),channel)
					);
				}

				if ( rand() < (int)RAND_MAX*mthreadProb)
				{
					do
					{
						FutureType result;
						mConsumers[i]->post(
							mpl::linkFunctor<::tasking::EGenericProcessResult,TYPELIST(uint64_t,Process*)>(
							makeMemberEncapsulate(&MasterThread::_consumerTask,this),::mpl::_v1,::mpl::_v2,channel,result,nTasks++)
						);
						++nTasks; //take next task into account
						post( [this,channel,result](uint64_t,Process*)
							{
								auto wr = ::tasking::waitForFutureMThread(result);
								if ( wr.isValid())
								{
									auto val = channel.getValue().value() + mValueToAdd;
									if ( val != wr.value())
										spdlog::error("Result value is not the expected one!!. Get {}, expected {}",result.getValue().value(),val);
								}
							
								mBarrier.set();	
								return ::tasking::EGenericProcessResult::KILL;
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
			if ( prodIdx == n)
			{
				mProducer->fireAndForget(
						mpl::linkFunctor<void,TYPELIST()>(
							makeMemberEncapsulate(&MasterThread::_producerTask,this),channel)
					);
			}
			spdlog::debug("{} jobs have been launched, from which {} are single threads",nTasks,nSinglethreads);

			mBarrier = Barrier(nTasks);
			for(auto th:mConsumers)
				th->resume();
			auto t0 = getTimer()->getMilliseconds();
            constexpr unsigned MAX_TIME = 3500;
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
					auto t1 = getTimer()->getMilliseconds();
					spdlog::info("Wait for responses ok. Time waiting: {} msecs",t1-t0);
				}
			
			if ( msecs - mStartTime > mMaxTime)
			{
				spdlog::info("Maximum test time reached. Finishing");
				this->finish();  
				return ::tasking::EGenericProcessResult::KILL; 
			}else			
				return ::tasking::EGenericProcessResult::CONTINUE;
		}
		void _producerTask(FutureType output) 
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
                output.setError(MyErrorInfo(0,"PRUEBA ERROR"));
                spdlog::debug("Genero error");
            }			
		}
		::tasking::EGenericProcessResult _consumerTask(uint64_t,Process*, FutureType input,FutureType output,int taskId ) 
		{
			// int tam = rand()%1000;
			// int arr[tam];  //Uvale, qué susto, esto no es standard. Funciona en gcc y clang pero no es standard
			spdlog::debug("Task {} waits for input",taskId);
			auto wr = ::tasking::waitForFutureMThread(input);
			if ( wr.isValid() )
			{
				output.setValue(wr.value() + mValueToAdd);
				spdlog::debug("Task {} gets value {}",taskId,input.getValue().value());
			}else
			{
				spdlog::debug("Task {} gets error waiting for input: {}",taskId,input.getValue().error().errorMsg);
					output.setError( MyErrorInfo(0,""));
			}			
			mBarrier.set();
			return ::tasking::EGenericProcessResult::KILL;
		}
		void _consumerTaskAsThread(FutureType input,int taskId ) 
		{
			auto wr = ::core::waitForFutureThread(input);
			if ( wr.isValid() )
			{
				spdlog::debug("Thread {} gets value {}",taskId,wr.value());
			}else
			{
				spdlog::debug("Thread {} gets error waiting for input: {}",taskId,input.getValue().error().errorMsg);
			}		
			mBarrier.set();
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
			// for(auto th:mConsumers)
			// {
			// 	th->join();
			// }
		}
};

int test_threading::test_futures()
{
/*
triggerOnDone: ¿tiene sentido? sería para llamar a un callback desde el Runnable adecaudo->¿merece la pena? sí->
¿otro nombre?
¿cómo resuelvo lo del kill? no me gusta mucho que esa opción esté ahí
esto está relacionado con el tema de devovler error, el valor y tal. Igual podría devovler el exception?? la cuestión es que si es otro
¿merecerá la pena devolver un optional (o mejor un variant) con el valor y el error si hubiese?->
¿sería malo pasar ese variant al triggerondone?
*/
	int result = 0;
	//@todo hasta que no haga bien lo del autodestroy, esto no está bien del todo. Deberia pasar los consumidores como shared_ptr
	
	auto producer = GenericThread::createEmptyThread(true,false);
	
    spdlog::set_level(spdlog::level::debug); // Set global log level
	constexpr size_t n = 1;
	constexpr unsigned int TESTTIME = 30*60*1000;
	std::array< std::shared_ptr<Thread>,n> consumers;
	for(size_t i=0;i<n;++i)
	{
		consumers[i] = GenericThread::createEmptyThread(true,false);
	}
	auto master = make_shared<MasterThread<n>>(producer,consumers,TESTTIME);
	master->start();	
	master->join();	
	
	return result;
}
