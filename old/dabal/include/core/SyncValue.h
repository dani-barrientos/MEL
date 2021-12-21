#pragma once
#include <core/CriticalSection.h>
#include <mpl/TypeTraits.h>
namespace core
{
	using ::mpl::TypeTraits;
	/**
	* wrapper for values that are accessed concurrently.
	* @todo hay que refinar esta idea, hecha a prisa y corriendo
	*/

	template <class T> class SyncValue
	{
	public:
		SyncValue():mAcquired(false){}
		~SyncValue();
		SyncValue( typename TypeTraits<T>::ParameterType value ):mValue( value ),mAcquired(false)
		{
		}
		inline operator T() const
		{
			return mValue;
		}
		inline T& getValue(){ return mValue; }
		inline const T& getValue() const{ return mValue;};
		//!not for concurrent access. Use exclusiveAssign instead or acquire/drop pair
		SyncValue& operator=( typename TypeTraits<T>::ParameterType value )
		{
			mValue = value;
			return *this;
		}
		bool operator==( typename TypeTraits<T>::ParameterType value ) const
		{
			//TODO debería proteger aqui??
			return mValue == value;
		}
        template <class U>
		friend bool operator==( typename TypeTraits<U>::ParameterType op1, const SyncValue<U>& op2 );
		//! acquire and drop must be paired
		void acquire();
		void drop();
		//assignment using mutex, so more than one thread can assign value concurrently
		SyncValue& exclusiveAssign( typename TypeTraits<T>::ParameterType value )
		{
			Lock lck(mSC);
			mValue = value;
			return *this;
		}
		const T& exclusiveRead() const
		{
			Lock lck(mSC);
			return mValue;
		}
		//! get internal CriticalSection, for advanced purposes
		inline CriticalSection& getCS();
	private:
		T mValue;
		mutable CriticalSection mSC;
		bool mAcquired;
	};
    template <class U>
    bool operator==( typename ::mpl::TypeTraits<U>::ParameterType op1, const ::core::SyncValue<U>& op2 )
    {
        return op1 == op2.mValue;
    }
    
	template <class T> SyncValue<T>::~SyncValue()
	{
		if ( mAcquired )
			mSC.leave();
	}
	template <class T>
	void SyncValue<T>::acquire()
	{
		mSC.enter();
		mAcquired = true;
	}
	template <class T>
	void SyncValue<T>::drop()
	{
		mAcquired = false;
		mSC.leave();
	}
	template <class T>
	CriticalSection& SyncValue<T>::getCS()
	{
		return mSC;
	}
}


 
