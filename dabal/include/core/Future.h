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
	/**
	* @brief Generic result error codes for future waiting
	*/
// no me gusta: ni nombre, deberái ser EWaitReuslt, ni qye aparezca eso de FUTURE
// no megusta que esté aqui, ¿dodne lo meto?
// no sé cómo encajar el unknownerror
	enum EWaitError{
		FUTURE_WAIT_OK = 0,
		FUTURE_RECEIVED_KILL_SIGNAL = -1, //!<Wait for Future was interrupted because waiting Process was killed
		FUTURE_WAIT_TIMEOUT = -2, //!<time out expired while waiting for Future
		FUTURE_UNKNOWN_ERROR = -3//!<Unknow error while waiting for Future
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
		template <class T> struct creturn_type
		{
			using type = const T&;
		};
		template <class T> struct creturn_type<std::reference_wrapper<T>>
		{
			using type = const typename std::reference_wrapper<T>::type&;
		};
		
	}
	template <class T,class ErrorType = ::core::ErrorInfo> class FutureValue : public std::variant<_private::NotAvailable,T,ErrorType>
	{
	
		static constexpr size_t ValidIdx = 1;
		typedef std::variant<_private::NotAvailable,T,ErrorType> Base;
		public:
			typedef T Type;
			typedef typename _private::return_type<T>::type ReturnType;
			typedef typename _private::creturn_type<T>::type CReturnType;

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
			ReturnType value()
			{
				return std::get<T>(*this);
			}
			CReturnType value() const
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
	struct VoidType
	{
	};
	template <class ErrorType> class FutureValue<void,ErrorType> : public std::variant<_private::NotAvailable,VoidType,ErrorType>
	{
		static constexpr size_t ValidIdx = 1;
		typedef std::variant<_private::NotAvailable,VoidType,ErrorType> Base;
		public:
			typedef void ReturnType;
			typedef void CReturnType;
			FutureValue(){}
			FutureValue(const ErrorType& err):Base(err){}
			FutureValue(ErrorType&& err):Base(std::move(err)){}
			/**
			 * @brief get if has valid value
			 */
			bool isValid() const{ return Base::index() == ValidIdx;}
			/**
			 * @brief get if either the value or eror have been set			 
			 */
			bool isAvailable() const{ return Base::index() != 0;}
			void setValid(){ Base::operator=(VoidType());}
			// wrapper for optional::value().  Same rules as std::Get, so bad_optional_access is thrown if not a valid value			
			const ErrorType& error() const
			{
				return std::get<ErrorType>(*this);
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
	namespace _private
	{
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
						private CallbackSubscriptor<::core::CSMultithreadPolicy,
						FutureValue<typename 
							std::conditional<
								std::is_lvalue_reference<T>::value,
								std::reference_wrapper<typename std::remove_reference<T>::type>,
								T>::type,ErrorType>&>,
						public std::enable_shared_from_this<FutureData<T,ErrorType>>
		{		
		public:
			typedef FutureValue<typename 
			std::conditional<
				std::is_lvalue_reference<T>::value,
				std::reference_wrapper<typename std::remove_reference<T>::type>,
				T>::type,ErrorType> ValueType;
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
			//@todo I should improve this function to avoid code bloat.
			template <class U>
			void setValue(U&& value)
			{
				volatile auto protectMe= FutureData<T,ErrorType>::shared_from_this();
				FutureData_Base::mSC.enter();	
				if ( mState == NOTAVAILABLE)
				{
					mValue = std::forward<U>(value);
					FutureData_Base::mState = VALID;
					FutureData_Base::mSC.leave();
					Subscriptor::triggerCallbacks(mValue);
				}else
					FutureData_Base::mSC.leave();
				
			}
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
			void assign(ValueType&& val)
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
			 * @return ubscription id. -1 if not subscribed, which can occur when function is executed directly if future is valid and
			 * returns an unsubsscription
			 * @warning callback receiver shoudn't wait or do context switch and MUST NOT block
			 * 
			 */
			template <class F> int subscribeCallback(F&& f)
			{
				Lock lck(FutureData_Base::mSC);
				bool continueSubscription = true;
				int result = -1;
				if ( mState != NOTAVAILABLE)
				{
					continueSubscription = (f(mValue) != ::core::ECallbackResult::UNSUBSCRIBE );
				}
				if ( continueSubscription)
					result = Subscriptor::subscribeCallback(std::forward<F>(f)); //shoudn't be neccesary if mstate already available, but for consistency				
				return result;
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
		template <typename ErrorType>
		class FutureData<void,ErrorType> : public FutureData_Base,
			private CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<void,ErrorType>&>,
			public std::enable_shared_from_this<FutureData<void,ErrorType>>
		{
			typedef CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<void,ErrorType>&> Subscriptor;
		public:
			typedef FutureValue<void,ErrorType> ValueType;

			FutureData(){};
			
			//overload to inicializce as valid
			FutureData(int)
			{
				mValue.setValid();
				FutureData_Base::mState = VALID;
			};
			~FutureData(){};

			template <class F> auto subscribeCallback(F&& f)
			{				
				Lock lck(FutureData_Base::mSC);
				bool continueSubscription = true;
				int result = -1;
				if ( mState != NOTAVAILABLE)
				{
					//in reality, this shouldn't be necessary and all callbacks should be unsubscribed always, because the Future only can trigger it
					//once
					continueSubscription = (f(mValue) != ::core::ECallbackResult::UNSUBSCRIBE );
				}
				if ( continueSubscription)
					result = Subscriptor::subscribeCallback(std::forward<F>(f)); //shoudn't be neccesary if mstate already available, but for consistency				
				return result;
			}
			template <class F> auto unsubscribeCallback(F&& f)
			{
				Lock lck(FutureData_Base::mSC);
				return Subscriptor::unsubscribeCallback(std::forward<F>(f));
			}
			ValueType& getValue(){ return mValue;}
			const ValueType& getValue()const{ return mValue;}
			inline void setValue( void )
			{
				volatile auto protectMe=FutureData<void,ErrorType>::shared_from_this();
				FutureData_Base::mSC.enter();	
				if ( mState == NOTAVAILABLE)
				{
					mValue.setValid();
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
			void assign(ValueType&& val)
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
		template <typename T,typename ET>
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
			typedef ET  ErrorType;			
			inline  const typename FutureData<T,ErrorType>::ValueType& getValue() const{ return ((FutureData<T,ErrorType>*)mData.get())->getValue();}		
			inline  typename FutureData<T,ErrorType>::ValueType& getValue() { return ((FutureData<T,ErrorType>*)mData.get())->getValue();}		
			void assign( const typename FutureData<T,ErrorType>::ValueType& val)
			{
				getData().assign(val);
			}
			void assign(  typename FutureData<T,ErrorType>::ValueType&& val)
			{
				getData().assign(std::move(val));
			}
			/**
			 * @brief Subscribe callback to be executed when future is ready (valid or error)
			 * @warning Usually callback is executed in the context of the thread doing setValue, but if Future is already ready
			 * callback will be executed in the context of the thread doing subscription. In general, for this and more reasons, callbacks responses 
			 * should be done buy launchin a task, so decoupliing totally the diferent tasks
			 * 
			 * @tparam F 
			 * @param f 
			 * @return auto 
			 */
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
		///@endcond
	}
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
	
	///@endcond
	
	template <typename T,typename ErrorType = ::core::ErrorInfo>
	class Future : public _private::Future_Common<T,ErrorType>
	{
	public:
		typedef typename _private::FutureData<T,ErrorType>::ValueType ValueType;
		Future(){};
		Future( const Future& f ):_private::Future_Common<T,ErrorType>(f){};
		Future( Future&& f ):_private::Future_Common<T,ErrorType>(std::move(f)){};
		Future(const T& val)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<T,ErrorType>>(val);
		}
		Future(T&& val)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<T,ErrorType>>(std::move(val));
		}
		Future& operator= ( const Future& f )
		{
			_private::Future_Common<T,ErrorType>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			_private::Future_Common<T,ErrorType>::operator=(f);
			return *this;
		};
		template <class F>
		void setValue( F&& value )
		{
		    ((_private::FutureData<T,ErrorType>*)_private::Future_Common<T,ErrorType>::mData.get())->setValue( std::forward<F>(value) ); 
		}	
		template <class F>
		void setError( F&& ei )
		{
			((_private::FutureData<T,ErrorType>*)_private::Future_Common<T,ErrorType>::mData.get())->setError( std::forward<F>(ei) ); 
		}
	};
	template <typename T,typename ErrorType>
	class Future<T&,ErrorType> : public _private::Future_Common<T&,ErrorType>
	{
	public:
		typedef typename _private::FutureData<T&,ErrorType>::ValueType ValueType;
		Future(){};
		Future( const Future& f ):_private::Future_Common<T&,ErrorType>(f){};
		Future( Future&& f ):_private::Future_Common<T&,ErrorType>(std::move(f)){};
		Future(T& val)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<T&,ErrorType>>(val);
		}


		Future& operator= ( const Future& f )
		{
			_private::Future_Common<T,ErrorType>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			_private::Future_Common<T,ErrorType>::operator=(f);
			return *this;
		};
		template <class F>
		void setValue( F&& value )
		{
		    ((_private::FutureData<T&,ErrorType>*)_private::Future_Common<T&,ErrorType>::mData.get())->setValue( std::forward<F>(value) ); 
		}	
		template <class F>
		void setError( F&& ei )
		{
			((_private::FutureData<T&,ErrorType>*)_private::Future_Common<T&,ErrorType>::mData.get())->setError( std::forward<F>(ei) ); 
		}
	};

	//specialization for void
	template <typename ErrorType>
	class Future<void,ErrorType> : public _private::Future_Common<void,ErrorType>
	{
	public:
		typedef typename _private::FutureData<void,ErrorType>::ValueType ValueType;
		Future(){};
		//fake initializacion to indicate we want to initialize as valid
		Future(int a)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<void,ErrorType>>(a);
		};
		Future(const Future& f):_private::Future_Common<void,ErrorType>(f){};	
		Future(Future&& f):_private::Future_Common<void,ErrorType>(std::move(f)){};	
		Future& operator= ( const Future& f )
		{
			_private::Future_Common<void,ErrorType>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			_private::Future_Common<void,ErrorType>::operator=(f);
			return *this;
		};
		
		inline void setValue( void ){ ((_private::FutureData<void,ErrorType>*)_private::Future_Base::mData.get())->setValue( ); }		
		template <class F>
		void setError( F&& ei )
		{
			((_private::FutureData<void,ErrorType>*)_private::Future_Base::mData.get())->setError( std::forward<F>(ei) ); 
		}		
	};
	/** @brief wrapper for future value after wait
	* @todo it shouldn't be in this file, but trying to find it a better place
	**/
	template <class T,class E> class WaitResult
	{        
		public:
			WaitResult(const ::core::EWaitError wc,const core::Future<T,E>& f):mWaitResult(wc),mFut(f){}
			bool isValid () const
			{
				return (mWaitResult == ::core::EWaitError::FUTURE_WAIT_OK)?mFut.getValue().isValid():false;
			}
			typename core::Future<T,E>::ValueType::CReturnType value() const{ return mFut.getValue().value();}
			typename core::Future<T,E>::ValueType::ReturnType value(){ return mFut.getValue().value();}
			const E& error() const{ 
				switch (mWaitResult)
				{               
				case ::core::EWaitError::FUTURE_WAIT_OK: //uhm... no cuadra...
					return mFut.getValue().error();
					break;
				case ::core::EWaitError::FUTURE_RECEIVED_KILL_SIGNAL:
					mEI.reset(new E(mWaitResult,"Kill signal received"));
					return *mEI;
					break;
				case ::core::EWaitError::FUTURE_WAIT_TIMEOUT:
					mEI.reset(new E(mWaitResult,"Time out exceeded"));
					return *mEI;
					break;
				case ::core::EWaitError::FUTURE_UNKNOWN_ERROR:
					mEI.reset(new E(mWaitResult,"Unknown error"));
					return *mEI;
					break;
				default:return mFut.getValue().error(); //silent warning
				}                
			}
			//! @brief access internal just if needed to do excepctional things
			const core::Future<T,E>& getFuture() const{ return mFut;}
			core::Future<T,E>& getFuture(){ return mFut;}
		private:
			::core::EWaitError mWaitResult;
			core::Future<T,E> mFut;
			mutable std::unique_ptr<E> mEI; //error info when needed

	};
}
