#include <BaseTest.h>
#include <iostream>
#include <text/logger.h>
using tests::BaseTest;
using std::string;

BaseTest::BaseTest( string name ):mName(std::move(name)){}
int BaseTest::executeTest()
{
    return onExecuteTest();
}
int BaseTest::executeAllTests()
{
    return onExecuteAllTests();
}
void BaseTest::setFailed(string extraText)
{
    std::cout << "Fail "<<extraText<<std::endl;   
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
void BaseTest::clearTextBuffer()
{
    using namespace std::string_literals;
    mTextBuffer.str(""s);
}
void BaseTest::addTextToBuffer(string str,LogLevel ll)
{
    switch (ll)
    {
        case LogLevel::Debug:
            mel::text::debug(str);
            break;
        case LogLevel::Info:
            mel::text::info(str);
            break;
        case LogLevel::Warn:
            mel::text::warn(str);
            break;
        case LogLevel::Error:
            mel::text::error(str);
            break;
        case LogLevel::Critical:
            mel::text::critical(str);
            break;
        default:break;
    }
    auto lck = mel::core::Lock(mCS);
    mTextBuffer << str;//std::move(str);    
}
string BaseTest::getBuffer() const
{
    return mTextBuffer.str();
}
size_t BaseTest::findTextInBuffer(string str,bool useRegEx )
{
    size_t result = 0;
    string buffer = mTextBuffer.str();
    if ( useRegEx )
    {
        //@todo
    }else
    {
        size_t pos = 0;
        pos = buffer.find(str,pos);
        size_t textSize = str.size();
        while( pos != string::npos)
        {
            ++result;
            pos = buffer.find(str,pos+textSize);
        }
    }
    return result;
}
bool BaseTest::checkOccurrences(string str,size_t n,const char* fileName,int lineNumber,LogLevel ll, bool useRegEx)
{
    size_t found;
    if ( (found = findTextInBuffer(str,useRegEx)) != n)
    {
        std::stringstream ss;
        ss<<":text \""<<str<< "\" has been found "<<found <<" times, expected was "<<n <<" . File = "<<fileName<<':'<<lineNumber; 
        setFailed(ss.str());
        switch (ll)
        {
            case LogLevel::Debug:
                mel::text::debug(">>>>>");
                mel::text::debug(mTextBuffer.str());
                mel::text::debug("<<<<<");
                break;
            case LogLevel::Info:
                mel::text::info(">>>>>");
                mel::text::info(mTextBuffer.str());
                mel::text::info("<<<<<");
                break;
            case LogLevel::Warn:
                mel::text::warn(">>>>>");
                mel::text::warn(mTextBuffer.str());
                mel::text::warn("<<<<<");
                break;
            case LogLevel::Error:
                mel::text::error(">>>>>");
                mel::text::error(mTextBuffer.str());
                mel::text::error("<<<<<");
                break;
            case LogLevel::Critical:
                mel::text::critical(">>>>>");
                mel::text::critical(mTextBuffer.str());
                mel::text::critical("<<<<<");
                break;
            default:break;
        }
        return false;
    }else
        return true;
}