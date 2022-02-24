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

void BaseTest::addMeasurement(string name, string value)
{
    std::cout << "<CTestMeasurement type=\"text/string\" name=\""<<name<<"\">"<<value<<"</CTestMeasurement>\n";
}
void BaseTest::addMeasurement(string name, double value)
{
    std::cout << "<CTestMeasurement type=\"numeric/double\" name=\""<<name<<"\">"<<value<<"</CTestMeasurement>\n";
}
//CDash documentation states that any numeric value is treated the same way, so really doesn't matter waht gos after "numeric/", but I introduce this overloads for possible future advances
void BaseTest::addMeasurement(string name, int value)
{
    std::cout << "<CTestMeasurement type=\"numeric/integer\" name=\""<<name<<"\">"<<value<<"</CTestMeasurement>\n";
}