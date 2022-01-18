#pragma once

#include <mpl/TypeTraits.h>
#include <memory>
#include <core/CriticalSection.h>

#include <string>
#include <memory>
#include <core/CallbackSubscriptor.h>
namespace core
{
	using core::CriticalSection;
	using mpl::TypeTraits;


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
	
		

		inline const ErrorInfo* getError() const { return mErrorInfo.get(); };
	protected:
		mutable CriticalSection	mSC; 
		std::unique_ptr< ErrorInfo > mErrorInfo; //It will have content when error
		EFutureState		mState;

	};
	template <typename ResultType>
	class FutureData : public FutureData_Base,
					private CallbackSubscriptor<::core::MultithreadPolicy,const FutureData<ResultType>&>,
					public std::enable_shared_from_this<FutureData<ResultType>>
	{		
	public:
		typedef typename mpl::TypeTraits< ResultType >::ParameterType ReturnType;
		typedef typename mpl::TypeTraits< ResultType >::ParameterType ParamType;
		typedef CallbackSubscriptor<::core::MultithreadPolicy, const FutureData<ResultType>&> Subscriptor;
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
		/**
		* set error info. TAKES OWNSERHIP
		*/
		void setError( ErrorInfo* ei )
		{
			volatile auto protectMe=FutureData<ResultType>::shared_from_this();
			mSC.enter();
			if ( mState == NOTAVAILABLE)
			{
				mErrorInfo.reset( ei  );
				mState = INVALID;
			}
			else
				delete ei;
			mSC.leave();
			Subscriptor::triggerCallbacks(*this);
		};
		/**
		 * @brief Subscribe to future available
		 * 
		 * @tparam F functor with signature 
		 * @param f 
		 * @return auto 
		 * @warning callback receiver shoudn't wait or do context switch and MUST NOT block
		 */
		template <class F> auto subscribeCallback(F&& f)
		{
			Lock lck(mSC);
			if ( mState != NOTAVAILABLE)
			{
				f(*this);
			}
			return Subscriptor::subscribeCallback(std::forward<F>(f)); //shoudn be neccesary if mstate already avaialbe but for consistency
		}
		template <class F> auto unsubscribeCallback(F&& f)
		{
			Lock lck(mSC);
			return Subscriptor::unsubscribeCallback( std::forward<F>(f));
		}

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
		volatile auto protectMe= FutureData<ResultType>::shared_from_this();
		FutureData_Base::mSC.enter();	
		if ( mState == NOTAVAILABLE)
		{
			mValue = value;
			FutureData_Base::mState = VALID;
			FutureData_Base::mSC.leave();
			Subscriptor::triggerCallbacks(*this);
		}else
			FutureData_Base::mSC.leave();

	}
	//specialization for void type. It's intented for functions returning void but working in a different thread
	//so user need to know when it finish
	//TODO no estoy un pijo convencido...
	template <>
	class FutureData<void> : public FutureData_Base,
		private CallbackSubscriptor<::core::MultithreadPolicy,const FutureData<void>&>,
		public std::enable_shared_from_this<FutureData<void>>
	{
		typedef CallbackSubscriptor<::core::MultithreadPolicy,const FutureData<void>&> Subscriptor;
	public:
		typedef void ReturnType;
		typedef void ParamType;

		FutureData(){};
		~FutureData(){};

		template <class F> auto subscribeCallback(F&& f)
		{
			bool trigger = false;
			Lock lck(FutureData_Base::mSC);
			if ( mState != NOTAVAILABLE)
			{
				f(*this);
			}				
			return Subscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f)
		{
			Lock lck(FutureData_Base::mSC);
			return Subscriptor::unsubscribeCallback(std::forward<F>(f));
		}
		inline void getValue() const{ return;}
		inline void setValue( void )
		{
			volatile auto protectMe=shared_from_this();
			FutureData_Base::mSC.enter();	
			if ( mState == NOTAVAILABLE)
			{
				FutureData_Base::mState = VALID;
				FutureData_Base::mSC.leave();
				Subscriptor::triggerCallbacks(*this);
			}else
				FutureData_Base::mSC.leave();
		}
		/**
		* set error info. TAKES OWNSERHIP
		*/
		void setError( ErrorInfo* ei )
		{
			volatile auto protectMe=shared_from_this();
			FutureData_Base::mSC.enter();
			if ( mState == NOTAVAILABLE)
			{
				mErrorInfo.reset( ei  );
				mState = INVALID;
			}
			else
				delete ei;
			FutureData_Base::mSC.leave();
			Subscriptor::triggerCallbacks(*this);
		};
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
		template <class F> auto subscribeCallback(F&& f) const						
		{
			//@todo no me gusta un pijo este cast, pero necesito que el subscribe actúa como mutable
			return const_cast<Future_Common<ResultType>*>(this)->getData().subscribeCallback( std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f) const
		{
			return const_cast<Future_Common<ResultType>*>(this)->getData().unsubscribeCallback( std::forward<F>(f));
		}
		inline void setError( FutureData_Base::ErrorInfo* ei ){ 
			((FutureData<ResultType>*)Future_Common<ResultType>::mData.get())->setError( ei ); } 
		//convenient overload getting err code and msg
		inline void setError( int code,const std::string& msg) 
		{
			FutureData_Base::ErrorInfo* ei = new FutureData_Base::ErrorInfo();
			ei->error = code;
			ei->errorMsg = msg;
			((FutureData<ResultType>*)Future_Common<ResultType>::mData.get())->setError(ei); 
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
		    ((FutureData<ResultType>*)Future_Common<ResultType>::mData.get())->setValue( value ); 
		}	
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
