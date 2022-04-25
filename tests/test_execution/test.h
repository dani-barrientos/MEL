#pragma once
#include <BaseTest.h>
namespace test_execution
{
    class TestExecution : public tests::BaseTest
    {
        public:
            static const std::string TEST_NAME;
            static void registerTest();        
            TestExecution():BaseTest(TEST_NAME){}
        protected:
        /**
         * @brief execution tests
         * commandline options
	    *   -n: test number (0->test launch; 1->test for; 2->advanced sample)
  	    *   -ls: loop size for test number 1
         * @return int 
         */
        int onExecuteTest() override;
        int onExecuteAllTests() override;
    };    
}