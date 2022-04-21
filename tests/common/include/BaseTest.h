#pragma once
#include <string>
#include <sstream>
#include <mutex>
namespace tests
{
    using std::string;
    class BaseTest
    {        
        public:
            BaseTest( string name );
            virtual ~BaseTest(){};
            /**
             * @brief Execute test reading CommandLine to parse concrete options
             * It calls virtual protected onExecuteTests which must be implemented in children
             */
            int executeTest();
            int executeAllTests();
            void setFailed(string extraText = "");
            /**
             * @brief Clear internal text buffer. See addTextToBuffer 
             * 
             */
            void clearTextBuffer();
            enum class LogLevel
            {
                None,
                Debug,
                Info,
                Warn,
                Error,
                Critical
            };            
            void addTextToBuffer(string str,LogLevel ll = LogLevel::None);
            string getBuffer() const;
            //return number of occurences of str in internal text buffer
            size_t findTextInBuffer(string str,bool useRegEx = false);
            /**
             * @brief Convenience function to check if given text appears given number of times
             * This is equivalent to if ( findText(str) != n ) setFailed
             * The current buffer content is shown in the given loglevel
             * @param str string to check
             * @param n number of occurrences should be
             * @param ll LogLevel where to show current buffer. 
             * @param userRegEx str will be treated as a regular expression
             * @return true if ok             
             */
            bool checkOccurrences(string str,size_t n,const char* fileName,int lineNumber,LogLevel ll = LogLevel::None ,bool useRegEx = false);
            
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
            std::ostringstream mTextBuffer;
            std::mutex mCS;
    };
}