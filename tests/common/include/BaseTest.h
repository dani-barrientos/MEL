#pragma once
#include <string>
namespace tests
{
    using std::string;
    class BaseTest
    {        
        public:
            BaseTest( string name );
            /**
             * @brief Execute test reading CommandLine to parse concrete options
             * It calls virtual protected onExecuteTests which must be implemented in children
             */
            int executeTest();
            int executeAllTests();
        protected:
            virtual int onExecuteTest() = 0;
            virtual int onExecuteAllTests() = 0;
        private:
            string mName;
    };
}