#pragma once

#include <tasking/GenericProcessDefs.h>
#include <tasking/Process.h>
using mel::tasking::Process;

#include <functional>
namespace mel
{
	namespace tasking
	{
		/**
		* @brief A Process constructed from a functor with signature *EGenericProcessResult(uint64_t,Process*)*
		* This process it's executed until given functor returns \ref EGenericProcessResult::KILL.
		*/
		class MEL_API GenericProcess : public Process
		{
			MEL_CORE_OBJECT_TYPEINFO;
		public:
			typedef std::function<EGenericProcessResult (uint64_t,Process*)> GenericCallback;
		private:
			GenericCallback					mProcessCallback;
			std::function<bool ()> 			mKillCallback;
			volatile EGenericProcessResult	mUpdateResult;

		public:
			/**
			* constructor
			* @todo hacer los constructores convenientes
			*/
			GenericProcess( );
			~GenericProcess();
			
			/**
			* @brief Set callable to be executed on process update
			* @param[in] functor A functor with signature EGenericProcessResult(uint64_t,Process*)
			*/
			template <class F> void setProcessCallback( F&& functor );
			inline const GenericProcess::GenericCallback& getProcessCallback() const;
			template <class F> void setKillCallback( F&& functor );			
		protected:
			void onInit(uint64_t msegs) override;
			/**
			* overridden from Process
			*/
			bool onKill() override;
			/**
			* overridden from Process
			* @param msegs    msegs
			* @remarks If callback method return false. Process is killed.
			*/
			void onUpdate(uint64_t msegs) override;
		};
		template <class F> void GenericProcess::setProcessCallback( F&& functor )
		{
			mProcessCallback = std::forward<F>(functor);
		}
		const GenericProcess::GenericCallback& GenericProcess::getProcessCallback() const
		{
			return mProcessCallback;
		}
		template <class F> void GenericProcess::setKillCallback( F&& functor )
		{
			mKillCallback = std::forward<F>(functor);
		}
	}
}