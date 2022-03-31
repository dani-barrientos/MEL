#pragma once

#include <mpl/TypeTraits.h>
#include <memory>
#include <core/CriticalSection.h>

#include <string>
#include <memory>
#include <core/CallbackSubscriptor.h>
#include <variant>
#include <optional>
#include <exception>
namespace core
{
	using core::CriticalSection;
	using mpl::TypeTraits;
/*
usar exception para el errorinfo
igual me convenía usar typeerasure para no manejar puntero directamente??
pros/cons uso de exception: 
 - interfaz consistente y poder meter directamente la exceción que se genere->duda...
 - me quitaría la personalizacion del errortype
 DUDA GORDA!!!LAS EXCEPCIONES NORMALMENTE SE CAPTURAN COMO & -> ¿CÓMO GUARDO EL PUNTERO? PORQUE ENCIMA NO PODRÁ HACERSE COPIA POR HERENCIA
->CREO QUE ESTO VA A ESTAR RELACIONADO CON EL STD::CURRENT_EXCEPTION Y EL EXCEPTION_PTR->SEGÚN LA DOC, EL PUNTERO PERMANECE MIENTRAS SE HAGAS REFERENCIA A ÉL
Na, pero el rollo es que no es un tipo concreto, no es un exception ni nada de eso..
lo que puedo hacer es relanzarla después y capturar el tipo concreto. Eso significa que podría poner un try/catch al acceder 
a un future, en vez de hacer un isValid (que podrái tenerlo igual). tal así sea más natural, algo como:
	Future<...> f = "executa algo"
	waitfor(f);
	try
	{
		f.getValue() ...
	}catch()
	{

	}

#include <exception>
#include <stdexcept>
#include <optional>
#include <iostream>

int f1()
{
    throw std::runtime_error("Error runtime");
    //throw std::bad_alloc();
    //throw "dani";
}
void f(std::optional<std::exception_ptr>& output)
{
    try
    {
        f1();
    }
    catch(std::exception& e)
    {
        std::cout << "catch exception\n";
        output = std::current_exception();
    }
    catch(...)
    {
        std::cout << "catch ...\n";
        output = std::current_exception();
    }
}
int main()
{    
    try
    {
        std::optional<std::exception_ptr> opt;
        {
            f(opt);
        }
        if ( opt != nullptr )
        {
            std::rethrow_exception(opt.value());
        }
    }
    catch(std::runtime_error& e)
    {
        std::cout << "Excepcion (runtime_error) capturada al final: "<<e.what();
    }
    catch(std::exception& e)
    {
        std::cout << "Excepcion (exception) capturada al final: "<<e.what();
    }catch(...)
    {
        std::cout << "Excepcion (unknown) capturada al final";
    }
    return 0;
}*/

/*
	struct ErrorInfo
	{
		ErrorInfo(int aErr,std::string aMsg):error(aErr),errorMsg(std::move(aMsg)){}
		//@remarks negative error code is reserved for internal errors
		int		error;  //there was error. Error code. Very simple for now. 
		std::string errorMsg;
	};*/
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
	template <class T> class FutureValue : public std::variant<_private::NotAvailable,T,std::exception_ptr>
	{
	
		static constexpr size_t ValidIdx = 1;
		typedef std::variant<_private::NotAvailable,T,std::exception_ptr> Base;
		public:
			typedef T Type;
			typedef typename _private::return_type<T>::type ReturnType;
			typedef typename _private::creturn_type<T>::type CReturnType;

			FutureValue(){}
			FutureValue(const T& v):Base(v){}
			FutureValue(T&& v):Base(std::move(v)){}
			FutureValue(std::exception_ptr err):Base(err){}			
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
			std::exception_ptr error() const
			{
				return std::get<std::exception_ptr>(*this);
			}
			auto& operator=(const T& v){
				Base::operator=(v);
				return *this;
			}
			auto& operator=(T&& v){
				Base::operator=(std::move(v));
				return *this;
			}
			auto& operator=(std::exception_ptr v){
				Base::operator=(v);
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
	template <> class FutureValue<void> : public std::variant<_private::NotAvailable,VoidType,std::exception_ptr>
	{
		static constexpr size_t ValidIdx = 1;
		typedef std::variant<_private::NotAvailable,VoidType,std::exception_ptr> Base;
		public:
			typedef void ReturnType;
			typedef void CReturnType;
			FutureValue(){}
			FutureValue(std::exception_ptr err):Base(err){}
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
			std::exception_ptr error() const
			{
				return std::get<std::exception_ptr>(*this);
			}			
			auto& operator=(std::exception_ptr v){
				Base::operator=(v);
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
		template <typename T>
		class FutureData : public FutureData_Base,
						private CallbackSubscriptor<::core::CSMultithreadPolicy,
						FutureValue<typename 
							std::conditional<
								std::is_lvalue_reference<T>::value,
								std::reference_wrapper<typename std::remove_reference<T>::type>,
								T>::type>&>,
						public std::enable_shared_from_this<FutureData<T>>
		{		
		public:
			typedef FutureValue<typename 
			std::conditional<
				std::is_lvalue_reference<T>::value,
				std::reference_wrapper<typename std::remove_reference<T>::type>,
				T>::type> ValueType;
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
				volatile auto protectMe= FutureData<T>::shared_from_this();
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
			void setError( std::exception_ptr ei )
			{
				volatile auto protectMe=FutureData<T>::shared_from_this();
				core::Lock lck(FutureData_Base::mSC);
				if ( mState == NOTAVAILABLE)
				{
					mValue = ei;
					mState = INVALID;
					Subscriptor::triggerCallbacks(mValue);
				}
			};
			template <class ET>
			void setError( ET&& ei )
			{
				volatile auto protectMe=FutureData<T>::shared_from_this();
				core::Lock lck(FutureData_Base::mSC);

				if ( mState == NOTAVAILABLE)
				{
					mValue = std::make_exception_ptr(std::forward<ET>(ei));
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
		template <>
		class FutureData<void> : public FutureData_Base,
			private CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<void>&>,
			public std::enable_shared_from_this<FutureData<void>>
		{
			typedef CallbackSubscriptor<::core::CSMultithreadPolicy,FutureValue<void>&> Subscriptor;
		public:
			typedef FutureValue<void> ValueType;

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
				volatile auto protectMe=FutureData<void>::shared_from_this();
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
			void setError( std::exception_ptr ei )
			{
				volatile auto protectMe=FutureData<void>::shared_from_this();
				FutureData_Base::mSC.enter();
				if ( mState == NOTAVAILABLE)
				{
					mValue = ei;
					mState = INVALID;
					Subscriptor::triggerCallbacks(mValue);
				}			
				FutureData_Base::mSC.leave();
			};
			template<class ET>
			void setError( ET&& ei )
			{
				volatile auto protectMe=FutureData<void>::shared_from_this();
				FutureData_Base::mSC.enter();
				if ( mState == NOTAVAILABLE)
				{
					mValue = std::make_exception_ptr(std::forward<ET>(ei));
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
		template <typename T>
		class Future_Common : public Future_Base
		{
		protected:
			Future_Common()
			{
				mData = std::make_shared<FutureData<T>>();
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
			inline  const typename FutureData<T>::ValueType& getValue() const{ return ((FutureData<T>*)mData.get())->getValue();}		
			inline  typename FutureData<T>::ValueType& getValue() { return ((FutureData<T>*)mData.get())->getValue();}		
			void assign( const typename FutureData<T>::ValueType& val)
			{
				getData().assign(val);
			}
			void assign(  typename FutureData<T>::ValueType&& val)
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
				return const_cast<Future_Common<T>*>(this)->getData().subscribeCallback( std::forward<F>(f));
			}
			template <class F> auto unsubscribeCallback(F&& f) const
			{
				return const_cast<Future_Common<T>*>(this)->getData().unsubscribeCallback( std::forward<F>(f));
			}
		private:
			inline const FutureData<T>& getData() const{ return *(FutureData<T>*)mData; }
			inline FutureData<T>& getData(){ return *(FutureData<T>*)mData.get(); }
		};
		///@endcond
	}
	/**
	* @class Future
	*  Represents a value retreived by (possibly) another thread. This value maybe is not present at the moment
	* and can be waited,etc
	* A value is retrieved using various ways:
	*	- polling in a loop checking for "getValid" and "error"
	*	- using wait and waitAsMThread functions
	*   - callback notification with subcribeCallback
	* should check getValid, in which case there is an error ( see error )
	* @todo por ahora un Future no es reaprovechable, es decir, una vez construido o establecido su valor o error
	* ya no puede "resetearse". Ni siquiera s� si tiene sentido
	* Adem�s, no est� pensado para ser usado por varios hilos a la vez (de nuevo, no creo que tenga sentido)
	*/
	///@cond HIDDEN_SYMBOLS
	
	///@endcond
	
	template <typename T>
	class Future : public _private::Future_Common<T>
	{
	public:
		typedef typename _private::FutureData<T>::ValueType ValueType;
		Future(){};
		Future( const Future& f ):_private::Future_Common<T>(f){};
		Future( Future&& f ):_private::Future_Common<T>(std::move(f)){};
		Future(const T& val)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<T>>(val);
		}
		Future(T&& val)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<T>>(std::move(val));
		}
		Future& operator= ( const Future& f )
		{
			_private::Future_Common<T>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			_private::Future_Common<T>::operator=(f);
			return *this;
		};
		template <class F>
		void setValue( F&& value )
		{
		    ((_private::FutureData<T>*)_private::Future_Common<T>::mData.get())->setValue( std::forward<F>(value) ); 
		}	
		template <class F>
		void setError( F&& ei )
		{
			((_private::FutureData<T>*)_private::Future_Common<T>::mData.get())->setError( std::forward<F>(ei) ); 
		}
	};
	template <typename T>
	class Future<T&> : public _private::Future_Common<T&>
	{
	public:
		typedef typename _private::FutureData<T&>::ValueType ValueType;
		Future(){};
		Future( const Future& f ):_private::Future_Common<T&>(f){};
		Future( Future&& f ):_private::Future_Common<T&>(std::move(f)){};
		Future(T& val)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<T&>>(val);
		}


		Future& operator= ( const Future& f )
		{
			_private::Future_Common<T>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			_private::Future_Common<T>::operator=(f);
			return *this;
		};
		template <class F>
		void setValue( F&& value )
		{
		    ((_private::FutureData<T&>*)_private::Future_Common<T&>::mData.get())->setValue( std::forward<F>(value) ); 
		}	
		template <class F>
		void setError( F&& ei )
		{
			((_private::FutureData<T&>*)_private::Future_Common<T&>::mData.get())->setError( std::forward<F>(ei) ); 
		}
	};

	//specialization for void
	template <>
	class Future<void> : public _private::Future_Common<void>
	{
	public:
		typedef typename _private::FutureData<void>::ValueType ValueType;
		Future(){};
		//fake initializacion to indicate we want to initialize as valid
		Future(int a)
		{
			_private::Future_Base::mData = std::make_shared<_private::FutureData<void>>(a);
		};
		Future(const Future& f):_private::Future_Common<void>(f){};	
		Future(Future&& f):_private::Future_Common<void>(std::move(f)){};	
		Future& operator= ( const Future& f )
		{
			_private::Future_Common<void>::operator=(f);
			return *this;
		};		
		Future& operator= (  Future&& f )
		{
			_private::Future_Common<void>::operator=(f);
			return *this;
		};
		
		inline void setValue( void ){ ((_private::FutureData<void>*)_private::Future_Base::mData.get())->setValue( ); }		
		template <class F>
		void setError( F&& ei )
		{
			((_private::FutureData<void>*)_private::Future_Base::mData.get())->setError( std::forward<F>(ei) ); 
		}		
	};
	/**
	 * @brief Exception class generated by WaitResult when wait for future gets an error
	 * 
	 */
	class WaitException: public std::runtime_error
	{
		public:
			WaitException(int code,const std::string& msg): mCode(code),std::runtime_error(msg){}
			WaitException(int code,std::string&& msg): mCode(code),std::runtime_error(std::move(msg)){}
			int getCode() const{ return mCode;}
		private:
			int mCode;
	};

	/** @brief wrapper for future value after wait
	* @todo it shouldn't be in this file, but trying to find it a better place
	**/
	template <class T> class WaitResult 
	{        
		public:
			WaitResult(const core::Future<T>& f):mFut(f){}
			bool isValid () const
			{
				return mFut.getValue().isValid();
			}
			typename core::Future<T>::ValueType::CReturnType value() const
			{				
				return mFut.getValue().value();			
			}
			typename core::Future<T>::ValueType::ReturnType value()
			{
				return mFut.getValue().value();
			}
			std::exception_ptr error() const{ 
				return mFut.getValue().error();				
			}
			//! @brief access internal just if needed to do excepctional things
			const core::Future<T>& getFuture() const noexcept{ return mFut;}
			core::Future<T>& getFuture() noexcept{ return mFut;}
		private:
			core::Future<T> mFut;
	};
}
