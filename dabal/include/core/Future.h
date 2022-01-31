#pragma once

#include <mpl/TypeTraits.h>
#include <memory>
#include <core/CriticalSection.h>

#include <string>
#include <memory>
#include <core/CallbackSubscriptor.h>
#include <variant>
#include <optional>
namespace core
{
	using core::CriticalSection;
	using mpl::TypeTraits;

	struct ErrorInfo
	{
		ErrorInfo(int aErr,std::string aMsg):error(aErr),errorMsg(std::move(aMsg)){}
		//@remarks negative error code is reserved for internal errors
		int		error;  //there was error. Error code. Very simple for now. 
		std::string errorMsg;
//		virtual ~ErrorInfo(){}
	};
	template <class T,class ErrorType> class FutureValue : public std::variant<T,ErrorType>
	{
		typedef std::variant<T,ErrorType> Base;
		public:
			FutureValue(){}
			FutureValue(const T& v):Base(v){}
			FutureValue(T&& v):Base(std::move(v)){}
			FutureValue(const ErrorType& err):Base(err){}
			FutureValue(ErrorType&& err):Base(std::move(err)){}
			/**
			 * @brief get if has valid value
			 */
			bool isValid() const{ return Base::index() == 0;}
			// wrapper for std::get.  Same rules as std::Get, so bad_variant_access is thrown if not a valid value
			T& value() {
				return std::get<T>(*this);
			}
			const T& value() const {
				return std::get<T>(*this);
			}
			const ErrorType& error() const
			{
				return std::get<ErrorType>(*this);
			}
			auto& operator=(const T& v){
				Base::operator=(v);
				return *this;
			}
			auto& operator=(T&& v){
				Base::operator=(std::move(v));
				return *this;
			}
			auto& operator=(const ErrorType& v){
				Base::operator=(v);
				return *this;
			}
			auto& operator=(ErrorType&& v){
				Base::operator=(std::move(v));
				return *this;
			}
	};
	//specialization for void 
	template <class ErrorType> class FutureValue<void,ErrorType> : public std::optional<ErrorType>
	{
		typedef std::optional<ErrorType> Base;
		public:
			FutureValue(){}
			FutureValue(const ErrorType& err):Base(err){}
			FutureValue(ErrorType&& err):Base(std::move(err)){}
			/**
			 * @brief get if has valid value
			 */
			bool isValid() const{ return !Base::has_value();}
			// wrapper for optional::value().  Same rules as std::Get, so bad_optional_access is thrown if not a valid value			
			const ErrorType& error() const
			{
				return Base::value();
			}			
			auto& operator=(const ErrorType& v){
				Base::operator=(v);
				return *this;
			}
			auto& operator=(ErrorType&& v){
				Base::operator=(std::move(v));
				return *this;
			}
	};

	enum EFutureState {NOTAVAILABLE,VALID,INVALID} ;
	///@cond HIDDEN_SYMBOLS
	/**
	* internal data for Future
	*/
	class FutureData_Base 
	{
	public:								
		FutureData_Base():mState(NOTAVAILABLE)
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
	protected:
		mutable CriticalSection	mSC; 
		EFutureState		mState;

	};
	template <typename ResultType,typename ErrorType>
	class FutureData : public FutureData_Base,
					private CallbackSubscriptor<::core::CSMultithreadPolicy,const FutureValue<ResultType,ErrorType>&>,
					public std::enable_shared_from_this<FutureData<ResultType,ErrorType>>
	{		
	public:
		typedef FutureValue<ResultType,ErrorType> ValueType;
		typedef typename mpl::TypeTraits< ValueType >::ParameterType ReturnType;
		typedef CallbackSubscriptor<::core::CSMultithreadPolicy,const ValueType&> Subscriptor;
		/**
		* default constructor
		*/
		FutureData(){};
		~FutureData(){};
		/**
		* constructor from value. It becomes valid. For casting purposes. 
		*/
		FutureData( const ResultType& value ):
			mValue(value){
			FutureData_Base::mState = VALID;
		}
		FutureData( ResultType&& value ):
			mValue(std::move(value)){
			FutureData_Base::mState = VALID;
		}

		ReturnType getValue() const
		{
			return mValue;
		}
		void setValue( const ResultType& value ){
			volatile auto protectMe= FutureData<ResultType,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();	
			if ( mState == NOTAVAILABLE)
			{
				mValue = value;
				FutureData_Base::mState = VALID;
				FutureData_Base::mSC.leave();
				Subscriptor::triggerCallbacks(mValue);
			}else
				FutureData_Base::mSC.leave();
		}
		void setValue( ResultType&& value ){
			volatile auto protectMe= FutureData<ResultType,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();	
			if ( mState == NOTAVAILABLE)
			{
				mValue = std::move(value);
				FutureData_Base::mState = VALID;
				FutureData_Base::mSC.leave();
				Subscriptor::triggerCallbacks(mValue);
			}else
				FutureData_Base::mSC.leave();
		}		
		void setError( const ErrorType& ei )
		{
			volatile auto protectMe=FutureData<ResultType,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();
			if ( mState == NOTAVAILABLE)
			{
				mValue = ei;
				mState = INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}
			FutureData_Base::mSC.leave();
		};
		void setError( ErrorType&& ei )
		{
			volatile auto protectMe=FutureData<ResultType,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();
			if ( mState == NOTAVAILABLE)
			{
				mValue = std::move(ei);
				mState = INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}
			FutureData_Base::mSC.leave();
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
			Lock lck(FutureData_Base::mSC);
			if ( mState != NOTAVAILABLE)
			{
				f(mValue);
			}
			return Subscriptor::subscribeCallback(std::forward<F>(f)); //shoudn be neccesary if mstate already avaialbe but for consistency
		}
		template <class F> auto unsubscribeCallback(F&& f)
		{
			Lock lck(FutureData_Base::mSC);
			return Subscriptor::unsubscribeCallback( std::forward<F>(f));
		}

	private:
		ValueType mValue;
	
	};

	//specialization for void type. It's intented for functions returning void but working in a different thread
	//so user need to know when it finish
	//TODO no estoy un pijo convencido...
	template <typename ErrorType>
	class FutureData<void,ErrorType> : public FutureData_Base,
		private CallbackSubscriptor<::core::CSMultithreadPolicy,const FutureValue<void,ErrorType>&>,
		public std::enable_shared_from_this<FutureData<void,ErrorType>>
	{
		typedef CallbackSubscriptor<::core::CSMultithreadPolicy,const FutureValue<void,ErrorType>&> Subscriptor;
	public:
		typedef FutureValue<void,ErrorType> ValueType;
		typedef typename mpl::TypeTraits< ValueType >::ParameterType ReturnType;

		FutureData(){};
		~FutureData(){};

		template <class F> auto subscribeCallback(F&& f)
		{
			bool trigger = false;
			Lock lck(FutureData_Base::mSC);
			if ( mState != NOTAVAILABLE)
			{
				f(mValue);
			}				
			return Subscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f)
		{
			Lock lck(FutureData_Base::mSC);
			return Subscriptor::unsubscribeCallback(std::forward<F>(f));
		}
		ReturnType getValue() const
		{
			return mValue;
		}
		inline void setValue( void )
		{
			volatile auto protectMe=FutureData<void,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();	
			if ( mState == NOTAVAILABLE)
			{
				FutureData_Base::mState = VALID;
				FutureData_Base::mSC.leave();
				Subscriptor::triggerCallbacks(mValue);
			}else
				FutureData_Base::mSC.leave();
		}
		/**
		* set error info. TAKES OWNSERHIP
		*/
		void setError( const ErrorType& ei )
		{
			volatile auto protectMe=FutureData<void,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();
			if ( mState == NOTAVAILABLE)
			{
				mValue = ei;
				mState = INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}			
			FutureData_Base::mSC.leave();
		};
		void setError( ErrorType&& ei )
		{
			volatile auto protectMe=FutureData<void,ErrorType>::shared_from_this();
			FutureData_Base::mSC.enter();
			if ( mState == NOTAVAILABLE)
			{
				mValue = std::move(ei);
				mState = INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}			
			FutureData_Base::mSC.leave();
		};
		private:
			ValueType mValue;
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
		/**
		* @brief Generic result error codes for future waiting
		*/
		enum EWaitError{
			FUTURE_RECEIVED_KILL_SIGNAL = -1, //!<Wait for Future was interrupted because waiting Process was killed
			FUTURE_WAIT_TIMEOUT = -2, //!<time out expired while waiting for Future
			FUTURE_UNKNOWN_ERROR = -3//!<Unknow error while waiting for Future
		};
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
				
	};
	///@endcond
	template <typename ResultType,typename ErrorType>
	class Future_Common : public Future_Base
	{
	protected:
		Future_Common()
		{
			mData = std::make_shared<FutureData<ResultType,ErrorType>>();
		};
		Future_Common( const Future_Common& f ):Future_Base( f ){}
		Future_Common( Future_Common&& f ):Future_Base( std::move(f) ){}
	public:
		
		inline  typename FutureData<ResultType,ErrorType>::ReturnType getValue() const{ return ((FutureData<ResultType,ErrorType>*)mData.get())->getValue();}		
		template <class F> auto subscribeCallback(F&& f) const						
		{
			//@todo no me gusta un pijo este cast, pero necesito que el subscribe actúa como mutable
			return const_cast<Future_Common<ResultType,ErrorType>*>(this)->getData().subscribeCallback( std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f) const
		{
			return const_cast<Future_Common<ResultType,ErrorType>*>(this)->getData().unsubscribeCallback( std::forward<F>(f));
		}
	private:
		inline const FutureData<ResultType,ErrorType>& getData() const{ return *(FutureData<ResultType,ErrorType>*)mData; }
		inline FutureData<ResultType,ErrorType>& getData(){ return *(FutureData<ResultType,ErrorType>*)mData.get(); }
	};
	template <typename ResultType,typename ErrorType = ::core::ErrorInfo>
	class Future : public Future_Common<ResultType,ErrorType>
	{
	public:
		typedef typename FutureData<ResultType,ErrorType>::ValueType ValueType;
		typedef typename FutureData<ResultType,ErrorType>::ReturnType ReturnType;
		Future(){};
		Future( const Future& f ):Future_Common<ResultType,ErrorType>(f){};
		Future( Future&& f ):Future_Common<ResultType,ErrorType>(std::move(f)){};

		template <class F>
		void setValue( F&& value )
		{
		    ((FutureData<ResultType,ErrorType>*)Future_Common<ResultType,ErrorType>::mData.get())->setValue( std::forward<F>(value) ); 
		}	
		template <class F>
		void setError( F&& ei )
		{
			((FutureData<ResultType,ErrorType>*)Future_Common<ResultType,ErrorType>::mData.get())->setError( std::forward<F>(ei) ); 
		}
	};

	//specialization for void
	template <typename ErrorType>
	class Future<void,ErrorType> : public Future_Common<void,ErrorType>
	{
	public:
		typedef typename FutureData<void,ErrorType>::ValueType ValueType;
		typedef typename FutureData<void,ErrorType>::ReturnType ReturnType;
		Future(){};
		Future(const Future& f):Future_Common<void,ErrorType>(f){};	
		Future(Future&& f):Future_Common<void,ErrorType>(std::move(f)){};	
		inline void setValue( void ){ ((FutureData<void,ErrorType>*)Future_Base::mData.get())->setValue( ); }		
		template <class F>
		void setError( F&& ei )
		{
			((FutureData<void,ErrorType>*)Future_Base::mData.get())->setError( std::forward<F>(ei) ); 
		}
	};


}
