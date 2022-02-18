#include "future_tests.h"
#include <core/ThreadRunnable.h>
using core::ThreadRunnable;
using namespace std;
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
		#endif
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
		#ifdef USE_SPDLOG
		spdlog::debug("MyErrorInfo");
		#endif
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
class MasterThread : public ThreadRunnable
{
	public:
		MasterThread(std::shared_ptr<ThreadRunnable> producer,std::array<std::shared_ptr<ThreadRunnable>,n> consumers,unsigned int maxTime) : 
			ThreadRunnable(true),mConsumers(consumers),mProducer(producer),mMaxTime(maxTime)
		{
		}	
	private:
		typedef Future<int,MyErrorInfo> FutureType;

		std::array<std::shared_ptr<ThreadRunnable>,n> mConsumers;
		std::shared_ptr<ThreadRunnable> mProducer;
		int				 mValueToAdd;
		Barrier			mBarrier;
		uint64_t		mLastDebugTime; //para mosrtar mensaje de debug de que todo va bien
		uint64_t 		mStartTime;
		unsigned int	mMaxTime; //msecs to do test
		void onStart() override
		{
			auto msecs = this->getTimer()->getMilliseconds();
			mStartTime = msecs;
			srand((unsigned)msecs);
			post( ::mpl::makeMemberEncapsulate(&MasterThread::_masterTask,this));					
		}
		::tasking::EGenericProcessResult _masterTask(uint64_t msecs,Process* p) 
		{	
			constexpr auto mthreadProb = 0.7f; //probability of task being a microthread instead blocking thread
			//constexpr auto mthreadProb = 1.0f; //probability of task being a microthread instead blocking thread
			constexpr auto newTaskProb = 0.5f; //probability of launching a new task
			//constexpr auto newTaskProb = 1.0f; //probability of launching a new task
			constexpr auto maxTasks = 500;
			//constexpr auto maxTasks = 1;
			//a common future ("channel") is created so producer will put ther its value and cosumers wait for it
			FutureType channel;            
			int nSinglethreads = 0;
			auto prodIdx = rand()%(n+1);
			mValueToAdd = rand()%50;  //value to add to input in consumers
			#ifdef USE_SPDLOG
			spdlog::debug("Producer idx {} de {}. Consumers must add {} to their input",prodIdx,n,mValueToAdd);
		#endif
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
						spdlog::debug("launch task {}",nTasks);
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
									#ifdef USE_SPDLOG
									if ( val != wr.value())
										spdlog::error("Result value is not the expected one!!. Get {}, expected {}",result.getValue().value(),val);
									#endif
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
		#ifdef USE_SPDLOG
			spdlog::debug("{} jobs have been launched, from which {} are single threads",nTasks,nSinglethreads);
		#endif

			mBarrier = Barrier(nTasks);
			for(auto th:mConsumers)
				th->resume();
			auto t0 = getTimer()->getMilliseconds();
            constexpr unsigned MAX_TIME = 5500;
			auto r = ::tasking::waitForBarrierMThread(mBarrier,MAX_TIME);
			if ( r != ::tasking::Event_mthread::EVENTMT_WAIT_OK )
			{
			#ifdef USE_SPDLOG
				spdlog::error("Wait for responses failed!!!!. {} workers remaining",mBarrier.getActiveWorkers());
		#endif
				this->finish();  
				return ::tasking::EGenericProcessResult::KILL; 
			}
			else if ( (msecs - mLastDebugTime) >= 5000 )
				{
					mLastDebugTime = msecs;
					auto t1 = getTimer()->getMilliseconds();
				#ifdef USE_SPDLOG
					spdlog::info("Wait for responses ok. Time waiting: {} msecs",t1-t0);
				#endif
				}
			
			if ( msecs - mStartTime > mMaxTime)
			{
			#ifdef USE_SPDLOG
				spdlog::info("Maximum test time reached. Finishing");
			#endif
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
            constexpr auto errProb = 0.2;
			//constexpr auto errProb = 0.0;
			auto value = rand()%max;
			Process::wait(200); //pruebas
            if ( value >= max*errProb )
            {
			#ifdef USE_SPDLOG
			    spdlog::debug("Genero valor = {}",value);
			#endif
                output.setValue(value);
            }else
            {
                output.setError(MyErrorInfo(0,"PRUEBA ERROR"));
			#ifdef USE_SPDLOG
                spdlog::debug("Genero error");
			#endif
            }			
		}
		::tasking::EGenericProcessResult _consumerTask(uint64_t,Process*, FutureType input,FutureType output,int taskId ) 
		{
			// int tam = rand()%1000;
			// int arr[tam];  //Uvale, qué susto, esto no es standard. Funciona en gcc y clang pero no es standard
		#ifdef USE_SPDLOG
			spdlog::debug("Task {} waits for input",taskId);
		#endif
			auto wr = ::tasking::waitForFutureMThread(input);
			if ( wr.isValid() )
			{
				output.setValue(wr.value() + mValueToAdd);
				#ifdef USE_SPDLOG
				spdlog::debug("Task {} gets value {}",taskId,input.getValue().value());
				#endif
			}else
			{
			#ifdef USE_SPDLOG
				spdlog::debug("Task {} gets error waiting for input: {}",taskId,input.getValue().error().errorMsg);
			#endif
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
				#ifdef USE_SPDLOG
				spdlog::debug("Thread {} gets value {}",taskId,wr.value());
		#endif
			}else
			{
				#ifdef USE_SPDLOG
				spdlog::debug("Thread {} gets error waiting for input: {}",taskId,input.getValue().error().errorMsg);
		#endif
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
			for(auto th:mConsumers)
			{
				th->join();
			}
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
	
	auto producer = ThreadRunnable::create(true);
	
	#ifdef USE_SPDLOG
    spdlog::set_level(spdlog::level::info); // Set global log level
		#endif
	constexpr size_t n = 10;
	constexpr unsigned int TESTTIME = 30*60*1000;
	std::array< std::shared_ptr<ThreadRunnable>,n> consumers;
	for(size_t i=0;i<n;++i)
	{
		consumers[i] = ThreadRunnable::create(true);
	}
	auto master = make_shared<MasterThread<n>>(producer,consumers,TESTTIME);
	master->start();	
	master->join();	
	
	return result;
}
