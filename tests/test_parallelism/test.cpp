#include "test.h"
#include <TestManager.h>
using tests::TestManager;
#include <parallelism/ThreadPool.h>
using namespace parallelism;
static int test()
{
	int result = 0;
    ThreadPool::ThreadPoolOpts opts;
    ThreadPool myPool(opts);
    return result;
}
void test_parallelism::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"parallelism tests",test);
}