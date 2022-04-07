#pragma once
#include <tasking/Event_mthread.h>
#include <atomic>
namespace mel
{
	namespace tasking
	{
		using mel::tasking::Event_mthread;
		/**
		 * @brief A critical section for synchronizing microthreads (AKA \ref ::mel::tasking::Process "Process")
		 * @todo multithread policy need to be more elaborated
		*/
		template <bool multithread = true >class CriticalSection_mthread
		{
		public:
			CriticalSection_mthread();
			/**
			* wait for critical section and enter
			* @return if false, then can't eneter critical section (becausee process kill, for example)
			* @remarks mejorar valor de salida, para tener algo ahora
			*/
			bool enter();
			void leave();
			//TODO try enter
		private:
			Event_mthread<mel::tasking::EventMTThreadSafePolicy> mEvent;
			std::shared_ptr<mel::tasking::Process>	mOwner;
			std::atomic<int>	mCount; //number of times owner as acquired section
		};
		///@cond HIDDEN_SYMBOLS
		template <> class MEL_API CriticalSection_mthread<false>
		{
		public:
			CriticalSection_mthread();
			/**
			* wait for critical section and enter
			* @return if false, then can't eneter critical section (becausee process kill, for example)
			* @remarks mejorar valor de salida, para tener algo ahora
			*/
			bool enter();
			void leave();
			//TODO try enter
		private:
			Event_mthread<::mel::tasking::EventNoMTThreadSafePolicy> mEvent;
			std::shared_ptr<::mel::tasking::Process>	mOwner;
			int	mCount; //number of times owner as acquired section
		};
		namespace _private
		{
			class _WrapperBase
			{
				public:
					virtual ~_WrapperBase(){}
				private:				
			};
			template <bool ismth> class _Wrapper : public _WrapperBase
			{
				public:
					_Wrapper(CriticalSection_mthread<ismth>& cs):mCS(cs)
					{
						cs.enter();
					}
					~_Wrapper()
					{
						mCS.leave();
					}
				private:
					CriticalSection_mthread<ismth>& mCS;
			};
			
		}
		///@endcond
		class Lock_mthread
		{
		public:
			//! enter critical section.
			template <bool ismth>
			Lock_mthread(CriticalSection_mthread<ismth>& cs):mWrapper(new _private::_Wrapper<ismth>(cs)){}		
		private:
			//CriticalSection_mthread& mCS;
			std::unique_ptr<_private::_WrapperBase> mWrapper;
		};	

	}
}