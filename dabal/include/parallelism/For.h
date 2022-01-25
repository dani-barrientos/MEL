#pragma once
#include <DabalLibType.h>
#include <parallelism/ThreadPool.h>
#include <parallelism/Barrier.h>
#include <mpl/TypeTraits.h>
#include <iterator>
#include <math.h>
#include <functional>
#include <mpl/Linker.h>
#ifndef _ITERATOR_DEBUG_LEVEL
#define _ITERATOR_DEBUG_LEVEL 0
#endif
#include <tasking/utilities.h>
#include <core/Thread.h>

namespace parallelism
{
	template <bool>
	struct Distance
	{
		template <class I> static typename std::iterator_traits<I>::difference_type get(I& a, I& b)
		{
			return std::distance(a, b);
		}
	};
	template <>
	struct Distance<true>
	{
		template <class I> static I get(I a, I b)
		{
			return b - a;
		}
	};
	template <bool>
	struct Advance
	{
		template <class I> static void get(I& it, int increment)
		{
			std::advance(it, increment);
		}
	};
	template <>
	struct Advance<true>
	{
		template <class I> static void get(I& it, int increment)
		{
			it += increment;
		}
	};
	
	template <bool>
	struct BulkExecute
	{
		template <class I, class F> inline static void execute(int& cont,int nIterations,I i,I, I end,F&& functor,int divisionSize,int leftOver,int increment,int,ThreadPool* tp, const ThreadPool::ExecutionOpts& opts,Barrier& barrier)
		{
			ThreadPool::ExecutionOpts newOpts(opts);
			newOpts.useCallingThread = false;  //only one iteration in calling thread

			bool finish = ((i == end) || (cont > nIterations));
			while (!finish)
			{
				tp->execute(newOpts, barrier, std::function<void()>([ divisionSize, functor, increment, i, cont, nIterations, leftOver]() mutable
				{
					I j = i;
					int size = (cont == nIterations) ? divisionSize + leftOver : divisionSize;
					for (int n = 0; n < size;)
					{
						functor(j);
						if (++n < size)
							Advance<::mpl::TypeTraits<I>::isArith>::get(j, increment);
					}
				}
						)
				);
				if (++cont <= nIterations)  //because an iteration for each thread
				{
					Advance<::mpl::TypeTraits<I>::isArith>::get(i, divisionSize*increment);
				}
				else
					finish = true;
			}
		}
	};
	//specialization whitout using iterators and for not arithmethic iterators
	//@todo ya no vale
	template <>
	struct BulkExecute<true>
	{
		template <class I, class F> static void execute(int& cont, int nIterations, I i,I begin, I end, F&& functor, int divisionSize, int leftOver, int increment, int loopSize, ThreadPool* tp, const ThreadPool::ExecutionOpts& opts, Barrier& barrier)
		{
			if (cont > nIterations)
				return;
			ThreadPool::ExecutionOpts newOpts(opts);
			newOpts.useCallingThread = false;  //only one iteration in calling thread

											   //! @note because of internal horrible behaviour on debug iterators in visual Studio, we need to hold a copy of container to avoid hold any iterator 
			typedef typename ::std::iterator_traits< typename std::decay<I>::type>::value_type ObjType;
			typedef ::std::vector<ObjType*> VType;
			::std::shared_ptr<VType> vCopy(new VType(loopSize));
			int idx = 0;

			for (auto j = begin; j != end; ++j)
			{
				(*vCopy.get())[idx++] = &(*j);
			}
			while (cont <= nIterations)
			{
				tp->execute(newOpts, barrier,
					std::function<void()>([divisionSize, functor, increment, vCopy, cont, nIterations, leftOver]() mutable
				{
					int size = (cont == nIterations) ? divisionSize + leftOver : divisionSize;
					for (int n = 0; n < size; ++n)
					{
						int idx = (cont - 1)*(divisionSize*increment) + n * increment;						
						//@todo no vale porque necesito iterador
						//ObjType* element = (*vCopy)[idx];
						//functor(*element);
					}
				}
						)
				);
				++cont;
			}
		}
	};
	/**
	* parallelized iteration, begin:end (end not included)
	*/
	class For
	{
	private:
		
		Barrier mBarrier;	
	/**
	* uso de incremento de otro tipo (float). 
	* loopSize? entiendo que ser� float tal y como est�, siendo 
	* ahora el calculo de nElements no vale creo porque si son floats ya no es necesario
	*/
	//! @note internal use
	template <class I, class F>	 For(ThreadPool* tp, const ThreadPool::ExecutionOpts& opts, I&& begin, I&& end, F&& functor, int increment, int loopSize)
	{
		typedef typename std::decay<I>::type DecayedIt;
		constexpr bool isArithIterator = ::mpl::TypeTraits<DecayedIt>::isArith;
		int nElements = (loopSize + increment - 1) / increment;  //"manual" ceil, because ceil function fails somtimes in fast floating mode
		size_t nThreads = tp->getNumThreads()+(opts.useCallingThread?1:0);
		if (begin == end)
			return;
		DecayedIt i(begin);

		if (nElements <= (int)nThreads || opts.groupTasks == false || nThreads == 0 )  //more or equal threads than tasks
		{
			//mBarrier.addWorkers(nElements);
			 mBarrier = Barrier(nElements);
			int cont = 0;
			if (opts.useCallingThread)
			{
				++cont;
				if (nElements > 1)
				{
					Advance<isArithIterator>::get(i, increment);
				}
			}
			ThreadPool::ExecutionOpts newOpts(opts);
			newOpts.useCallingThread = false;  //only one iteration in calling thread
			bool finish = ((i == end) || (cont >= nElements));
			while (!finish)
			{
				//tp->execute(newOpts, mBarrier, false, std::bind(typename std::decay<F>::type(functor), std::ref(GetElement<isArithIterator>::get(i)))); //@todo notengo claro que deba usar ref???
				tp->execute(newOpts, mBarrier, std::bind(typename std::decay<F>::type(functor),i)); 
				if (++cont < nElements)
				{
					Advance<isArithIterator >::get(i, increment);
				}
				else
					finish = true;
			}
			if (opts.useCallingThread && nElements > 0)
			{
				functor(begin);
				mBarrier.set();
			}
		}
		else  //less threads than elements to process
		{
			int divisionSize = nElements / nThreads;
			int nIterations = nThreads;  //number of parallel iterations
			int leftOver = nElements % nIterations;
//			logging::Logger::getLogger()->info("Comienzo: divisionSize=%d; nThreads=%d; nElements=%d; leftOver=%d; loopSize=%d,increment=%d", divisionSize, nThreads, nElements, leftOver,loopSize,increment);
		//	mBarrier.addWorkers(nIterations);
			mBarrier = Barrier(nIterations);
			int cont = 1;
			if (opts.useCallingThread)
			{
				if (nIterations > 1)
				{
					Advance<isArithIterator>::get(i, divisionSize*increment);
				}
				++cont;
			}
			//BulkExecute<_ITERATOR_DEBUG_LEVEL != 0 && !isArithIterator >::execute(cont, nIterations, i,std::forward<I>(begin), std::forward<I>(end), std::forward<F>(functor), divisionSize, leftOver, increment, loopSize, tp, opts, mBarrier);
			//@todo arreglar metodo sin iteradores
			BulkExecute<false>::execute(cont, nIterations, i,std::forward<I>(begin), std::forward<I>(end), std::forward<F>(functor), divisionSize, leftOver, increment, loopSize, tp, opts, mBarrier);
		
			if (opts.useCallingThread && nIterations > 0)
			{
				int size = divisionSize;
				typename std::decay<I>::type j = begin;
				for (int n = 0; n < size/* && j != end*/;)
				{
					functor(j);
					if (++n < size)
						Advance<isArithIterator>::get(j, increment);
				}
				mBarrier.set();
			}
		}
	}
	public:
		//template <class I, class F> For(ThreadPool* tp, const ThreadPool::ExecutionOpts& opts, I&& begin, I&& end, F&& functor,int inc = 1) : For(tp, opts, std::forward<I>(begin), std::forward<I>(end), std::forward<F>(functor), inc, Distance<::mpl::TypeTraits<I>::isArith>::get(begin,end) )
		template <class I, class F> For(ThreadPool* tp, const ThreadPool::ExecutionOpts& opts, I begin, I end, F&& functor, int inc = 1) : For(tp, opts, begin, end, std::forward<F>(functor), inc, Distance<::mpl::TypeTraits<I>::isArith>::get(begin, end))
		//template <class I, class F> For(ThreadPool* tp, const ThreadPool::ExecutionOpts& opts, I begin, I end, F&& functor, int inc = 1) : For(tp, opts, begin, end, std::forward<F>(functor), inc, ::parallelism::Distance<::mpl::TypeTraits<std::decay<I>::type>::isArith>::get(begin, end))
		{
		}
		For() {}
		inline auto wait(unsigned int msecs = ::core::Event::EVENT_WAIT_INFINITE ) const
		{
			return ::core::waitForBarrierThread(mBarrier,msecs);
		}
		//@todo quitar esto que no me gusta. Aislarlo como para barreras y futures
		inline auto waitAsMThread(unsigned int msecs = ::core::Event::EVENT_WAIT_INFINITE) const
		{
			::tasking::waitForBarrierMThread(mBarrier,msecs);
		}		
	};
};

