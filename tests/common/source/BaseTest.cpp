#include <BaseTest.h>
using tests::BaseTest;
BaseTest::BaseTest( string name ):mName(std::move(name)){}
int BaseTest::executeTest()
{
    return onExecuteTest();
}
int BaseTest::executeAllTests()
{
    return onExecuteAllTests();
}