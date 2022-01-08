#include <TestManager.h>
using tests::TestManager;

TestManager::TestType TestManager::getTest(const string& test)
{
    auto i = mTests.find(test);
    if ( i != mTests.end()) 
        return i->second.second;
    else
        return nullptr;
}