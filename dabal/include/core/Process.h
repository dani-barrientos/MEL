#pragma once

#include <DabalLibType.h>
#include <core/Type.h>
using core::Type;

#include <mpl/binary.h>
using mpl::binary;


#include <core/Callback.h>
using core::Callback;
#include <stdint.h>

#if defined(_IOS) || defined(_MACOSX)
#include <TargetConditionals.h>
#endif
#if defined(_X86_GCC) || TARGET_CPU_X86 ||  _MSC_VER
#include <core/Process_X86.h>
#elif TARGET_CPU_X86_64
    #include <core/Process_x86_64_MAC.h>
#elif TARGET_CPU_ARM64

    #include <core/Process_ARM64_IPhone.h>
#elif defined (_ARM_GCC) && (!defined(_IOS))
#include <core/Process_ARM_GCC.h>
#elif defined(_IOS)
	#if !TARGET_IPHONE_SIMULATOR
        #if defined(__arm__)
            #include <core/Process_ARM_IPhone.h>
        #elif defined(__arm64__)
            #include <core/Process_ARM64_IPhone.h>
            #else
                #pragma message "Unknown ARM architecture"
        #endif
	#else
		#include <core/Process_X86.h>
	#endif
#elif defined (_ANDROID)
	#include <core/Process_ARM_Android.h>
#endif
#if defined(_IOS) || defined(_MACOSX)
    #define OPTIMIZE_FLAGS
#elif defined(_ANDROID)
	#define OPTIMIZE_FLAGS
#elif defined(_ARM_GCC) || defined(_X86_GCC)
	#define OPTIMIZE_FLAGS __attribute__((optimize(0)))
#elif defined(_MSC_VER)
	#define OPTIMIZE_FLAGS
#endif

#include <core/CallbackSubscriptor.h>
using core::CallbackSubscriptor;

// #include <core/ProcessException.h>
// using core::ProcessException;
/**
* @namespace core
*/

namespace core
{

	
	class ProcessScheduler;  //predeclaration
	/**
	* @brief a periodic task
	* A Process is scheduled by a ProcessScheduler.
	* @remarks tasks are accoring according to a Timer, which uses uint64_t type to express msecs, but for eficiency reasons
	* using a 64 bits type on 32 bits machines will be very "agressive", so we use unsigned int as time for tasks so getting the low part
	* of the original 64 bit time and hoping it will be enough...
	* @todo what about using size_t constant?I'm afraid it would have problems
	* @remarks switching a Process inside a try/catch context will invalidate exception handling for this context. If this fact is carefully considered when doing "switchs"
	* it will not be a problema, but in Windows (maybe other Operating Systems?) there is a exploitation prevent system (called SEHOP, see https://blogs.technet.microsoft.com/srd/2009/02/02/preventing-the-exploitation-of-structured-exception-handler-seh-overwrites-with-sehop/) )that will make the app crash
	* because Windows interpret it as a hack process. This option is disabled in worksations but enabled for Windows Server- To disable it, go to HKEY_LOCAL_MACHINE/SYSTEM/CurrentControlSet/Control/Session Manager/Kernel/DisableExceptionChainValidation
	*/
	struct KillEvent
	{
	};
	class DABAL_API Process :
		public std::enable_shared_from_this<Process>,
		public MThreadAttributtes,
		private CallbackSubscriptor<KillEvent,true,bool, std::shared_ptr<Process>>  //TODO que poco me gusta esta herencia, incrementa el tama�o de los Process y quisiera que fuesen m�s ligeros
	{
		DABAL_CORE_OBJECT_TYPEINFO_ROOT;
	
		//!should be implemented in platform-dependent code
        static void _switchProcess( ) OPTIMIZE_FLAGS ;
	public:
		/**
		* Process states.
		* These states are not exclusive, so some of them can be simultaneous
		* PREPARADO: est� creado pero todav�a no se inici� INICIADO: se inici� y se est�
		* ejecutando PAUSADO: eso mismo MUERTO: eso mismo.  Ya se encuentra fuera del
		* planificador
		* @created 28-mar-2005 16:20:24
		* @version 1.0
		*/
		typedef enum
		{
			/**
			* Process created but not executed
			*/
			PREPARED = binary<1>::value,
			/**
			* executing process normally
			*/
			INITIATED = binary<10>::value,
			PAUSED = binary<100>::value,
			/**
			* it's going to die, but process manager doesn't discard it
			*/
			PREPARED_TO_DIE = binary<1000>::value,
			TRYING_TO_KILL = binary<10000>::value,
			WAITING_FOR_SCHEDULED = binary<100000>::value,
			/**
			* process is out of process manager
			*/
			DEAD = binary<1000000>::value
		} EProcessState;
		//! reason why Process returns for context switch
		enum ESwitchResult{ ESWITCH_OK,ESWITCH_WAKEUP,ESWITCH_KILL };
		/**
		* nontrustworthy. Don't use yet still
		*/
		//Process( const Process& proc2 );
		/**
		*
		*
		* @param reserveStack If true then statck is initially reserved with capacity
		* @param capacity Initial capacity (only makes sense if reserveStack = true)
		*/
		Process( bool reserveStack = false,unsigned short capacity = 64 );
		/**
		*	destructor
		*/
		virtual ~Process(void);
		/**
		* sets processManager which holds this process
		*
		* @param gestor    gestor
		*/
		void setProcessScheduler(ProcessScheduler*const  gestor);
		/**
		* pause this process
		*/
		virtual void pause( bool resetPreviousTime = false );
		/**
		* activate this process (exit from pause state)
		*/
		virtual void activate( );
		/**
		* mark this process to be eliminated by the process manager.
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
		inline void kill( bool force = false );
		/**
		*  set callback to call after process receives the kill signal and goes to PREPARED_TO_DIE (will be removed
		* from scheduler next iteration)
		* A Process can receive a kill signal but it goes to PREAPARED_TO_DIE if onKill returns true, so this callback is
		* only triggered in this last case
		* callback signature: void f( Process* )
		*/
		template <class F>
		void subscribeKillCallback( F functor );
		/** **********************************************************************/
		/*  removes kill callback set with setKillCallback
		*/
		template <class F>
		void unsubscribeKillCallback( F functor ); 
		/**
		* sets process in init state
		*/
		virtual void reset();
		/**
		* Set the period for this process
		*
		* @param value the new period (in msecs)
		*/
		inline void setPeriod(unsigned int value);

		EProcessState getState() const;
		inline bool getActive() const;
		/**
		* it's process out of process manager?
		*/
		inline bool getDead() const;
		/**
		* it's process prepared to be eliminated from process manager?
		*/
		inline bool getPreparedToDie() const;
		inline unsigned int getPeriod() const;
		//inline bool getAutoDeleted() const;
		/**
		* it's process finished correctly?
		*/
		inline bool getFinished() const;
		inline bool getInitiated() const;
		/**
		* returns time elapsed during this iteration 
		*/
		unsigned int getElapsedTime() const;
		inline unsigned int getPausedTime() const;
		/**
		* @return time in previous iteration
		*/
		inline uint64_t getPreviousTime() const;
		/**
		* @return time of execution for current iteration (i.e. time passed at update function )
		*/
		inline uint64_t getUpdateTime() const;
		inline void resetTime();
		/**
		* execution function. It calls update() when time > mPeriod
		*
		* @param msegs    msegs
		*/

		void onUpdate(uint64_t msegs) OPTIMIZE_FLAGS;

		/**
		* gets ProcessScheduler which holds this Process
		*/
		inline ProcessScheduler *const getProcessScheduler() const;

		/**
		* add new Process to internal ProcessScheduler to execute
		* @brief this Process will be executed at its own period but will be paused and killed
		*  when parent Process will be
		*/
		//void attachProcess( Process* );

		/**
		* set next process to execute. It's appended to the end of the list.
		* Each process can have only one next process.
		* @remarks It's stored as SmartPtr
		*/
		//void setNext( Process* );
		//inline Process* getNext();

		inline void setExtraInfo( void* );
		inline void* getExtraInfo() const;

		/**
		*  get Task ID provided by its processcheduler
		*/
		inline unsigned int getId() const;
		/**
		* time after wich Process will start
		*/
		inline void setStartTime( unsigned int );
		inline unsigned int getStartTime( ) const;
		// get time when process began
		inline uint64_t getBeginTime() const;

		/**
		* @return true if process received kill signal
		* @see switchProcess for comments
		*/
		static ESwitchResult wait( unsigned int msegs ) OPTIMIZE_FLAGS; 

		/**
		* wrapper for _switchProcess
		* @return true if process received kill signal
		* @remarks the objective is to throw a ProcessException if process has received kill signal, but an unknown error in
		*	Visual Studio microthread implementacion doesn't allow it. Also, you can not use a try-catch block after any context switch operation
		*   (switchProcess,wait,sleep) except in a child function. For example the next code breaks:
		*		{
		*			switchProcess(false);
		*			try{
		*				....tikitiki...
		*				throw <some exception>;
		*			}catch( <some exception> ){}
		*		}
		*		but if you put the try-catch block in another function, called by this, then it works (but maybe it hasn't any usefulness)
		*
		* @remarks see comments on SEHOP in file header
		*/
		
		static ESwitchResult switchProcess( bool v ) OPTIMIZE_FLAGS;
		/**
		* stop process. To reactivate you must use wakeUp
		* @param[in] postSleep functor (signature <void,void>) to execute just in the moment when Process go to sleep
		* @return resulting state of the operation
		* @see sleep
		* @remarks not multithread-safe
		*/
		template <class F>
		static ESwitchResult sleepAndDo( F postSleep ) OPTIMIZE_FLAGS
		{
			return _sleep( new Callback<void,void>( postSleep,::core::use_functor ) );
		}
		/**
		* @param[in] postWait functor (signature <void,void>) to execute just in the moment when Process go to sleep
		* @return resulting state of the operation
		* @see sleep
		* @remarks not multithread-safe
		*/
		template <class F>
		static ESwitchResult waitAndDo( unsigned int msegs,F postWait ) OPTIMIZE_FLAGS
		{
			return _wait( msegs,new Callback<void,void>( postWait,::core::use_functor ) );
		}

		/**
		* stop process. To reactivate you must use wakeUp
		* @return true if process received kill signal
		* @see switchProcess for comments
		*/
		static ESwitchResult sleep( ) OPTIMIZE_FLAGS;
		inline bool getAsleep() const;
		inline bool getWaiting() const;
		void wakeUp();

	protected:

		/**
		* scheduler in which is inserted
		*/
		ProcessScheduler* mOwnerProcessScheduler;

		/**
		* main execution block
		*
		*
		* @param msegs    msegs
		*/
		void execute(uint64_t msegs) OPTIMIZE_FLAGS;

		/**
		* @warning no la tengo muy clara si realmente se podr� evitar
		*/
		void setFinished( bool );

	private:
		static ESwitchResult _sleep(  Callback<void,void>* ) OPTIMIZE_FLAGS;
		static ESwitchResult _wait( unsigned int msegs, Callback<void,void>* ) OPTIMIZE_FLAGS;
		typedef CallbackSubscriptor<KillEvent,true,bool, std::shared_ptr<Process>> KillEventSubscriptor;
		EProcessState mState;
		EProcessState mPreviousState;
		unsigned int mPeriod;
		volatile bool mSleeped;
		volatile bool mWakeup; //temp value to know if context switch comes from a wakeup
		volatile bool mWaiting; //true when in a wait operation @todo en realidad ahora no me sirve para nada, lo dejo por si acaso hasta tenerlo claro, pero es conveniente quitarlo

		/**
		* si es true indica que fue terminado correctamente y no "forzado"
		*/
		bool	mFinished;
		bool	mResetPreviousTime;
		/**
		* time at process execution
		*/
		uint64_t mLastTime;
		uint64_t mUpdateTime;
		/**
		* time at previous iteration
		*/		
		//ProcessScheduler*	mAttachedProcesses;
		unsigned int mProcessId;


		//extra data passed to Process for custom processing TODO temporal hasta tener el extradata bien
		void*			mExtrainfo;		
		uint64_t		mPreviousTime;
		uint64_t		mBeginTime;  //time stored at init
		uint64_t		mPauseTime;  //time when process was paused
		unsigned int	mStartTime; //!when to start process since insertion in scheduler (default = 0)
		unsigned int	mPausedTime;  
		//SmartPtr<Process>	mNext; //next process in chain. It's scheduled when killed
		//Callback<void,Process*>* mKillCallback;


		volatile void checkMicrothread(uint64_t msegs ) OPTIMIZE_FLAGS;

	protected:
		/**
		* behaviour function. The main function of a process. MUST be implemented in
		* childs. It's called automatically when elapsed time >= mPeriod
		*
		* @param msecs milliseconds
		*/
		virtual void update(uint64_t msecs) = 0;
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
		* @remarks it's only called when kill if called with force = false
		*/
		virtual bool onKill(){ return true;};
		/**
		* called when kill is definitively done
		*/
		virtual void killDone(){};
		
		friend class ProcessScheduler;
		/**
		* this is intended to be used by ProcessScheduler
		*/
		inline void setDead();
		inline void setId( unsigned int id );
	};
	
	unsigned int Process::getId() const
	{
		return mProcessId;
	}
	void Process::setId( unsigned int id )
	{
		mProcessId = id;
	}
	void Process::setPeriod(unsigned int value)
	{
		mPeriod = value;
	}

	void Process::setDead()
	{
		mState = DEAD;
	}
	ProcessScheduler *const Process::getProcessScheduler() const
	{
		return mOwnerProcessScheduler;
	}

	bool Process::getActive() const
	{
		return (mState!= PAUSED) && !mSleeped;  //quitar la condicion de mSleeped?? no es ortogonal
	}
	bool Process::getDead() const
	{
		return (mState == DEAD );
	}
	bool Process::getPreparedToDie() const
	{
		return (mState == PREPARED_TO_DIE);
	}
	unsigned int Process::getPeriod() const
	{
		return mPeriod;
	}

	bool Process::getFinished() const
	{
		return mFinished;
	}
	bool Process::getInitiated() const
	{
		return ( mState == INITIATED );
	}
	void Process::resetTime()
	{
		mLastTime = 0;
	}
	uint64_t Process::getPreviousTime() const
	{
		return mPreviousTime;
	}
	uint64_t Process::getUpdateTime() const
	{
		return mUpdateTime;
	}
	void Process::kill( bool force )
	{
		//TODO esta funcion no est� un pijo bien, hay que revisarla junto con el Process::execute

		if ( !(mState & DEAD) ) 
		{
			if ( force )
			{
				if ( mSwitched )
				{
					mState = WAITING_FOR_SCHEDULED;
					wakeUp();
				}else
				{
					mState = PREPARED_TO_DIE;
					killDone();
				}
			}else
			{
				bool ok = onKill();
				if ( ok )
				{
					if ( mSwitched )
					{
						mState = WAITING_FOR_SCHEDULED;
						wakeUp();
					}else
					{
						mState = PREPARED_TO_DIE;
						killDone();
					}
				}else
				{
					mState = TRYING_TO_KILL;
				}

			}
			if ( mState & PREPARED_TO_DIE  )	
			{
				KillEventSubscriptor::triggerCallbacks( shared_from_this() );
			}

		}

	}
	void Process::setExtraInfo( void* info )
	{
		mExtrainfo = info;
	}
	void* Process::getExtraInfo() const
	{
		return mExtrainfo;
	}
	void Process::setStartTime( unsigned int st )
	{
		mStartTime = st;
	}
	unsigned int Process::getStartTime( ) const
	{
		return mStartTime;
	}
	uint64_t Process::getBeginTime() const
	{
		return mBeginTime;
	}
	/*Process* Process::getNext()
	{
		return mNext.getPtr();
	}*/
	/*template <class F>
	void Process::setKillCallback( F functor )
	{
		delete mKillCallback;
		mKillCallback = new Callback<void,Process*>( functor, ::core::use_functor );
	}*/

/*
	void Process::clearKillCallback()
	{
		delete mKillCallback;
		mKillCallback=NULL;
	}
*/
	template <class F>
	void Process::subscribeKillCallback( F functor )
	{
		KillEventSubscriptor::subscribeCallback( functor );
	}
	template <class F>
	void Process::unsubscribeKillCallback( F functor )
	{
		KillEventSubscriptor::unsubscribeCallback( functor );
	}
	bool Process::getAsleep() const
	{
		return mSleeped;
	}
	bool Process::getWaiting() const
	{
		return mWaiting;
	}
	unsigned int Process::getPausedTime() const
	{
		return mPausedTime;
	}
}

