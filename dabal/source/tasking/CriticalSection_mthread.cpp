#include <tasking/CriticalSection_mthread.h>
using ::tasking::CriticalSection_mthread;
using ::tasking::Lock_mthread;
#include <tasking/ProcessScheduler.h>
using ::tasking::ProcessScheduler;
#include <stdexcept>

CriticalSection_mthread::CriticalSection_mthread() :
	mEvent(false, true),
	mOwner(NULL),
	mCount(0)
{
}
bool CriticalSection_mthread::enter()
{
	auto current = ProcessScheduler::getCurrentProcess();
	//remember: this is not multithread, is multi-microthread
	if (mOwner != current)
	{
		if (mEvent.wait() == Event_mthread::EVENTMT_WAIT_KILL)
			return false;
		mEvent.reset();
		mOwner = current;
	}
	++mCount;
	return true;
}
void CriticalSection_mthread::leave()
{
	if (--mCount == 0)
	{
		mEvent.set(false);
		mOwner = NULL;
	}
}

Lock_mthread::Lock_mthread(CriticalSection_mthread& cs) :mCS(cs)
{
	if (!cs.enter())
		throw std::runtime_error("can't lock critical section");
//		throw ::core::IllegalStateException("can't lock critical section");
}
Lock_mthread::~Lock_mthread()
{
	mCS.leave();
}