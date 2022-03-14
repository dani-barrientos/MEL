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
	};
	
	namespace _private
	{
		struct NotAvailable
		{

		};
		template <class T> struct is_ref_wrap
		{
			enum {value = false};
		};
		template <class T> struct is_ref_wrap<std::reference_wrapper<T>>
		{
			enum { value = true};
		};
		template <class T> struct return_type
		{
			using type = T&;
		};
		template <class T> struct return_type<std::reference_wrapper<T>>
		{
			using type = typename std::reference_wrapper<T>::type&;
		};
	}
	template <class T,class ErrorType = ::core::ErrorInfo> class FutureValue : public std::variant<_private::NotAvailable,T,ErrorType>
	{
	
		static constexpr size_t ValidIdx = 1;
		typedef std::variant<_private::NotAvailable,T,ErrorType> Base;
		public:
			FutureValue(){}
			FutureValue(const T& v):Base(v){}
			FutureValue(T&& v):Base(std::move(v)){}
			FutureValue(const ErrorType& err):Base(err){}
			FutureValue(ErrorType&& err):Base(std::move(err)){}
			FutureValue(const FutureValue& v):Base(v){}
			FutureValue(FutureValue&& v):Base(std::move(v)){}
			/**
			 * @brief get if has valid value
			 */
			bool isValid() const{ return Base::index() == ValidIdx;}
			bool isAvailable() const{ return Base::index() != 0;}
			// wrapper for std::get.  Same rules as std::Get, so bad_variant_access is thrown if not a valid value
			typename _private::return_type<T>::type value()
			{
				return std::get<T>(*this);
			}
			const typename _private::return_type<T>::type value() const
			{
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
			auto& operator=(const FutureValue& v)
			{
				Base::operator=(v);
				return *this;
			}
			auto& operator=( FutureValue& v)
			{
				Base::operator=(v);
				return *this;
			}
			auto& operator=(FutureValue&& v)
			{
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
	template <typename T,typename ErrorType>
	class FutureData : public FutureData_Base,
					private CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<T,ErrorType>&>,
					public std::enable_shared_from_this<FutureData<T,ErrorType>>
	{		
	public:
		typedef FutureValue<typename 
        std::conditional<
            std::is_reference<T>::value,
            std::reference_wrapper<typename std::remove_reference<T>::type>,
            T>::type> ValueType;
		
	/*---
		typedef FutureValue<ResultType,ErrorType> ValueType;
		//typedef typename mpl::TypeTraits< ValueType >::ParameterType ReturnType;
		typedef ValueType& ReturnType;*/
		typedef CallbackSubscriptor<::core::CSMultithreadPolicy, ValueType&> Subscriptor;
		/**
		* default constructor
		*/
		FutureData(){};
		~FutureData(){};
		/**
		* constructor from value. It becomes valid. For casting purposes. 
		*/
		template <class U> FutureData( U&& value):mValue(std::forward<U>(value))
		{
			FutureData_Base::mState = VALID;
		}
		ValueType& getValue(){ return mValue;}
        const ValueType& getValue()const{ return mValue;}
		template <class U>
		void setValue(U&& value)
		{
			mValue = std::forward<U>(value);
		}
	/*
		FutureData( const ResultType& value ):
			mValue(value){
			FutureData_Base::mState = VALID;
		}
		FutureData( ResultType value ):
			mValue(value){
			FutureData_Base::mState = VALID;
		}
		FutureData( ResultType&& value ):
			mValue(std::move(value)){
			FutureData_Base::mState = VALID;
		}
		ReturnType getValue()
		{
			return mValue;
		}
		const ReturnType getValue() const
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
		}	*/	
		void setError( const ErrorType& ei )
		{
			volatile auto protectMe=FutureData<T,ErrorType>::shared_from_this();
			core::Lock lck(FutureData_Base::mSC);
			if ( mState == NOTAVAILABLE)
			{
				mValue = ei;
				mState = INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}
		};
		void setError( ErrorType&& ei )
		{
			volatile auto protectMe=FutureData<T,ErrorType>::shared_from_this();
			core::Lock lck(FutureData_Base::mSC);

			if ( mState == NOTAVAILABLE)
			{
				mValue = std::move(ei);
				mState = INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}

		};
		void assign(const ValueType& val)
		{
			core::Lock lck(FutureData_Base::mSC);
			mValue = val;
			if (mValue.isAvailable())
			{
				mState = val.isValid()?VALID:INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}else
				mState = NOTAVAILABLE;

		}
		void assign(ValueType& val)
		{
			core::Lock lck(FutureData_Base::mSC);
			mValue = std::move(val);
			if (mValue.isAvailable())
			{
				mState = val.isValid()?VALID:INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}else
				mState = NOTAVAILABLE;
		}
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
		private CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<void,ErrorType>&>,
		public std::enable_shared_from_this<FutureData<void,ErrorType>>
	{
		typedef CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<void,ErrorType>&> Subscriptor;
	public:
		typedef FutureValue<void,ErrorType> ValueType;
		typedef typename mpl::TypeTraits< ValueType >::ParameterType ReturnType;

		FutureData(){};
		//overload to inicializce as valid
		FutureData(int)
		{
			FutureData_Base::mState = VALID;
		};
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
		void assign(const ValueType& val)
		{
			core::Lock lck(FutureData_Base::mSC);
			mValue = val;
			if (mValue.isAvailable())
			{
				mState = val.isValid()?VALID:INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}else
				mState = NOTAVAILABLE;
		}
		void assign(ValueType& val)
		{
			core::Lock lck(FutureData_Base::mSC);
			mValue = std::move(val);
			if (mValue.isAvailable())
			{
				mState = val.isValid()?VALID:INVALID;
				Subscriptor::triggerCallbacks(mValue);
			}else
				mState = NOTAVAILABLE;
		}
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
	template <typename T,typename ErrorType>
	class Future_Common : public Future_Base
	{
	protected:
		Future_Common()
		{
			mData = std::make_shared<FutureData<T,ErrorType>>();
		};
		Future_Common( const Future_Common& f ):Future_Base( f ){}
		Future_Common( Future_Common&& f ):Future_Base( std::move(f) ){}
		Future_Common& operator= ( const Future_Common& f )
		{
			Future_Base::operator=(f);
			return *this;
		};		
		Future_Common& operator= (  Future_Common&& f )
		{
			Future_Base::operator=(f);
			return *this;
		};
	public:
		
		inline  typename FutureData<T,ErrorType>::ValueType getValue() const{ return ((FutureData<T,ErrorType>*)mData.get())->getValue();}		
		inline  const typename FutureData<T,ErrorType>::ValueType getValue() { return ((FutureData<T,ErrorType>*)mData.get())->getValue();}		
		void assign( const typename FutureData<T,ErrorType>::ValueType& val)
		{
			getData().assign(val);
		}
		void assign(  typename FutureData<T,ErrorType>::ValueType&& val)
		{
			getData().assign(std::move(val));
		}
		template <class F> auto subscribeCallback(F&& f) const						
		{
			//@todo no me gusta un pijo este cast, pero necesito que el subscribe actúa como mutable
			return const_cast<Future_Common<T,ErrorType>*>(this)->getData().subscribeCallback( std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f) const
		{
			return const_cast<Future_Common<T,ErrorType>*>(this)->getData().unsubscribeCallback( std::forward<F>(f));
		}
	private:
		inline const FutureData<T,ErrorType>& getData() const{ return *(FutureData<T,ErrorType>*)mData; }
		inline FutureData<T,ErrorType>& getData(){ return *(FutureData<T,ErrorType>*)mData.get(); }
	};
	template <typename T,typename ErrorType = ::core::ErrorInfo>
	class Future : public Future_Common<T,ErrorType>
	{
	public:
		typedef typename FutureData<T,ErrorType>::ValueType ValueType;
		Future(){};
		Future( const Future& f ):Future_Common<T,ErrorType>(f){};
		Future( Future&& f ):Future_Common<T,ErrorType>(std::move(f)){};

		Future& operator= ( const Future& f )
		{
			Future_Common<T,ErrorType>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			Future_Common<T,ErrorType>::operator=(f);
			return *this;
		};
		template <class F>
		void setValue( F&& value )
		{
		    ((FutureData<T,ErrorType>*)Future_Common<T,ErrorType>::mData.get())->setValue( std::forward<F>(value) ); 
		}	
		template <class F>
		void setError( F&& ei )
		{
			((FutureData<T,ErrorType>*)Future_Common<T,ErrorType>::mData.get())->setError( std::forward<F>(ei) ); 
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
		Future& operator= ( const Future& f )
		{
			Future_Common<void,ErrorType>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			Future_Common<void,ErrorType>::operator=(f);
			return *this;
		};
		
		inline void setValue( void ){ ((FutureData<void,ErrorType>*)Future_Base::mData.get())->setValue( ); }		
		template <class F>
		void setError( F&& ei )
		{
			((FutureData<void,ErrorType>*)Future_Base::mData.get())->setError( std::forward<F>(ei) ); 
		}		
	};


}
