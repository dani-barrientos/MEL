#include "test.h"
using test_parallelism::TestParallelism;
#include <TestManager.h>
using tests::TestManager;
#include <parallelism/ThreadPool.h>
using namespace parallelism;
#include <parallelism/For.h>

#include <array>
#include <core/Timer.h>
using mel::core::Timer;
#include <vector>
#include <list>
#include <random>
#include <sstream>
#include <text/logger.h>

const std::string TestParallelism::TEST_NAME = "parallelism";
static std::atomic<int> sAt = 0;
class Base
{
    public:
        typedef std::vector<float> Collection;

        Base( Collection& col):mVector(col)
        {
        }
        void f(){ operation();}
        virtual void operation()
        {
             
        }
    protected:
        Collection& mVector;
};
class Hija : public Base
{
    public:
        typedef std::uniform_real_distribution<> TDist;
        Hija(Collection& col,size_t idx,TDist dist,std::mt19937& generator):
            Base(col),mIdx(idx),mDist(dist),mGenerator(generator){}
        void operation() override
        {
            //because every object shares same vector and generator, it will hurts concurrency performance
            auto v = mDist(mGenerator);
            mVector[mIdx] = v;
          //  ++sAt; pruebas de acceso a recurso compartido
        }
    private:
        size_t mIdx;
        TDist mDist;
        std::mt19937& mGenerator;
};

class TestResult
{    
    public:
        TestResult(string name,size_t nThreads,unsigned int baseTime):mBaseTime(baseTime),mName(name),mData(nThreads){}
        void addTime(int nThread,unsigned int time){ mData[nThread-1] = time;}
        unsigned int getTime(int nThread) const{ return mData[nThread-1];}
        const string& getName() const{ return mName;}
        unsigned int getBaseTime() const{ return mBaseTime;}
        const auto& getData() const{ return mData;}
    private:
        unsigned int mBaseTime;
        string mName;
        std::vector<unsigned int> mData;//time for each Thread
};
int TestParallelism::onExecuteTest()
{
	int result = 0;
    mel::text::set_level(text::level::info);    
    Timer timer;
    constexpr int n = 100000;
    constexpr int tries = 50;
    constexpr int maxThreads = 20;
    std::map<string,std::unique_ptr<TestResult>> results;

    std::vector<float> values(n,0.f);    
    std::array<std::unique_ptr<Base>,n> objs;
    std::vector<std::unique_ptr<Base>> objs2(n);
    std::list<std::unique_ptr<Base>> objs3;
    std::list<Hija> objs4;
     std::random_device rd;
    std::mt19937 gen(rd());
    Hija::TDist dist(1.0,100.0);
    for(size_t i = 0;i<n;++i)
    {
        objs[i].reset(new Hija(values,i,dist,gen));
        objs2[i].reset(new Hija(values,i,dist,gen));
        objs3.push_front(std::make_unique<Hija>(values,i,dist,gen));
        objs4.emplace_front(values,i,dist,gen);
    }
    constexpr const char* PARALLEL_ARRAY_INDEXED = "Parallel array indexed";
    constexpr const char* PARALLEL_VECTOR_INDEXED = "Parallel vector indexed";
    constexpr const char* PARALLEL_ARRAY_ITERATOR = "Parallel array iterator";
    constexpr const char* PARALLEL_VECTOR_ITERATOR = "Parallel vector iterator";
    constexpr const char* PARALLEL_LIST_ITERATOR = "Parallel list iterator";
    auto numCores = ::mel::core::getNumProcessors();
    mel::text::info("Running test from 1 thread to {}. Number of cores is {}",maxThreads,numCores);
    //go through the diferent collections, in the straight way (monothread)    
    auto t0 = timer.getMilliseconds();
    decltype(t0) elapsed;
    
    for(int i = 0; i < tries; ++i )
        for(auto& obj:objs)
        {
            obj->f();
        }
    elapsed = timer.getMilliseconds()-t0;
    mel::text::info("Straight method (array). Time: {}",elapsed);
    auto test = std::make_unique<TestResult>(PARALLEL_ARRAY_INDEXED,maxThreads,elapsed);
    results[test->getName()]=std::move(test);
    test = std::make_unique<TestResult>(PARALLEL_ARRAY_ITERATOR,maxThreads,elapsed);
    results[test->getName()]=std::move(test);
    t0 = timer.getMilliseconds();
    for(int i = 0; i < tries; ++i )
        for(auto& obj:objs2)
        {
            obj->f();
        }
    elapsed = timer.getMilliseconds()-t0;
    mel::text::info("Straight method (vector). Time: {}",elapsed);
    test = std::make_unique<TestResult>(PARALLEL_VECTOR_INDEXED,maxThreads,elapsed);
    results[test->getName()]=std::move(test);
    test = std::make_unique<TestResult>(PARALLEL_VECTOR_ITERATOR,maxThreads,elapsed);
    results[test->getName()]=std::move(test);
    t0 = timer.getMilliseconds();
    for(int i = 0; i < tries; ++i )
        for(auto& obj:objs3)
        {
            obj->f();
        }
    elapsed = timer.getMilliseconds()-t0;
    mel::text::info("Straight method (list). Time: {}",elapsed);    
    test = std::make_unique<TestResult>(PARALLEL_LIST_ITERATOR,maxThreads,elapsed);
    results[test->getName()]=std::move(test);

    ThreadPool::ThreadPoolOpts opts;      
         
    for(int i = 1; i <= maxThreads;++i)
    {
        mel::text::info("Using {} threads",i);
        if ( i == numCores)
            mel::text::info("Same number of CPU cores"); 
        opts.nThreads = i;    
        ThreadPool myPool(opts);
        ThreadPool::ExecutionOpts exopts;
        exopts.useCallingThread = false;
        exopts.groupTasks = true; //pruebas, con false ahora no me funciona
    // no me parece suficiente mejora, especialmnente en release
    // igual el acceso al vector, qeu es contiguo, sea el problema

        //go through the diferent collections in parallel    
        int begin = 0;
        int end = n;
        for(auto& v:values)
            v=0.f;
        t0 = timer.getMilliseconds();    
        for(int i = 0; i < tries; ++i )
            ::mel::core::waitForBarrierThread(::parallelism::_for(&myPool,exopts,begin,end,[&objs](int idx)
                {
                    objs[idx]->f();
                //  mel::text::debug("it {}",idx);
                })
            );
           
        elapsed = timer.getMilliseconds()-t0;
        mel::text::info("Parallel method indexing (array). Time: {}",elapsed);
        results[PARALLEL_ARRAY_INDEXED]->addTime(i,elapsed);
        t0 = timer.getMilliseconds();
        for(int i = 0; i < tries; ++i )
            ::mel::core::waitForBarrierThread( ::mel::parallelism::_for(&myPool,exopts,begin,end,[&objs2](int idx)
                {
                    objs2[idx]->f();
                //  mel::text::debug("it {}",idx);
                }));            
        elapsed = timer.getMilliseconds()-t0;
        mel::text::info("Parallel method indexing (vector). Time: {}",elapsed);
        results[PARALLEL_VECTOR_INDEXED]->addTime(i,elapsed);
        t0 = timer.getMilliseconds();        
        for(int i = 0; i < tries; ++i )
            ::mel::core::waitForBarrierThread( ::mel::parallelism::_for(&myPool,exopts,objs.begin(),objs.end(),[](decltype(objs)::iterator i  )
                {
                    (*i)->f();
                //  mel::text::debug("it {}",idx);
                }));        
        elapsed = timer.getMilliseconds()-t0;
        mel::text::info("Parallel method with iterators (array). Time: {}",elapsed);
        results[PARALLEL_ARRAY_ITERATOR]->addTime(i,elapsed);
        t0 = timer.getMilliseconds();
        for(int i = 0; i < tries; ++i )
            ::mel::core::waitForBarrierThread( ::mel::parallelism::_for(&myPool,exopts,objs2.begin(),objs2.end(),[](decltype(objs2)::iterator i  )
                {
                    (*i)->f();
                }));
        elapsed = timer.getMilliseconds()-t0;
        mel::text::info("Parallel method with iterators (vector). Time: {}",elapsed);
        results[PARALLEL_VECTOR_ITERATOR]->addTime(i,elapsed);
        
        t0 = timer.getMilliseconds();
        for(int i = 0; i < tries; ++i )
            ::mel::core::waitForBarrierThread( ::mel::parallelism::_for(&myPool,exopts,objs3.begin(),objs3.end(),[](decltype(objs3)::iterator i  )
                {
                    (*i)->f();
                }));
        elapsed = timer.getMilliseconds()-t0;
        mel::text::info("Parallel method with iterators (list). Time: {}",elapsed);
        results[PARALLEL_LIST_ITERATOR]->addTime(i,elapsed);
        
       
        // este no face falta, era por curiosidad    
        // t0 = timer.getMilliseconds();
        
        // for(int i = 0; i < tries; ++i )
        //     For(&myPool,exopts,objs4.begin(),objs4.end(),[](decltype(objs4)::iterator i  )
        //         {
        //             (*i).f();
        //         }).wait();
        // spdlog::info("Parallel method with iterators plain objects (list). Time: {}",timer.getMilliseconds()-t0);
        
    }
    //show results
    mel::text::info("Results:");
    for(const auto& result:results)
    {
        const auto& test = result.second;
        mel::text::info("  Test: {}",test->getName());
        const auto& data = test->getData();
        unsigned int min = std::numeric_limits<unsigned int>::max();
        unsigned int max = 0;
        size_t maxIdx,minIdx;        
        typedef list<std::pair<int,unsigned int>> TList;
        TList ordered;
        for(size_t i=0;i<data.size();++i)
        {
             if ( data[i] >= max )
             {
                 max = data[i];
                 maxIdx = i;
             }
             if (data[i] < min)
             {
                 min = data[i];
                 minIdx = i;
             }
             ordered.push_back({i,data[i]});
        }
        ordered.sort(
            [](const TList::value_type& v1,const TList::value_type& v2)
            {
                return v1.second<v2.second;
            }
        );
        std::stringstream str;
        str.precision(3);
        str<<'\t';
        for(const auto& element:ordered)
        {
            str<< element.second*100.f/test->getBaseTime() << "% ("<<element.first+1<<") ";  //number of threads is plus 1
        }
        mel::text::info(str.str());
        mel::text::info("      Best time = {}% for {} threads",min*100.f/test->getBaseTime(),minIdx+1);
        mel::text::info("      Worst time = {}% for {} threads",max*100.f/test->getBaseTime(),maxIdx+1);
		
    }

    return result;
}
void TestParallelism::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"parallelism tests",std::make_unique<TestParallelism>());
}
int TestParallelism::onExecuteAllTests()
{
    //return executeTest(); //same tests as individual
    return 0; //@todo todavái no tengo estos testes bien montados. De todos modos los testes de execution son más genéricos
}
