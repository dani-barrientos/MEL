#include <core/Singleton.h>
#include <unordered_map>
using std::unordered_map;
#include <string>
using std::string;
#include <utility>
#include <BaseTest.h>
#include <memory>
using std::unique_ptr;

namespace tests
{    
    using namespace std::string_literals;
    class TestManager : public ::core::Singleton<TestManager,false,false>
    {
        friend class ::core::Singleton<TestManager,false,false>;
        public:
            typedef unordered_map<std::string,std::pair<std::string,unique_ptr<BaseTest>>> TestsMap;
            template <class Key,class Val> void registerTest(Key&& testName,Val&& doc,unique_ptr<BaseTest>&& testObj);
            unique_ptr<BaseTest>& getTest(const string& test);
            const TestsMap& getTests() const{ return mTests;}
            TestsMap& getTests(){ return mTests;}
        private:
             TestsMap mTests;
    };
    template <class Key,class Val> void TestManager::registerTest(Key&& testName,Val&& doc,unique_ptr<BaseTest>&& testObj)
    {
        if (mTests.find(testName) != mTests.end() )
            throw std::runtime_error("Test "s + testName+"already registered" );
        mTests[testName] = std::make_pair(doc,std::move(testObj));
    }
}