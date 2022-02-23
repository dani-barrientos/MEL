#include <BaseTest.h>
#include <iostream>
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
void BaseTest::setFailed()
{
    std::cout << "Fail\n";    
}