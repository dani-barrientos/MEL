#include <parallelism/ThreadPool.h>
using parallelism::ThreadPool;
#include <core/GenericThread.h>
using core::GenericThread;

#include <logging/Logger.h>
using logging::Logger;


ThreadPool::ThreadPool( const ThreadPoolOpts& opts ):
	mLastIndex(-1),
	mPool(nullptr),
	mOpts(opts)
{
	if (opts.nThreads == THREADS_USE_ALL_CORES)
	{
		mNThreads = ::core::getNumProcessors();
	}
	else if (opts.nThreads < 0)
	{
		int n = (int)::core::getNumProcessors() + opts.nThreads;
		if (n < 0)
			n = 0;
		mNThreads = (unsigned int)n;
	}else // > 0
		mNThreads = (unsigned int)opts.nThreads;	
	if (mNThreads > 0)
	{
		bool applyAffinity = (opts.affinity != THREAD_AFFINITY_ALL || opts.forceAffinitty);

		uint64_t pAff = applyAffinity?core::getProcessAffinity():0;
		const uint64_t lastProc = 0x8000000000000000;
		uint64_t coreSelector = lastProc;
		mPool = new Thread*[mNThreads];
		for (unsigned int i = 0; i < mNThreads; ++i)
		{
			Thread* th;
			th = GenericThread::createEmptyThread(false, false);
			::Logger::getLogger()->debugf("ThreadPool. Create thread %ld", 1, th->getThreadId());
			mPool[i] = th;
			if (pAff != 0 && applyAffinity)
			{
				if (!opts.forceAffinitty)
					th->setAffinity(opts.affinity);
				else
				{
					do
					{
						//circular shift
						if (coreSelector & 0x8000000000000000)
							coreSelector = 1;
						else
							coreSelector = coreSelector << 1;
					} while (((coreSelector&opts.affinity) == 0) || ((coreSelector&pAff) == 0));
					if (!th->setAffinity(coreSelector))
						Logger::getLogger()->error("Error setting thread affinity");
				}
			}		
			th->start();  //need to start after setting affinity
		}
		
	}
}
ThreadPool::~ThreadPool()
{
	unsigned int i;
	for ( i = 0; i < mNThreads; ++i )
		mPool[i]->finish();
	for ( i = 0; i < mNThreads; ++i )
	{
		mPool[i]->join();
		delete mPool[i];
	}
	delete[]mPool;
}

size_t ThreadPool::_chooseIndex(const ExecutionOpts& opts) {
	size_t result;
	switch (opts.schedPolicy) 
	{
		case SP_ROUNDROBIN:
			result = (int)((mLastIndex + 1) % mNThreads); 
			break;
		case SP_BESTFIT:
			//@todo choose least busy thread
			result = (int)((mLastIndex + 1) % mNThreads);
			break;
		case SP_EXPLICIT:
			//assert(opts.threadIndex < (size_t)mNThreads); 
			result = opts.threadIndex; 
			break;
		default:
			result = (int)((mLastIndex + 1) % mNThreads);
	}
	return result;
}