#pragma once
#include <BaseTest.h>
namespace test_parallelism
{
    class TestParallelism : public tests::BaseTest
    {
        public:
            static const std::string TEST_NAME;
            static void registerTest();        
            TestParallelism():BaseTest(TEST_NAME){}
        protected:
        /**
         * @brief Threading tests
                  */
        int onExecuteTest() override;
        int onExecuteAllTests() override;
    };
    
    void registerTest();
    void allTests();
}