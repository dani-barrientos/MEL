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
            void setFailed();
            /**
             * @brief Add value to add to dashboard
             * 
             * @param value 
             * Remember -D option must be used at executing ctest. you can set this option in cmaketools, but at this momento, Visual Studio Code nned to be restarted when changed
             */
            static void addMeasurement(string name, string value);
            static void addMeasurement(string name, double value);
            static void addMeasurement(string name, int value);
        protected:
            virtual int onExecuteTest() = 0;
            virtual int onExecuteAllTests() = 0;
        private:
            string mName;
    };
}