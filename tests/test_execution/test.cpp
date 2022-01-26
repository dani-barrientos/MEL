#include "test.h"
#include <core/GenericThread.h>
using core::GenericThread;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <spdlog/spdlog.h>
#include <CommandLine.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/Process.h>
using tasking::Process;



//pruebas---
template <class ExecutorAgent> class Executor;
template <class ExecutorType> class Continuation;
template <class ExecutorType> class ContinuationData;
//sería clase genérica que usa cualquier agente de ejecución->probar uno con threadPool y que distribuya tareas
template <> class Executor<Runnable>
{
	friend class ContinuationData<Executor<Runnable>>;
	public:
		Executor(std::shared_ptr<Runnable> runnable):mRunnable(runnable)
		{

		};
		template <class F> Continuation<Executor<Runnable>> launch( F&& f);
	private:
		std::weak_ptr<Runnable> mRunnable; //@todo weak_ptr??al final este necesita de un shared_ptr, por tanto
		template <class F> void _execute(F&& f)
		{
			mRunnable.lock()->fireAndForget(std::forward<F>(f));
		}
};

template <class ExecutorType> class ContinuationData final : public enable_shared_from_this<ContinuationData<ExecutorType>>
{
	typedef enable_shared_from_this<ContinuationData<ExecutorType>> Base;
	public:
	//	static std::atomic<int> counter; //para pruebas
		template <class F> ContinuationData(ExecutorType ex,F&& f ):mState(EState::NONE), mExecutor(std::move(ex)),mFunct(std::forward<F>(f)),mNext(nullptr)
		{			
	//		++counter;
		}
		~ContinuationData()
		{
	//		spdlog::debug("~ContinuationData()");
	//		--counter;
		}
		template <class F> Continuation<ExecutorType> next(F&& f);		
		void start()
		{
			mExecutor._execute(
				[_this = Base::shared_from_this()]() mutable
				{					
					::core::Lock lck(_this->mCS);				
					_this->mFunct();
					_this->mState = EState::DONE;
					if ( _this->mNext )
						_this->mNext->start();
				}
			);
		}
	private:	
		enum class EState :uint8_t{NONE,DOING,DONE};
		CriticalSection mCS;
		ExecutorType 	mExecutor;
		EState 			mState;		
		std::shared_ptr<ContinuationData> mNext;
		std::function<void()> mFunct;
	
};
//std::atomic<int> ContinuationData::counter = 0;
template <class ExecutorType> class Continuation
{
	friend ExecutorType;
	public:
		typedef ContinuationData<ExecutorType> DataType;
		template <class F> Continuation(ExecutorType ex,F&& f ):mData(make_shared<DataType>(ex,std::forward<F>(f))){}		
		template <class F> Continuation<ExecutorType> next(F&& f)
		{
			return mData->next(std::forward<F>(f));
		}
		Continuation(shared_ptr<DataType> data):mData(data){}  //@todo debe ser privado pero algo no me va bien con el friend
		Continuation(Continuation&& cont):mData(std::move(cont.mData)){}
		Continuation(const Continuation& cont):mData(cont.mData){}
	private:
		void _start(){ if(mData) mData->start();}
		shared_ptr<DataType> mData;

};
template <class ExecutorType> template <class F> Continuation<ExecutorType> ContinuationData<ExecutorType>::next(F&& f)
{
	::core::Lock lck(mCS);
	if ( mState== EState::DONE)
	{
		//spdlog::debug("next. Straight execution");
		//straight execution 
		return mExecutor.launch(f);
	}else
	{
		//spdlog::debug("next. Not available");
		mNext = make_shared<typename Continuation<ExecutorType>::DataType>(mExecutor,std::forward<F>(f));
		return Continuation(mNext); 
	}
}	
template <class F> Continuation<Executor<Runnable>> Executor<Runnable>::launch( F&& f )
{
	Continuation<Executor<Runnable>> result(*this,std::forward<F>(f));
	result._start();
	return result;
}	

/**
 * @brief execution tests
 * commandline options
????
 * @return int 
 */
static int test()
{
	int result = 0;
	auto th1 = GenericThread::createEmptyThread(false);
	//th1->start();	
	{
//---- pruebas executors
	Executor<Runnable> ex(th1);
	auto cont = ex.launch(
		[]()
		{
			spdlog::debug("UNO");
			::tasking::Process::wait(4000);
			spdlog::debug("DOS");
		}
	).next([]()
		{
			spdlog::debug("TRES");
			::tasking::Process::wait(1000);
			spdlog::debug("CUATRO");
		}
	);
//	spdlog::debug("Remaining Data 1: {}",ContinuationData::counter);
	Thread::sleep(3000);
//	spdlog::debug("Remaining Data 2: {}",ContinuationData::counter);
	cont.next([]()
		{
			spdlog::debug("CINCO");
		}
	);	
	}
//	spdlog::debug("Remaining Data 3: {}",ContinuationData::counter);	
	th1->start();		
	Thread::sleep(60000);
	spdlog::debug("finish");
	th1->finish();
	th1->join();
	
	return result;
}
void test_execution::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"execution tests:\n - 0 = mono thread;\n - 1 = performance launching a bunch of tasks",test);
}
