#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <mpl/TypeTraits.h>
#include <memory>
#include <core/CriticalSection.h>

#include <string>
#include <memory>
#include <core/CallbackSubscriptor.h>
#include <variant>
#include <exception>
#include <functional>
namespace mel
{
	namespace core
	{
		using mel::core::CriticalSection;
		using mel::mpl::TypeTraits;

		/**
		* @brief Generic result error codes for future waiting
		*/
		enum EWaitError{
			FUTURE_WAIT_OK = 0,
			FUTURE_RECEIVED_KILL_SIGNAL = -1, //!<Wait for Future was interrupted because waiting Process was killed
			FUTURE_WAIT_TIMEOUT = -2, //!<time out expired while waiting for Future
			FUTURE_UNKNOWN_ERROR = -3//!<Unknow error while waiting for Future
		};
		///@cond HIDDEN_SYMBOLS
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
		///@endcond
		/**
		 * @brief Wrapper for value holded by a Future
		 */
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
		///@cond HIDDEN_SYMBOLS
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
		///@endcond
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
				//! use carefully, only for very special cases
				CriticalSection& getMutex(){ return mSC;}
			protected:
				mutable CriticalSection	mSC; 
				EFutureState		mState;

			};
			template <typename T>
			class FutureData : public FutureData_Base,
							public CallbackSubscriptor<::mel::core::CSMultithreadPolicy,
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
				typedef CallbackSubscriptor<::mel::core::CSMultithreadPolicy, ValueType&> Subscriptor;
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
				 * @tparam F functor with signature void (Future::ValueType&)
				 * @param f 
				 * @return unsubscription id. -1 if not subscribed, which can occur when function is executed directly if future is valid and
				 * returns an unsubsscription
				 * @warning callback receiver shoudn't wait or do context switch and MUST NOT block
				 * 
				 */
				template <class F> int subscribeCallback(F&& f)
				{
					Lock lck(FutureData_Base::mSC);
					int result = -1;
					if ( mState != NOTAVAILABLE)
					{
						f(mValue);
					}else
					{
						result = Subscriptor::subscribeCallback( std::function<::mel::core::ECallbackResult (ValueType&)>([f = std::forward<F>(f)](ValueType& vt)
							{
								f(vt);
								return ::mel::core::ECallbackResult::UNSUBSCRIBE;
							})
						);
					}					
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
				public CallbackSubscriptor<::mel::core::CSMultithreadPolicy,FutureValue<void>&>,
				public std::enable_shared_from_this<FutureData<void>>
			{
				typedef CallbackSubscriptor<::mel::core::CSMultithreadPolicy,FutureValue<void>&> Subscriptor;
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
						f(mValue);					
					}else
					{
						result = Subscriptor::subscribeCallback( std::function<::mel::core::ECallbackResult(ValueType&)>(
							[f = std::forward<F>(f)](ValueType& vt)
							{
								f(vt);
								return ::mel::core::ECallbackResult::UNSUBSCRIBE;
							})
						);				
					}
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
					volatile auto protectMe=FutureData<void>::shared_from_this();
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
					volatile auto protectMe=FutureData<void>::shared_from_this();
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

			class FutureDataContainer
			{
				public:
					FutureDataContainer(std::shared_ptr<FutureData_Base> ptr):mPtr(ptr){}
					inline const std::shared_ptr<FutureData_Base>& getPtr() const{ return mPtr;}
					inline std::shared_ptr<FutureData_Base>& getPtr(){ return mPtr;}
					inline void setPtr(const std::shared_ptr<FutureData_Base>& ptr){mPtr = ptr;}
				private:
					std::shared_ptr<FutureData_Base> mPtr;
			};
			/**
			* no templated common data.
			*/
			class Future_Base
			{
			protected:
				std::shared_ptr<FutureDataContainer> mData;
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
					return mData->getPtr()->getValid();
				}
				inline EFutureState getState() const
				{
					return mData->getPtr()->getState();
				}
				//Check if given future points to same data
				inline bool operator == ( const Future_Base& f ) const{ return mData == f.mData; };
						
			};
			
		}
		///@endcond		
		/**
		 * @brief Common code for Futures that doesn't need to be templated
		 * 
		 * @tparam T 
		 */
		template <typename T>
		class Future_Common 
		///@cond HIDDEN_SYMBOLS
		: public _private::Future_Base
		///@endcond
		{
		protected:
			Future_Common()
			{
				mData = std::make_shared<_private::FutureDataContainer>( std::make_shared<_private::FutureData<T>>());
			};
			Future_Common( const Future_Common& f ): _private::Future_Base( f ){}
			Future_Common( Future_Common&& f ): _private::Future_Base( std::move(f) ){}
			Future_Common& operator= ( const Future_Common& f )
			{
				_private::Future_Base::operator=(f);
				return *this;
			};		
			Future_Common& operator= (  Future_Common&& f )
			{
				_private::Future_Base::operator=(f);
				return *this;
			};
		public:
			/**
			 * @brief Get the Value object
			 * @return const ::mel::core::FutureValue
			 */
			inline  const typename _private::FutureData<T>::ValueType& getValue() const{ return static_cast<_private::FutureData<T>*>(mData->getPtr().get())->getValue();}
			/**
			 * @brief Get the Value object
			 * @return const ::mel::core::FutureValue
			 */
			inline  typename _private::FutureData<T>::ValueType& getValue() { return static_cast<_private::FutureData<T>*>(mData->getPtr().get())->getValue();}
			void assign( const typename _private::FutureData<T>::ValueType& val)
			{
				getData().assign(val);
			}
			void assign( typename _private::FutureData<T>::ValueType&& val)
			{
				getData().assign(std::move(val));
			}
			/**
			 * @brief Makes this Future to point to the same value as the given Future
			 * @details This ways, both futures will share the same value/error. If input Future is already set at that moment, 
			 * callbacks are triggeres as usual. It's improtante to note that *all futures* sharing same data will change
			 */
			void assign( Future_Common<T>& val)
			{
				core::Lock lck(val.getData().getMutex()); 
				auto ptr = mData->getPtr(); //to avoid destruction before unlock
				core::Lock lck2(getData().getMutex()); 								
				val.getData().append(std::move(getData()));
				setData(val.mData->getPtr());
				if (val.getValue().isAvailable())
					getData().triggerCallbacks(getData().getValue());
			}
			/**
			 * @brief Subscribe callback to be executed when future is ready (valid or error)
			 * @param[in] F Callable with signature `void( \ref ::mel::core::FutureValue& )`
			 * @warning Usually callback is executed in the context of the thread doing setValue, but if Future is already ready
			 * callback will be executed in the context of the thread doing subscription. In general, for this and more reasons, callbacks responses 
			 * should be done buy launchin a task, so decoupliing totally the diferent tasks
			 * @return callback id
			 */
			template <class F> int subscribeCallback(F&& f) const						
			{
				//@todo no me gusta un pijo este cast, pero necesito que el subscribe actúa como mutable
				return const_cast<Future_Common<T>*>(this)->getData().subscribeCallback( std::forward<F>(f));
			}
			/**
			 * @brief Unsubscribe given callback		
			 */
			template <class F> auto unsubscribeCallback(F&& f) const
			{
				return const_cast<Future_Common<T>*>(this)->getData().unsubscribeCallback( std::forward<F>(f));
			}
		private:
			inline const _private::FutureData<T>& getData() const{ return *static_cast<_private::FutureData<T>*>(mData->getPtr().get()); }
			inline _private::FutureData<T>& getData(){ return *static_cast<_private::FutureData<T>*>(mData->getPtr().get()); }
			inline void setData(const std::shared_ptr<_private::FutureData_Base	>& ptr)
			{
				mData->setPtr(ptr);
			}
		};
		/**
		* @brief Represents a value that *maybe* is not present at the current moment.
		* @details This value will be generated by *someone* and will be available at "some moment"
		*  The value is retrieved using various ways:
		*	- pooling in a loop checking for "getValid" and "error"
		*	- using wait functions, as ::mel::tasking::waitForFutureMThread or ::mel::core::waitForFutureThread
		*   - callback notification with \ref Future_Common::subscribeCallback
		* should check getValid, in which case there is an error ( see error )
		*/
		template <typename T> class Future : public Future_Common<T>
		{
		public:
			typedef typename _private::FutureData<T>::ValueType ValueType;
			Future(){};
			Future( const Future& f ):Future_Common<T>(f){};
			Future( Future&& f ):Future_Common<T>(std::move(f)){};
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
				Future_Common<T>::operator=(f);
				return *this;
			};		
			Future& operator= (  Future&& f )
			{
				Future_Common<T>::operator=(f);
				return *this;
			};
			template <class F>
			void setValue( F&& value )
			{
				static_cast<_private::FutureData<T>*>(Future_Common<T>::mData->getPtr().get())->setValue( std::forward<F>(value) );
			}	
			template <class F>
			void setError( F&& ei )
			{
				static_cast<_private::FutureData<T>*>(Future_Common<T>::mData->getPtr().get())->setError( std::forward<F>(ei) ); 
			}
		};
		///@cond HIDDEN_SYMBOLS	
		template <typename T>
		class Future<T&> : public Future_Common<T&>
		{
		public:
			typedef typename _private::FutureData<T&>::ValueType ValueType;
			Future(){};
			Future( const Future& f ):Future_Common<T&>(f){};
			Future( Future&& f ):Future_Common<T&>(std::move(f)){};
			Future(T& val)
			{
				_private::Future_Base::mData = std::make_shared<_private::FutureData<T&>>(val);
			}


			Future& operator= ( const Future& f )
			{
				Future_Common<T>::operator=(f);
				return *this;
			};		
			Future& operator= (  Future&& f )
			{
				Future_Common<T>::operator=(f);
				return *this;
			};
			template <class F>
			void setValue( F&& value )
			{				
				static_cast<_private::FutureData<T&>*>(Future_Common<T&>::mData->getPtr().get())->setValue( std::forward<F>(value) );
			}	
			template <class F>
			void setError( F&& ei )
			{
				static_cast<_private::FutureData<T&>*>(Future_Common<T&>::mData->getPtr().get())->setError( std::forward<F>(ei) ); 
			}
		};

		//specialization for void
		template <>
		class Future<void> : public Future_Common<void>
		{
		public:
			typedef typename _private::FutureData<void>::ValueType ValueType;
			Future(){};
			//fake initializacion to indicate we want to initialize as valid
			Future(int a)
			{
//				_private::Future_Base::mData = std::make_shared<_private::FutureData<void>>(a);
				_private::Future_Base::mData = std::make_shared<_private::FutureDataContainer>( std::make_shared<_private::FutureData<void>>(a));
			};
			Future(const Future& f):Future_Common<void>(f){};	
			Future(Future&& f):Future_Common<void>(std::move(f)){};	
			Future& operator= ( const Future& f )
			{
				Future_Common<void>::operator=(f);
				return *this;
			};		
			Future& operator= (  Future&& f )
			{
				Future_Common<void>::operator=(f);
				return *this;
			};
			
			inline void setValue( void ){ 
				static_cast<_private::FutureData<void>*>(Future_Common<void>::mData->getPtr().get())->setValue( );
				}		
			template <class F>
			void setError( F&& ei )
			{
				static_cast<_private::FutureData<void>*>(Future_Common<void>::mData->getPtr().get())->setError( std::forward<F>(ei) ); 
			}		
		};
		///@endcond
		
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

		/** @brief Wrapper for future value after wait
		 * 
		**/
		template <class T> class WaitResult 
		{        
			public:
				WaitResult(const mel::core::Future<T>& f):mFut(f){}
				bool isValid () const
				{
					return mFut.getValue().isValid();
				}
				typename mel::core::Future<T>::ValueType::CReturnType value() const
				{				
					return mFut.getValue().value();			
				}
				typename mel::core::Future<T>::ValueType::ReturnType value()
				{
					return mFut.getValue().value();
				}
				std::exception_ptr error() const{ 
					return mFut.getValue().error();				
				}
				//! @brief access internal just if needed to do excepctional things
				const mel::core::Future<T>& getFuture() const noexcept{ return mFut;}
				core::Future<T>& getFuture() noexcept{ return mFut;}
			private:
				core::Future<T> mFut;
		};
	}
}