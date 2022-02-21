#pragma once
#include <BaseTest.h>
namespace test_callbacks
{
    class TestCallbacks : public tests::BaseTest
    {
        public:
            static const std::string TEST_NAME;
            static void registerTest();        
            TestCallbacks():BaseTest(TEST_NAME){}
        protected:
        /**
         * @brief execution tests
         * commandline options
	    *   -n: test number (0->test launch; 1->test for)
  	    *   -ls: loop size for test number 1
         * @return int 
         */
        int onExecuteTest() override;
        int onExecuteAllTests() override;
    };
    
}