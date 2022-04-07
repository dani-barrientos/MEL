///////////////////////////////////////////////////////////
//  ProcessScheduler.h
//  Implementation of the Class ProcessScheduler
//  Created on:      29-mar-2005 10:00:12
///////////////////////////////////////////////////////////

#pragma once

#include <tasking/Process.h>

#include <forward_list>
using std::forward_list;
using mel::tasking::Process;

#include <utility>


#include <core/CriticalSection.h>
using mel::core::CriticalSection;

#include <core/Timer.h>
using mel::core::Timer;
#include <core/CallbackSubscriptor.h>
using mel::core::CallbackSubscriptor;
#include <mpl/Int2Type.h>
using::mel::mpl::Int2Type;
#include <atomic>
#include <memory>
#include <deque>
#include <variant>
#include <vector>
namespace mel
{
	namespace tasking
	{
		typedef CallbackSubscriptor< ::mel::core::CSMultithreadPolicy, std::shared_ptr<Process>> WakeSubscriptor;
		typedef CallbackSubscriptor< ::mel::core::CSNoMultithreadPolicy, std::shared_ptr<Process>> SleepSubscriptor;
		typedef CallbackSubscriptor< ::mel::core::CSNoMultithreadPolicy, std::shared_ptr<Process>> EvictSubscriptor;
		/**
		* Process manager. It can be seen as a task scheduler
		* @version 1.0
		* @remarks as is explained in Process, temporization is used with a Timer but getting only the 32 bit low part of the
		* 64 bits time returned, for efficiency reasons.
		*/
		class MEL_API ProcessScheduler 
		{
			typedef unsigned int ThreadID;
			friend class Process;
			//container for new tasks when using lock_Free
			class LockFreeTasksContainer
			{		
				public:	
					struct ElementType
					{
						ElementType():p(nullptr),st(0),valid(false){}
						std::shared_ptr<Process> p;
						unsigned int st;
						std::atomic<bool> valid;
					};
				private:
					typedef std::vector<ElementType> PoolType;
					std::atomic<size_t> mCurrIdx = 0;
					std::atomic<size_t> mSize;
					std::deque<PoolType> mPool;				
					size_t mChunkSize;
					size_t mMaxSize; 
					volatile bool mInvalidate;
					CriticalSection mSC;
				public:
					LockFreeTasksContainer(size_t chunkSize,size_t maxChunks );
					PoolType::value_type& operator[](size_t idx);
					void add(std::shared_ptr<Process>& process,unsigned int startTime);
					inline size_t getCurrIdx(std::memory_order mo = std::memory_order_relaxed){
						return mCurrIdx.load(mo);
					}
					void clear();
					size_t size() const{return mSize.load(std::memory_order_acquire);}  //pruebas memoryorder
					//return previous value
					size_t exchangeIdx(size_t v,std::memory_order order = std::memory_order_seq_cst);
					//void lock();
					bool isInvalidate() const{ return mInvalidate;}
					void setInvalidate(bool v){ mInvalidate = v;}
					size_t getMaxSize() const{ return mMaxSize;}

			};
			
		public:
			typedef std::pair<std::shared_ptr<Process>,unsigned int> ProcessElement;
			typedef forward_list<ProcessElement> TProcessList; //pairs of processes and starttime		
			typedef std::deque< std::pair<std::shared_ptr<Process>,unsigned int> > TNewProcesses; //pairs of processes and starttime in blocking scheduler
			

			/**
			 * @brief options for lock_free scheduler
			 * 
			 */
			struct LockFreeOptions
			{
				size_t chunkSize = 512; //!< number of task per block
				size_t maxChunks = 4; //!< maximun number of blocks to allocate when growing, before reset
		
			};
			//! @brief option for blocking scheduler
			struct BlockingOptions
			{
				//empty now
			};
			//!@brief Scheduler creation options. By default use BlockingOotions
			typedef std::variant<BlockingOptions,LockFreeOptions> SchedulerOptions;
			/**
			 * @brief Constructor
			 * 
			 * @param initialPoolSize  initial size of pool tasks, that will grow if neccesary. if > maxNewTasks, is assigned same value (only for lock-free mode)
			 * @param maxNewTasks maximum task to grow the pool. When this size is overcame, the buffer is reseted.(only for lock-free mode)
			 */
			ProcessScheduler( SchedulerOptions opts);
			~ProcessScheduler(void);

			/**
			* insert new process to be executed. Execution order doesn't necessarily respect the post order
			*
			* @param process insert new process in scheduler
			* @param starttime msecs to wait to start it
			* @warning 	if process is already inserted behaviour is unpredictable
			*/
			void insertProcess( std::shared_ptr<Process> process,unsigned int startTime = 0);
			/**
			 * @brief Insert new process in scheduler without locking processlist
			 * @details Use this function very carefully, when you know that no other thread is inserting processes or executing it
			 * 
			 * @param process 
			 * @param startTime 
			 */
			void insertProcessNoLock( std::shared_ptr<Process> process,unsigned int startTime = 0);


			/**
			*  gets how many process in scheduler
			*/
			inline unsigned int getProcessCount() const;
			/*
			* get number of processes running (not sleeped)
			*/
			inline unsigned int getActiveProcessCount() const;
			/**
			* =============================================================================
			*/
			void executeProcesses();
			/**
			* ! pausa todos los procesos
			*/
			void pauseProcesses();
			/**
			* Pauses proceses matching the given predicate
			*
			* @param predicate predicate
			* @todo no me gusta, hay que generalizarlo mejor. Como obtener los procesos que cunplen algo y actuar sobre ellos
			*/
			template <class T>
			void pauseProcesses(T& predicate);

			
			/**
			* =============================================================================
			*/
			void activateProcesses();
			/**
			* send processes kill signal. It doesn�t kill inmediately, it's done at execution cycle begin, so
			* you need to continue processing until no more process are pending (getProcessCount() == 0)
			* @param[in] deferred If true, then kill will be posted as a new process. If False, it's done inmediately.
			* First type is when done into Scheduler context and second form is when done from another thread
			@todo no me gusta
			*/
			void killProcesses( bool deferred );

			/**
			* Checks for a given task to be finished
			* @param[in] taskId the task to check
			* @return true if the task has already been executed. false otherwise
			* @warning  this function is not thread-safe. If you need it you
			*  must block executeprocess() and checkFor() with a critical section
			*/
			//inline bool checkFor(unsigned int taskId);

			inline TProcessList& getProcesses();
	//		inline TNewProcesses& getPendingProcesses();
			/**
			* remove all processes without taking care
			* @remarks for use with caution
			*/
			void destroyAllProcesses();

			/**
			* set Timer to use.
			*/
			void setTimer(std::shared_ptr<Timer> timer );
			/**
			* get used Timer
			*/
			inline const std::shared_ptr<Timer> getTimer() const;
			inline std::shared_ptr<Timer> getTimer() ;

			/**
			* get current executing process in currenexecutiont thread
			*/
			static std::shared_ptr<Process> getCurrentProcess();
			
			/**
			* subscribe to process sleep event
			*/
			template <class F> int susbcribeSleepEvent(F&& f)
			{
				return mSS.subscribeCallback(std::forward<F>(f));
			}
			template <class F> void unsusbcribeSleepEvent(F&& f)
			{
				mSS.unsubscribeCallback(std::forward<F>(f));
			}
			void unsusbcribeSleepEvent(int id)
			{
				mSS.unsubscribeCallback(id);
			}
			template <class F> int susbcribeWakeEvent(F&& f)
			{
				return mWS.subscribeCallback(std::forward<F>(f));
			}
			template <class F> void unsusbcribeWakeEvent(F&& f)
			{
				mWS.unsubscribeCallback(std::forward<F>(f));
			}
			template <class F> int subscribeProcessEvicted(F&& f)
			{
				return mES.subscribeCallback(std::forward<F>(f));
			}
			template <class F> void unsubscribeProcessEvicted(F&& f)
			{
				mES.unsubscribeCallback(std::forward<F>(f));
			}
			void unsubscribeProcessEvicted(int id)
			{
				mES.unsubscribeCallback(id);
			}
		private:				
			SchedulerOptions mOpts;
			bool mIsLockFree; //same as checking index in mOpts but for performance
			struct ProcessInfo  //for TLS
			{
				std::shared_ptr<Process> current;
			};
			ProcessInfo*	mProcessInfo;
			TProcessList mProcessList;
			std::unique_ptr<LockFreeTasksContainer> mLockFreeTasks;
			//new processes to insert next time blocking variant
			TNewProcesses  mBlockingTasks;
			size_t mLastIdx;	
			mutable CriticalSection	mCS;		
			std::shared_ptr<Timer>			mTimer;
			std::atomic<unsigned int> mProcessCount;
			std::atomic<int32_t>		mInactiveProcessCount;
			bool					mKillingProcess; //flag to mark when ther is a kill task pending
			#ifndef NDEBUG
			void* _stack = nullptr;
			#endif
			WakeSubscriptor mWS;
			SleepSubscriptor mSS;
			EvictSubscriptor mES;

			/**
			* helper function
			*/
			void _executeProcesses(uint64_t time,TProcessList& processes) OPTIMIZE_FLAGS;

			void _killTasks();
			void _triggerSleepEvents(std::shared_ptr<Process> p);
			void _triggerWakeEvents(std::shared_ptr<Process> p);
			static ProcessInfo* _getCurrentProcessInfo();
			/**
			* internal method intended to be used by Process to notify was awakened
			* @todo maybe this will be deprecated in favor of another mechanism (CallbackSubscriptor, etc)
			*/
			void processAwakened(std::shared_ptr<Process>);
			/**
			* internal method intended to be used by Process to notify was asleep
			* @todo maybe this will be deprecated in favor of another mechanism (CallbackSubscriptor, etc)
			*/
			void processAsleep(std::shared_ptr<Process>);
		};
		

		template <class T>
		void ProcessScheduler::pauseProcesses(T& predicate)
		{
			for(auto i = mProcessList.begin() ; i != mProcessList.end(); ++i)
			{
				if ( predicate( *i ) )
				{
					(*i).first->pause();
				}
			}
		}
		ProcessScheduler::TProcessList& ProcessScheduler::getProcesses() 
		{
			return mProcessList;
		}
		/*ProcessScheduler::TNewProcesses& ProcessScheduler::getPendingProcesses()
		{
			return mNewProcesses;
		}*/

		unsigned int ProcessScheduler::getProcessCount() const
		{

			return mProcessCount;
		}
		unsigned int ProcessScheduler::getActiveProcessCount() const
		{
			return getProcessCount()-((unsigned int)mInactiveProcessCount);
		}

		const std::shared_ptr<Timer> ProcessScheduler::getTimer() const
		{
			return mTimer;
		}
		std::shared_ptr<Timer> ProcessScheduler::getTimer()
		{
			return mTimer;
		}

	}
}
