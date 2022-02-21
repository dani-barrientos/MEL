#include <TestManager.h>
using tests::TestManager;

static unique_ptr<tests::BaseTest> nulltest;
unique_ptr<tests::BaseTest>& TestManager::getTest(const string& test)
{
    auto i = mTests.find(test);
    if ( i != mTests.end()) 
        return i->second.second;
    else
        return nulltest;
}