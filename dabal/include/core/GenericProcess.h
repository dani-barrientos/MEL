#pragma once

#include <core/GenericProcessDefs.h>
#include <core/Process.h>
using core::Process;

#include <functional>
namespace core
{
	/**
	* A Process constructed from a functor
	* This Process it's executed until given functor returns true. This functor is given the current state wich is:
	*		INIT : first time it's executed
	*		RUN : second and next times
	*		KILL : a kill request it's done. 
	*/
	class DABAL_API GenericProcess : public Process
	{
		DABAL_CORE_OBJECT_TYPEINFO;
	public:
		//typedef Callback< bool, uint64_t, Process*, EGenericProcessState > GenericCallback;
		typedef std::function<EGenericProcessResult (uint64_t,Process*,EGenericProcessState)> GenericCallback;
	private:
		//GenericCallback*		mProcessCallback;
		GenericCallback			mProcessCallback;
		EGenericProcessState	mCurrentState;
		volatile EGenericProcessResult			mUpdateResult;
		bool					mAutoKill;

	public:
		typedef EGenericProcessState EState; //for compliance
		/**
		* constructor
		* @todo hacer los constructores convenientes
		*/
		GenericProcess( );
		~GenericProcess();
		
		/**
		* set callback to execute on process update
		* @param[in] functor A functor with signature bool f(unsigned int msegs, Process*, EGenericProcessState )
		*/
		template <class F> void setProcessCallback( F&& functor );
		/*void setProcessCallback( std::function< bool(uint64_t,Process*,EGenericProcessState)>&& );
		void setProcessCallback(const std::function< bool(uint64_t, Process*, EGenericProcessState)>&);
		void setProcessCallback(std::function< bool(uint64_t, Process*, EGenericProcessState)>&);*/
		inline const GenericProcess::GenericCallback& getProcessCallback() const;
		/**
		* set if process will be atomatically killed when kill signal is received
		* instead waiting for internal callback to terminate.
		* Class default is "false"
		*/
		inline void setAutoKill( bool value );
		inline bool getAutoKill() const;
		/*
		* set callback to execute on process init
		* @param[in] functor. A functor with signature bool f(unsigned int msegs, Process* )
		
		template <class F>
		void setInitCallback( F functor );
		*/
		/*
		* set callback to execute on process kill. it must return boolean (see Process::onKill)
		* @param functor. A functor with signature bool f(unsigned int msegs, Process* )
		*
		template <class F>
		void setKillCallback( F functor );*/

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
		// delete mProcessCallback;
		// mProcessCallback = new GenericCallback( ::std::forward<F>(functor),::core::use_functor );
		mProcessCallback = std::forward<F>(functor);
	}
	const GenericProcess::GenericCallback& GenericProcess::getProcessCallback() const
	{
		return mProcessCallback;
	}
	void GenericProcess::setAutoKill( bool value )
	{
		mAutoKill = value;
	}
	bool GenericProcess::getAutoKill() const
	{
		return mAutoKill;
	}


};