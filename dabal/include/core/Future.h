#pragma once
// #include <core/IRefCount.h>
// using core::IRefCount;

//#include <core/Event.h>
//#include <core/Event_mthread.h>
#include <mpl/TypeTraits.h>
#include <memory>
//#include <core/CriticalSection.h>
//#include <mpl/MemberEncapsulate.h>
//using mpl::makeMemberEncapsulate;
#include <string>
#include <memory>
#include <core/CallbackSubscriptor.h>
namespace core
{
	//using core::Event;
	//using core::Event_mthread;
	using core::CriticalSection;
	using mpl::TypeTraits;
	

//nombnrarlos bien. �meter nuevo estado UNINITIALIZED?
	enum EFutureState {NOTAVAILABLE,VALID,INVALID} ;
	///@cond HIDDEN_SYMBOLS
	/**
	* internal data for Future
	*/
	class FutureData_Base 
	{
	public:
		/**
		* estructura muy simple para almacenar errores en Future.ES NECESARIO MEJORARLA
		*/
		struct ErrorInfo
		{
			//@remarks negative error code is reserved for internal errors
			int		error;  //there was error. Error code. Very simple for now. 
			std::string errorMsg;
			virtual ~ErrorInfo(){}
			bool operator== (const ErrorInfo & err) const {return true;}
		};
		//result code
		enum EWaitResult{ FUTURE_WAIT_OK = 0,
			FUTURE_RECEIVED_KILL_SIGNAL = -1,
			FUTURE_WAIT_TIMEOUT = -2,
			FUTURE_UNKNOWN_ERROR = -3};

		FutureData_Base():/*mResultAvailable(false,false),mResultAvailableMThread(false,false),*/mState(NOTAVAILABLE)/*,mRefCount(0)*/
		{}

		virtual ~FutureData_Base() //no me gusta nada que sea virtual. Intentarlo sin ello
		{}
		inline bool getValid() const
		{
			return mState == VALID;
		}
		inline EFutureState getState() const
		{
			return mState;
		}
		/**
		* waits for value to be present

		*/
	/*
		EWaitResult wait( unsigned int msecs ) const
		{
			if ( mResultAvailable.wait( msecs) == Event::EVENT_WAIT_TIMEOUT )
			{
				mSC.enter();	
				//podr�a ocurrir que en este lapso de tiempo se estableciese correctamente el valor
				if( !getValid() )
				{
					return FUTURE_WAIT_TIMEOUT;
				}
			}
			return FUTURE_WAIT_OK;
		}
		EWaitResult waitAsMThread( unsigned int msecs ) const
		{
			EWaitResult result;
			//first check if already set
			if ( mState != NOTAVAILABLE )
				result = FUTURE_WAIT_OK;
			else
			{
				mSC.enter();
				Event_mthread::EWaitCode eventresult;
				eventresult = mResultAvailableMThread.waitAndDo(makeMemberEncapsulate( &CriticalSection::leave, &mSC ), msecs );
				switch( eventresult )
				{
				case Event_mthread::EVENTMT_WAIT_KILL:
					//event was triggered because a kill signal
					result = FUTURE_RECEIVED_KILL_SIGNAL;
					break;
				case Event_mthread::EVENTMT_WAIT_TIMEOUT:
					result = FUTURE_WAIT_TIMEOUT;
					break;
				default:
					result = FUTURE_WAIT_OK;
					break;
				}
			}
			return result;			
		}*/
		/**
		* set error info. TAKES OWNSERHIP
		*/
		void setError( ErrorInfo* ei )
		{
			//@todo tanto esto como el setValue, tal vez sólo deba ser ejecutado por uno y no necesitaría bloqueo,,
			//mSC.enter();	
			if ( mState == NOTAVAILABLE)
			{
				mErrorInfo.reset( ei  );
				mState = INVALID;
				//mResultAvailable.set();
				//mResultAvailableMThread.set();
			}
			else
				delete ei;
			//mSC.leave();	
		};

		inline const ErrorInfo* getError() const { return mErrorInfo.get(); };
	protected:
		//Event	   mResultAvailable;
		//mutable Event_mthread	   mResultAvailableMThread; 
		//mutable CriticalSection	mSC; 
		std::unique_ptr< ErrorInfo > mErrorInfo; //It will have content when error
		EFutureState		mState;

	};
	template <typename ResultType>
	class FutureData : public FutureData_Base,
					public 	CallbackSubscriptor<::core::MultithreadPolicy,typename ::mpl::TypeTraits<ResultType>::ParameterType>
	{		
	public:
		typedef typename mpl::TypeTraits< ResultType >::ParameterType ReturnType;
		typedef typename mpl::TypeTraits< ResultType >::ParameterType ParamType;
		/**
		* default constructor
		*/
		FutureData(){};
		~FutureData(){};
		/**
		* constructor from value. It becomes valid. For casting purposes. 
		*/
		FutureData( ParamType value ):
		mValue(value){
			FutureData_Base::mState = VALID;
//			FutureData_Base::mResultAvailable.set();
//			FutureData_Base::mResultAvailableMThread.set();
		}
	

		typename FutureData<ResultType>::ReturnType getValue() const;
		void setValue( ParamType value );

	private:
		ResultType mValue;
	
	};

	template <typename ResultType>
	typename FutureData<ResultType>::ReturnType FutureData<ResultType>::getValue() const
	{
		return mValue;
	}
	template <typename ResultType>
	void FutureData<ResultType>::setValue( ParamType value )
	{
		//FutureData_Base::mSC.enter();	
		if ( mState == NOTAVAILABLE)
		{
			mValue = value;
			FutureData_Base::mState = VALID;
		//	FutureData_Base::mResultAvailable.set();
	//		FutureData_Base::mResultAvailableMThread.set();
			CallbackSubscriptor<::core::MultithreadPolicy, typename mpl::TypeTraits< ResultType >::ParameterType>::triggerCallbacks(value);
		}
		//FutureData_Base::mSC.leave();	

	}
	//specialization for void type. It's intented for functions returning void but working in a different thread
	//so user need to know when it finish
	//TODO no estoy un pijo convencido...
	template <>
	class FutureData<void> : public FutureData_Base
	{
	public:
		typedef void ReturnType;
		typedef void ParamType;

		FutureData(){};
		~FutureData(){};

		inline void getValue() const{ return;}
		inline void setValue( void )
		{

//			FutureData_Base::mSC.enter();	
			if ( mState == NOTAVAILABLE)
			{
				FutureData_Base::mState = VALID;
//				FutureData_Base::mResultAvailable.set();
//				FutureData_Base::mResultAvailableMThread.set();
			}
//			FutureData_Base::mSC.leave();	

		}
	};

	
	
	///@endcond
	/**
	* @class Future
	*  Represents a value retreived by (possibly) another thread. This value maybe is not present at the moment
	* and can be waited,etc
	* A value is retrieved using various ways:
	*	- polling in a loop checking for "getValid" and "getError"
	*	- using wait and waitAsMThread functions
	* should check getValid, in which case there is an error ( see getError )
	* @todo por ahora un Future no es reaprovechable, es decir, una vez construido o establecido su valor o error
	* ya no puede "resetearse". Ni siquiera s� si tiene sentido
	* Adem�s, no est� pensado para ser usado por varios hilos a la vez (de nuevo, no creo que tenga sentido)
	*/
	///@cond HIDDEN_SYMBOLS
	/**
	* no templated common data.
	*/
	class Future_Base
	{
	protected:
		std::shared_ptr<FutureData_Base> mData;
		Future_Base():mData(nullptr){};

	public:
		Future_Base( const Future_Base& f )
		{
			mData = f.mData; 
		};
		Future_Base( Future_Base&& f )
		{
			mData = std::move(f.mData); 
		};

		virtual ~Future_Base() //no me gusta un pijo que sea virtual, pero necesario si se quieren manejar los futures sin necesidad de saber el tipo, lo cual es muy importante
		{
		}

		typedef FutureData_Base::ErrorInfo ErrorInfo;
		Future_Base& operator= ( const Future_Base& f )
		{
			mData = f.mData;
			return *this;
		};
		Future_Base& operator= (  Future_Base&& f )
		{
			mData = std::move(f.mData);
			return *this;
		};
		/**
		* waits for value to be present in a thread context
		* @param[in] msecs Milliseconds to wait for
		*/
		//inline FutureData_Base::EWaitResult wait( unsigned int msecs = Event::EVENT_WAIT_INFINITE ) const { return mData->wait( msecs ); };
		/**
		* waits in a Process context
		* @return result codes. Any of EWaitResult
		*/
		// inline FutureData_Base::EWaitResult waitAsMThread( unsigned int msecs = Event_mthread::EVENTMT_WAIT_INFINITE ) const
		// {
		// 	return mData->waitAsMThread( msecs );
		// }
		/**
		* return if data is valid
		*/
		inline bool getValid() const
		{
			return mData->getValid();
		}
		inline EFutureState getState() const
		{
			return mData->getState();
		}
		//Check if given future points to same data
		inline bool operator == ( const Future_Base& f ) const{ return mData == f.mData; };
		/**
		* return if there is an error. NULL if not
		*/
		inline const FutureData_Base::ErrorInfo* getError() const { return mData->getError(); };
 		inline void setError( FutureData_Base::ErrorInfo* ei ){ mData->setError( ei ); } 
		//convenient overload getting errr code and msg
		inline void setError( int code,const std::string& msg) 
		{
			ErrorInfo* ei = new ErrorInfo();
			ei->error = code;
			ei->errorMsg = msg;
			mData->setError(ei); 
		}
		
	};
	template <typename ResultType>
	class Future_Common : public Future_Base
	{
	protected:
		Future_Common()
		{
			mData = std::make_shared<FutureData<ResultType>>();
		};
		Future_Common( const Future_Common& f ):Future_Base( f ){}			
	public:
		inline const FutureData<ResultType>& getData() const{ return *(FutureData<ResultType>*)mData; }
		inline FutureData<ResultType>& getData(){ return *(FutureData<ResultType>*)mData.get(); }


		inline  typename FutureData<ResultType>::ReturnType getValue() const{ return ((FutureData<ResultType>*)mData.get())->getValue();}
		
		/**
		* return if there is an error. NULL if not
		*/
// 		inline const FutureData_Base::ErrorInfo* getError() const { return mData->getError(); };
// 		inline void setError( FutureData_Base::ErrorInfo* ei ){ ((FutureData<ResultType>*)mData)->setError( ei ); } 
		template <class F> void subscribeCallback(F&& f) const						
		{
			//@todo no me gusta un pijo este cast, pero necesito que el subscribe actúa como mutable
			const_cast<Future_Common<ResultType>*>(this)->getData().subscribeCallback( std::forward<F>(f));
		}
	};
	///@endcond
	template <typename ResultType>
	class Future : public Future_Common<ResultType>
	{
	public:
		typedef typename mpl::TypeTraits< ResultType >::ParameterType ParamType;
		typedef typename mpl::TypeTraits< ResultType >::ParameterType ReturnType;
		Future(){};
		Future( const Future& f ):Future_Common<ResultType>(f){};

		inline void setValue( typename FutureData<ResultType>::ParamType value )
		{
		    ((FutureData<ResultType>*)Future_Common<ResultType>::mData.get())->setValue( value ); }
	};

	//specialization for void
	template <>
	class Future<void> : public Future_Common<void>
	{
	public:
		typedef void ParamType;
		typedef void ReturnType;
		Future(){};
		Future(const Future& f):Future_Common<void>(f){};
	
		inline void setValue( void ){ ((FutureData<void>*)Future_Base::mData.get())->setValue( ); }
	};


}
