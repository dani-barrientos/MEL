#include "test.h"
#include <TestManager.h>
using tests::TestManager;
#include <parallelism/ThreadPool.h>
using namespace parallelism;
#include <parallelism/For.h>
using parallelism::For;
#include <array>
#include <core/Timer.h>
using core::Timer;
#include <vector>
#include <random>
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
            mVector[mIdx] = mDist(mGenerator);
          //  ++sAt; pruebas de acceso a recurso compartido
        }
    private:
        size_t mIdx;
        TDist mDist;
        std::mt19937& mGenerator;
};

static int test()
{
	int result = 0;
    Timer timer;
    constexpr int n = 100000;
    constexpr int tries = 500;
    std::vector<float> values(n,0.f);
    std::array<std::unique_ptr<Base>,n> objs;
     std::random_device rd;
    std::mt19937 gen(rd());
    Hija::TDist dist(1.0,100.0);
    for(size_t i = 0;i<n;++i)
    {
        objs[i].reset(new Hija(values,i,dist,gen));
    }
    ThreadPool::ThreadPoolOpts opts;
    //opts.nThreads = 1;    
    ThreadPool myPool(opts);

    
    auto t0 = timer.getMilliseconds();

    for(int i = 0; i < tries; ++i )
        for(auto& obj:objs)
        {
            obj->f();
        }
    spdlog::info("Resultado normal. Time: {}",timer.getMilliseconds()-t0);
    ThreadPool::ExecutionOpts exopts;
    exopts.useCallingThread = false;
    int begin = 0;
    int end = n;
    for(auto& v:values)
        v=0.f;
    t0 = timer.getMilliseconds();
    for(int i = 0; i < tries; ++i )
        For(&myPool,exopts,begin,end,[&objs](int idx)
            {
                objs[idx]->f();
              //  spdlog::debug("it {}",idx);
            }).wait();

    spdlog::info("Resultado parallel. Time: {}",timer.getMilliseconds()-t0);
    //auto d = ::parallelism::Distance<::mpl::TypeTraits<::mpl::TypeTraits<int&&>::UnReferenced>::isArith>::get(0,5);
    //auto d = ::parallelism::Distance<::mpl::TypeTraits<std::decay<int&&>::type>::isArith>::get(0,5);
    //std::iterator_traits<int>::value_type v;
    return result;
}
void test_parallelism::registerTest()
{
    TestManager::getSingleton().registerTest(TEST_NAME,"parallelism tests",test);
}
/*
deque<int> cola;
    int maximum = 0;
    for(int i = 0; i < n; i++)
    {        
        if ( arr[i] > maximum)
            maximum = arr[i];
        cola.push_front(arr[i]);        
        if ( cola.size() == k )
        {
            cout << maximum << ' ';
            if ( maximum == cola.back())
            {
                cola.pop_back();
                maximum = *std::max_element(cola.begin(),cola.end());           
            }else            
                cola.pop_back();
        }
    }
    cout << '\n';
    */