#pragma once
#include <core/CallbackSubscriptor.h>
#include <core/CriticalSection.h>
#include <memory>
namespace parallelism
{
	class Barrier;
	class DABAL_API BarrierData : private core::CallbackSubscriptor<::core::CSNoMultithreadPolicy,const BarrierData&>,
		public std::enable_shared_from_this<BarrierData>
	{
		friend class ::parallelism::Barrier;		
		typedef CallbackSubscriptor<::core::CSNoMultithreadPolicy,const BarrierData&> Subscriptor;
	private:
		BarrierData(size_t nWorkers):mActiveWorkers(nWorkers){}		
		void set();		
		inline int getActiveWorkers() const { return mActiveWorkers; }
		template <class F> auto subscribeCallback(F&& f)
		{
			volatile auto protectMe = shared_from_this();
			Lock lck(mCS);
			if (mActiveWorkers==0)
				f(*this);
			return Subscriptor::subscribeCallback(std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f)
		{
			Lock lck(mCS);
			return Subscriptor::unsubscribeCallback(std::forward<F>(f));
		}
	protected:
		int	mActiveWorkers;  //para implementacion ingenua
		::core::CriticalSection mCS;

	};
	/**
	 * @brief Multithread barrier
	 * 
	 */
	class DABAL_API Barrier
	{
	public:
		explicit Barrier( size_t nWorkers = 0 ):mData( new BarrierData( nWorkers ) )
		{
		}
		Barrier( const Barrier& o2):mData(o2.mData){ }
		Barrier( Barrier&& o2):mData(std::move(o2.mData)){}
		Barrier& operator=(const Barrier& o2){mData = o2.mData;return *this;}
		Barrier& operator=(Barrier&& o2){mData = std::move(o2.mData);return *this;}
		// inline void addWorkers(size_t nWorkers)
		// {
		// 	mData->addWorkers(nWorkers);
		// }
		/**
		* called by each worker to notify barrier was reach
		*/
		inline void set()
		{
			mData->set();
		}
		inline int getActiveWorkers() const
		{ 
			return mData->getActiveWorkers(); 
		}
		template <class F> auto subscribeCallback(F&& f) const
		{
			return const_cast<BarrierData*>(mData.get())->subscribeCallback(std::forward<F>(f));
		}
		template <class F> auto unsubscribeCallback(F&& f) const
		{
			const_cast<BarrierData*>(mData.get())->unsubscribeCallback(std::forward<F>(f));
		}
	private:
		std::shared_ptr<BarrierData>	mData;
	protected:
		Barrier( BarrierData* data );
		
	};
}