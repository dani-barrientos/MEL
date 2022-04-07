#include <tasking/CriticalSection_mthread.h>
using ::mel::tasking::CriticalSection_mthread;
//using ::mel::tasking::Lock_mthread;
#include <tasking/ProcessScheduler.h>
using ::mel::tasking::ProcessScheduler;
#include <stdexcept>

template <> CriticalSection_mthread<true>::CriticalSection_mthread() :
	mEvent(false, true),
	mOwner(NULL),
	mCount(0)
{
}
template <> bool CriticalSection_mthread<true>::enter()
{
	auto current = ProcessScheduler::getCurrentProcess();
	//remember: this is not multithread, is multi-microthread
	if (mOwner != current)
	{
		if (mEvent.wait() == EEventMTWaitCode::EVENTMT_WAIT_KILL)
			return false;
		mEvent.reset();
		mOwner = current;
	}
	mCount.fetch_add(1,std::memory_order_relaxed);	
	return true;
}
template <> void CriticalSection_mthread<true>::leave()
{
	//if (--mCount == 0)
	if (mCount.fetch_sub(1,std::memory_order_relaxed) == 1 )
	{
		mEvent.set(false);
		mOwner = NULL;
	}
}

CriticalSection_mthread<false>::CriticalSection_mthread() :
	mEvent(false, true),
	mOwner(NULL),
	mCount(0)
{
}
bool CriticalSection_mthread<false>::enter()
{
	auto current = ProcessScheduler::getCurrentProcess();
	//remember: this is not multithread, is multi-microthread
	if (mOwner != current)
	{
		if (mEvent.wait() == EEventMTWaitCode::EVENTMT_WAIT_KILL)
			return false;
		mEvent.reset();
		mOwner = current;
	}
	++mCount;
	return true;
}
void CriticalSection_mthread<false>::leave()
{
	if (--mCount == 0)
	{
		mEvent.set(false);
		mOwner = NULL;
	}
}

// Lock_mthread::Lock_mthread(CriticalSection_mthread& cs) :mCS(cs)
// {
// 	if (!cs.enter())
// 		throw std::runtime_error("can't lock critical section");
// }
// Lock_mthread::~Lock_mthread()
// {
// 	mCS.leave();
// }