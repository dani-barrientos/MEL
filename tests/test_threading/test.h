#pragma once
#include <BaseTest.h>
namespace test_threading
{
    class TestThreading : public tests::BaseTest
    {
        public:
            static const std::string TEST_NAME;
            static void registerTest();        
            TestThreading():BaseTest(TEST_NAME){}
        protected:
        /**
         * @brief Tasking tests
         * commandline options
         * -n <number> -> test number:
         * 		0 = microthreading-mono thread
         * 		1 = lots of tasks
         * 		2 = basic Future uses
         * 		3 = testing lock_free scheduler
         * 		4 = microthread+exceptions
         * 		5 = hard Future uses
         * @return int 
         */
        int onExecuteTest() override;
        int onExecuteAllTests() override;
    };
        
}