#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <MelLibType.h>

#include <mpl/binary.h>
using mel::mpl::binary;


#include <core/Callback.h>
using mel::core::Callback;
#include <stdint.h>
#include <memory>
#include <mpl/Tuple.h>

#if defined(MEL_IOS) || defined(MEL_MACOSX)
#include <TargetConditionals.h>
#endif
#if defined(MEL_X86_GCC) || TARGET_CPU_X86 ||  defined(MEL_X86_MSVC)
#include <tasking/Process_X86.h>
#elif TARGET_CPU_X86_64
    #include <tasking/Process_X64_MAC.h>
#elif TARGET_CPU_ARM64
    #include <tasking/Process_ARM64_IPhone.h>
#elif defined (_ARM_GCC) && (!defined(MEL_IOS))
#include <tasking/Process_ARM_GCC.h>
#elif defined(MEL_IOS)
	#if !TARGET_IPHONE_SIMULATOR
        #if defined(__arm__)
            #include <tasking/Process_ARM_IPhone.h>
        #elif defined(__arm64__)
            #include <tasking/Process_ARM64_IPhone.h>
            #else
                #pragma message "Unknown ARM architecture"
        #endif
	#else
		#include <core/Process_X86.h>
	#endif
#elif defined(MEL_EMSCRIPTEN)
	#include <tasking/Process_emsdk.h>
#elif defined(MEL_ANDROID)
	#include <tasking/Process_ARM_Android.h>
#elif defined(MEL_X64_GCC) || defined(MEL_X64_CLANG)
	#include <tasking/Process_X64_GCC.h>
#elif defined (MEL_WINDOWS) && defined (MEL_X64_MSVC)
hola 3
	#include <tasking/Process_X64_MSVC.h>
#endif
#if defined(MEL_IOS) || defined(MEL_MACOSX) 
    #define OPTIMIZE_FLAGS __attribute__ ((optnone)) 
#elif defined(MEL_EMSCRIPTEN) 
	#define OPTIMIZE_FLAGS
#elif defined(MEL_ANDROID) 
	#define OPTIMIZE_FLAGS
#elif defined(_ARM_GCC) || defined(MEL_X86_GCC) || defined(MEL_X64_GCC) 
	#define OPTIMIZE_FLAGS __attribute__ ((optimize(0)))
#elif defined(MEL_X64_CLANG)
	#define OPTIMIZE_FLAGS __attribute__ ((optnone)) 
#elif defined(MEL_WINDOWS)
	#define OPTIMIZE_FLAGS	
#endif
namespace mel
{
	/**
	 * @brief Tasking system
	 * @details Based on the concept of *microthread*, which is represented by class Process. A microthread alows to have cooperative multitasking, so that a single
	 * thread can execute thousands concurrent (in a *cooperative* sense ) tasks. Think microthread as a very light fiber
	*/
	namespace tasking
	{
		class ProcessScheduler;  //predeclaration
		/**
		* @class Process
		* @brief A periodic task, implementing a *microthread*.
		* @details Once the process is inserted in a \ref ::mel::tasking::ProcessScheduler (usually done through a Runnable via ::mel::tasking::Runnable::post or ::mel::tasking::Runnable::execute) 
		* the update() (private) member function will be called. because this class is **abstract**, a concrete %Process must implement the function onUpdate function to code its custom behaviour.
		* The task will be executed as time intervals, given by the period (\ref getPeriod) until is killed (see \ref kill)
		* @remarks  Tasks are executed according to a \ref ::mel::core::Timer "Timer", which uses uint64_t type to express msecs, but for eficiency reasons
		* using a 64 bits type on 32 bits machines will be very "agressive", so we use unsigned int as time for tasks so getting the low part
		* of the original 64 bit time and hoping it will be enough...
		* @remarks Switching a Process inside a try/catch context doesn't fit very well with exception handling for this context. This issue is being addressed... If this fact is carefully considered when doing "switchs"
		* it will not be a problem, but in Windows (maybe other Operating Systems?) there is a exploitation prevent system (called SEHOP, see https://blogs.technet.microsoft.com/srd/2009/02/02/preventing-the-exploitation-of-structured-exception-handler-seh-overwrites-with-sehop/) )that will make the app crash
		* because Windows interpret it as a hack process. This option is disabled in worksations but enabled for Windows Server- To disable it, go to HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/Session Manager/Kernel/DisableExceptionChainValidation
		*/
		class MEL_API Process :
		///@cond HIDDEN_SYMBOLS
			public std::enable_shared_from_this<Process>,
			public MThreadAttributtes		
		///@endcond
		{
		
			//!should be implemented in platform-dependent code
			static void _switchProcess( ) OPTIMIZE_FLAGS ;
		public:		
			enum EProcessState : uint8_t
			{			
				PREPARED, //!< Process created but not executed
				INITIATED , //!< executing process normally
				ASLEEP, //!< sleeping. Waiting for a wakeup
				PREPARED_TO_DIE, //!< it's going to die, but process manager doesn't discard it yet
				TRYING_TO_KILL, //!< sending kill signal but no accepted yet
				KILLING_WAITING_FOR_SCHEDULED , //!< switched and no shceduled yet
				DEAD  //!< process is out of process manager
			};
			//! Reason why the process returns from context switch
			enum class ESwitchResult{
				ESWITCH_OK,  //!< return from context switch was ok
				ESWITCH_WAKEUP,  //!<return from context switch was because a wakeup
				ESWITCH_ERROR,  //!<switch couldn't be done
				ESWITCH_KILL //!<return from context switch because a kill
				};
			
			/**
			* @brief constructor
			* @param capacity Initial stack capacity,in bytes. A value of 0 means no stack precreated. In any case, stack grows as needed
			*/
			Process( unsigned short capacity = 0 );
			virtual ~Process(void);
			/**
			* @brief Sets ProcessScheduler which holds this process. Each process can only be scheduled by one ProcessScheduler
			*/
			void setProcessScheduler( ProcessScheduler* const  mgr);
			/**
			 * @brief pause execution until wakeup called
			 * @ref onPause is called in order children can do custom behaviour
			 */
			void pause( );
			
			/**
			* @brief mark this process to be eliminated by the process manager.
			* Internally, kill calls virtual onKill, which returns true if kill can be acomplished or not.
			* In case of true, then process is put in PREPARED_TO_DIE state, so the scheduler can remove it.
			* In case of false, process is put in a TRYING_TO_KILL state so scheduler will continue to trying to
			* kill it in next iterations.
			* You can know if a process is die checking getDead
			* If no Smart pointers link to the process, it is deleted
			* @remarks if process is not already inited (state == PREPARED) then onKill is not called @todo CAMBIAR ESTO?
			*
			* @param[in] force if true, then killing is without regarding previous explained process, so
			* Process go inmediately to PREPARED_TO_DIE
			*
			* @remarks not multithread safe, kill should be done in the owner thread context (se abordar� en el futuro)
			*/
			void kill( bool force = false );
			/**
			* @brief Sets process in init state
			*/
			virtual void reset();
			/**
			* @brief Set the period for this process
			* @param value the new period (in msecs)
			*/
			inline void setPeriod(unsigned int value);

			EProcessState getState() const{ return mState;}
			/**
			* check if process is dead
			*/
			inline bool getDead() const;
			/**
			* check if process is going to die
			*/
			
			//inline bool getPreparedToDie() const;
			//! get period (milliseconds)
			inline unsigned int getPeriod() const;						
			/**
			* returns time elapsed during this iteration 
			*/
			unsigned int getElapsedTime() const;
			/**
			* @return time when task was executed in last iteration, when onUpdate is called: 
			* @note context switches or waits inside code doesn't modify update time.
			*/
			inline uint64_t getLastUpdateTime() const;
			inline void resetTime();
			

			/**
			* @brief  Gets \ref ProcessScheduler which holds this Process
			*/
			inline ProcessScheduler *const getProcessScheduler() const;

			/**
			* @return true if process received kill signal
			* @see switchProcess for comments
			*/
			static ESwitchResult wait( unsigned int msegs ) OPTIMIZE_FLAGS; 
			/**
			 * @brief Evict current process and will continue executing from the same point the next time is scheduled
			 * @details this function is a static one because, in the same way threads work, is the *currently executing process* which can be evicted
			 * in any point of the execution call stack. 
			 * @param continueInmediately if true, the process will continue executing in the next shceduler cycle, so without respecting the period.
			 */		
			static ESwitchResult switchProcess( bool continueInmediately ) OPTIMIZE_FLAGS;
			/**
			* @brief Sleep current process and execute given callable. To reactivate to you must use \ref wakeUp
			* @param[in] postSleep functor (signature void(void) ) to execute just in the moment when Process go to sleep
			* @return resulting state of these operation
			* @see sleep
			* @remarks not multithread-safe
			*/
			template <class F>
			static ESwitchResult sleepAndDo( F postSleep );
			/**
			 * @brief Wait for a given time and execute given callable
			* @param[in] postWait functor (signature <void,void>) to execute just in the moment when Process go to sleep
			* @return resulting state of the operation
			* @see sleep
			* @remarks not multithread-safe
			*/		
			template <class F>
			static ESwitchResult waitAndDo( unsigned int msegs,F postWait ) OPTIMIZE_FLAGS;
			/**
			* @brief pause **current process**. To reactivate you must use wakeUp
			* @return true if process received kill signal
			* @see switchProcess for comments
			*/
			static ESwitchResult sleep( ) OPTIMIZE_FLAGS;
			inline bool getAsleep() const;
			/**
			 * @brief wakeup an asleep process or an evicted process (that process having called swtich or wait)
			 */
			void wakeUp();
			//@todo puesta publica para probar llamada desde JS
			void _execute(uint64_t msegs) OPTIMIZE_FLAGS;
		private:
			//static ESwitchResult _sleep(  Callback<void,void>* ) OPTIMIZE_FLAGS;
			static mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)> _preSleep() OPTIMIZE_FLAGS;
			static ESwitchResult _postSleep(mel::mpl::Tuple<TYPELIST(int,Process*,unsigned int)>) OPTIMIZE_FLAGS;
			//static ESwitchResult _wait( unsigned int msegs, Callback<void,void>* ) OPTIMIZE_FLAGS;
			static mel::mpl::Tuple<TYPELIST(bool,Process*)> _preWait() OPTIMIZE_FLAGS;
			static ESwitchResult _postWait(uint64_t msegs,mel::mpl::Tuple<TYPELIST(bool,Process*)>) OPTIMIZE_FLAGS;
			EProcessState mState;
			EProcessState mPreviousState;
			volatile bool mWakeup; //temp value to know if context switch comes from a wakeup
			bool 	mPauseReq;
			unsigned int mPeriod;
			uint64_t mLastUpdateTime; //!<time at process execution
			ProcessScheduler* mOwnerProcessScheduler; //!<scheduler in which is inserted
			/**
			* main execution block
			* @param msegs    msegs
			*/
			//void _execute(uint64_t msegs) OPTIMIZE_FLAGS;
			volatile void checkMicrothread(uint64_t msegs ) OPTIMIZE_FLAGS;
			/**
			* execution function. It calls update() when time > mPeriod
			*
			* @param msegs    msegs
			*/
			void update(uint64_t msegs) OPTIMIZE_FLAGS;
			/**
			* this is intended to be used by ProcessScheduler
			*/
			inline void setDead();
		protected:
			/**
			* behaviour function. The main function of a process. MUST be implemented in
			* childs. It's called automatically when elapsed time >= mPeriod
			*
			* @param msecs milliseconds
			*/
			virtual void onUpdate(uint64_t msecs) = 0;
			/**
			* called when process inits
			*
			* @param msecs milliseconds
			*/
			virtual void onInit(uint64_t msecs){};
			/**
			* called when process is going to be got out of scheduler and was inited (not in PREPARED state)
			* can be overridden by children
			* @return bool. True if kill can be acomplished
			* @remarks it's only called when kill is called with force = false
			*/
			virtual bool onKill(){ return true;};
			/**
			 * @brief Called when a process is paused. 
			 */
			virtual void onPause(){};
			/**
			 * @brief Called when process is woken up
			 * 
			 */
			virtual void onWakeUp(){}
			/**
			* called when kill is definitively done
			*/
			virtual void killDone(){};
			
			friend class ProcessScheduler;
			
		};
		
		
		void Process::setPeriod(unsigned int value)
		{
			mPeriod = value;
		}

		void Process::setDead()
		{
			mState = EProcessState::DEAD;
		}
		ProcessScheduler *const Process::getProcessScheduler() const
		{
			return mOwnerProcessScheduler;
		}
		/*bool Process::getDead() const
		{
			return (mState ==EProcessState:: DEAD );
		}
		bool Process::getPreparedToDie() const
		{
			return (mState == EProcessState::PREPARED_TO_DIE);
		}*/
		unsigned int Process::getPeriod() const
		{
			return mPeriod;
		}

		
		
		void Process::resetTime()
		{
			mLastUpdateTime = 0;
		}
		// uint64_t Process::getPreviousTime() const
		// {
		// 	return mPreviousTime;
		// }
		
		// uint64_t Process::getLastTime() const
		// {
		// 	return mLastTime;
		// }
		uint64_t Process::getLastUpdateTime() const
		{
			return mLastUpdateTime;
		}
		

		bool Process::getAsleep() const
		{
			return mState == EProcessState::ASLEEP && !mWakeup;
		}
		
		template <class F> Process::ESwitchResult Process::waitAndDo( unsigned int msegs,F postWait )
		{
			auto v = _preWait();
			//trigger callback
			postWait();
			return 	_postWait(msegs,v);
			//return _wait( msegs,new Callback<void,void>( postWait,::core::use_functor ) );
		}
		template <class F> Process::ESwitchResult Process::sleepAndDo( F postSleep )
		{			
			auto v = _preSleep();
			//trigger callback
			postSleep();
			return 	_postSleep(v);
			//return _sleep( new Callback<void,void>( postSleep,::core::use_functor ) );
		}
	}
}
