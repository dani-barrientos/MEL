#include "test.h"
using test_callbacks::TestCallbacks;
#include <iostream>
#include <core/CallbackSubscriptor.h>
using mel::core::CallbackSubscriptor;
#include <mpl/Int2Type.h>
using mel::mpl::Int2Type;
#include <TestManager.h>
using tests::TestManager;
using namespace mel;

const std::string TestCallbacks::TEST_NAME = "callbacks";
tests::BaseTest* sCurrentTest = nullptr;
void addText(string str)
{
    if ( sCurrentTest )
        sCurrentTest->addTextToBuffer(std::move(str),tests::BaseTest::LogLevel::Info);
}

::core::ECallbackResult f1(float a)
{
    addText("f1");
    return ::mel::core::ECallbackResult::NO_UNSUBSCRIBE;
}
::core::ECallbackResult f2(float& a)
{
    addText("f2");
    return ::mel::core::ECallbackResult::NO_UNSUBSCRIBE;
}


typedef std::pair<Int2Type<0>,CallbackSubscriptor<::mel::core::CSNoMultithreadPolicy,float>> CS1; 
typedef std::pair<Int2Type<1>,CallbackSubscriptor<::mel::core::CSNoMultithreadPolicy,float&>> CS2; 
class Pepe : private CS1,
 private CS2
{
    public:
    template <class T>
     auto subscribe1(T&& f)
     {
         return CS1::second.subscribeCallback(std::forward<T>(f));
     }
    template <class T>
     auto subscribe2(T&& f)
     {
         return CS2::second.subscribeCallback(std::forward<T>(f));
     }
      template <class T>
     auto unsubscribe1(T&& f)
     {
         return CS1::second.unsubscribeCallback(std::forward<T>(f));
     }
    template <class T>
     auto unsubscribe2(T&& f)
     {
         return CS2::second.unsubscribeCallback(std::forward<T>(f));
     }
     auto trigger1(float a)
     {
         return CS1::second.triggerCallbacks(a);
     }
     auto trigger2(float a)
     {
         return CS2::second.triggerCallbacks(a);
     }
};
template <class F> void _subscribe2( Pepe& obj,F&& f)
{
	obj.subscribe2(std::forward<F>(f));
}
int TestCallbacks::onExecuteTest()
{    
    sCurrentTest = this;
    Pepe pp;

    pp.subscribe1(f1);

    const std::function<::mel::core::ECallbackResult(float)> f = [](float v)
    {
        addText("lambda1");
        return ::mel::core::ECallbackResult::UNSUBSCRIBE;
    };
    pp.subscribe1(f);
    int s1 = pp.subscribe1(std::function<::mel::core::ECallbackResult(float)>(f1));
    pp.subscribe2(std::function<::mel::core::ECallbackResult(float&)>(
        [](float)
        {
            addText("lambda2");
            return ::mel::core::ECallbackResult::NO_UNSUBSCRIBE;
        }
    ));
    pp.subscribe2(std::function<::mel::core::ECallbackResult(float&)>(f2));
    _subscribe2(pp,std::function<::mel::core::ECallbackResult(float&)>(
        [](float)
        {
            addText("lambda3");
            return ::mel::core::ECallbackResult::UNSUBSCRIBE;
        }
    ));
    sCurrentTest->clearTextBuffer();
    pp.trigger1(5);
    sCurrentTest->checkOccurrences("f1",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->checkOccurrences("lambda1",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->clearTextBuffer();
    pp.trigger1(5);
    sCurrentTest->checkOccurrences("f1",2,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->checkOccurrences("lambda1",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    pp.unsubscribe1(s1);
    pp.unsubscribe1( std::function<::mel::core::ECallbackResult(float&)>(
        [](float)
        {
            return ::mel::core::ECallbackResult::NO_UNSUBSCRIBE;
        }));
    sCurrentTest->clearTextBuffer();
    pp.trigger1(5);
    sCurrentTest->checkOccurrences("f1",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    pp.unsubscribe1(f1);
    sCurrentTest->clearTextBuffer();
    pp.trigger1(5);
    sCurrentTest->checkOccurrences("f1",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);

    //Second subscriptor
    sCurrentTest->clearTextBuffer();
    pp.trigger2(6);  
    sCurrentTest->checkOccurrences("f2",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->checkOccurrences("lambda2",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->checkOccurrences("lambda3",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->clearTextBuffer();
    pp.trigger2(6);  
    sCurrentTest->checkOccurrences("f2",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->checkOccurrences("lambda2",1,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    sCurrentTest->checkOccurrences("lambda3",0,__FILE__,__LINE__,tests::BaseTest::LogLevel::Info);
    
    
    return 0;
}
/*
core::ECallbackResult f1(int )
{
    return mel::core::ECallbackResult::NO_UNSUBSCRIBE;
}
core::ECallbackResult f2(int,float )
{
    return mel::core::ECallbackResult::NO_UNSUBSCRIBE;
}
core::ECallbackResult f3()
{
    return mel::core::ECallbackResult::NO_UNSUBSCRIBE;
}
core::ECallbackResult f4(int,float,float,int,int,float,int )
{
    return mel::core::ECallbackResult::NO_UNSUBSCRIBE;
}

int test_callbacks::test()
{
    
    CallbackSubscriptor<::mel::core::CSNoMultithreadPolicy,int> cs;   
    CallbackSubscriptor<::mel::core::CSMultithreadPolicy,int,float> cs2;
    CallbackSubscriptor<::mel::core::CSNoMultithreadPolicy,void> cs3;
    CallbackSubscriptor<::mel::core::CSMultithreadPolicy,int,float,float,int,int,float,int> cs4;

    cs.subscribeCallback(f1);
    cs.triggerCallbacks(1);
    cs2.subscribeCallback(f2);    
    cs2.unsubscribeCallback(f2);
    cs2.subscribeCallback( std::function< mel::core::ECallbackResult(int,float)>(f2));
  //  cs2.unsubscribeCallback( std::function< mel::core::ECallbackResult(int,float)>(f2));
    cs2.triggerCallbacks(2,4.5f);
    cs2.triggerCallbacks(2,4.5f);

    cs3.subscribeCallback(f3);
    cs3.triggerCallbacks();

    cs4.subscribeCallback(f4);
    return 0;
}
*/
void TestCallbacks::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"callbacks and callbacksubscriptor tests",std::make_unique<TestCallbacks>());
}

int TestCallbacks::onExecuteAllTests()
{
    return executeTest();
}
