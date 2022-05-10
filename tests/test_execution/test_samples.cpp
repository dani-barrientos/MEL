#include "test_samples.h"
#include <tasking/utilities.h>
#include <execution/RunnableExecutor.h>
#include <execution/ThreadPoolExecutor.h>
#include <functional>
using std::string;
#include <memory>
using namespace mel;
using namespace std::literals::string_literals;

//code for the samples in the documentation
template <class ExecutorType> void _sampleBasic(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();
    th->fireAndForget(
            [ex]() mutable {
              try {
                auto res = ::mel::tasking::waitForFutureMThread(
                    mel::execution::launch(
                        ex, [](float param) noexcept { return param + 6.f; },
                        10.5f) |
                    mel::execution::next([](float param) noexcept {
                      return std::to_string(param);
                    }) |
					mel::execution::getExecutor([](auto ex) noexcept {
						if constexpr(execution::ExecutorTraits<decltype(ex)>::has_parallelism)
							mel::text::info("Current executor supports true parallelism. Next job will be executed parallelized");
						else
							mel::text::info("Current executor doesn't support true parallelism. Next job will be executed sequentially");
                    }) |
                    mel::execution::parallel(
                        [](const string &str) noexcept{
                          mel::text::info("Parallel 1. {}", str + " hello!");
                        },
                        [](const string &str) noexcept{
                          mel::text::info("Parallel 2. {}", str + " hi!");
                        },
                        [](const string &str) noexcept{
                          mel::text::info("Parallel 2. {}", str + " whats up!");
                        }));
                if (res.isValid()) {
                  ::mel::text::info("Result value = {}", res.value());
                }
              } catch (core::WaitException &e) {
                ::mel::text::error("Error while waiting: Reason={}", e.what());
              } catch (...) {
                ::mel::text::error("Error while waiting: Unknown reason={}");
              }
            },
            0, ::mel::tasking::Runnable::killFalse);
}
template <class ExecutorType> void _sampleReference(ExecutorType ex)
{	
	string str = "Hello";
	auto th = ThreadRunnable::create();
	/*th->fireAndForget([ex,&str] () mutable
	{
		auto res = ::mel::tasking::waitForFutureMThread(
			execution::launch(ex,[](string& str) noexcept ->string&
			{	
				//First job
				str += " Dani.";
				return str;
			},std::ref(str))
			| mel::execution::next( [](string& str) noexcept -> string&
			{
				//Second job 
				str+= " How are you?";
				return str;
			})
			| mel::execution::next( [](string& str ) noexcept
			{
				//Third job
				str += "Bye!";
				return str;
			})
			| mel::execution::next( [](string& str ) noexcept
			{
				//Fourth job
				str += "See you!";
				return str;
			})
		);
		if (res.isValid())
		{
			::text::info("Result value = {}",res.value());
			::text::info("Original str = {}",str);
		}
	},0,::tasking::Runnable::killFalse);
	*/
	th->fireAndForget([ex,&str] () mutable
	{
		auto res = ::mel::tasking::waitForFutureMThread(
			execution::start(ex)
			| mel::execution::inmediate(std::ref(str))
			| mel::execution::next([](string& str) noexcept ->string&
			{	
				//First job
				str += " Dani.";
				return str;
			})
			| mel::execution::next( [](string& str) noexcept -> string&
			{
				//Second job 
				str+= " How are you?";
				return str;
			})
			| mel::execution::next( [](string& str ) noexcept
			{
				//Third job
				str += "Bye!";
				return str;
			})
			| mel::execution::next( [](string str ) noexcept
			{
				//Fourth job
				str += "See you!";
				return str;
			})
		);
		if (res.isValid())
		{
			::text::info("Result value = {}",res.value());
			::text::info("Original str = {}",str);
		}
	},0,::tasking::Runnable::killFalse);

}
template <class ExecutorType> void _sampleError1(ExecutorType ex)
{	
	string str = "Hello";
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex,&str] () mutable
	{
		try
		{
		auto res = ::mel::tasking::waitForFutureMThread(
			execution::launch(ex,[](string& str) noexcept ->string&
			{	
				//First job
				str += " Dani.";
				return str;
			},std::ref(str))
			| mel::execution::next( [](string& str) -> string&
			{
				//Second job 
				/*if ( ::mel::tasking::Process::wait(1000) != ::mel::tasking::Process::ESwitchResult::ESWITCH_OK)
					return str;*/
				str+= " How are you?";
				throw std::runtime_error("This is an error");				
			})
			| mel::execution::next( [](string& str ) noexcept
			{
				//Third job. Will never be executed!!!
				str += "Bye!";
				return str;
			})
			| mel::execution::next( [](string str ) noexcept
			{
				//Fourth job. Will never be executed
				str += "See you!";
				return str;
			})
			
		);
			::text::info("Result value = {}",res.value());
		}
		catch(core::WaitException& e)
		{
			::text::error("Some error occured!! Code= {}, Reason: {}",(int)e.getCode(),e.what());
		}
		catch(std::exception& e)
		{
			::text::error("Some error occured!! Reason: {}",e.what());
		}
		::text::info("Original str = {}",str);
	},0,::tasking::Runnable::killFalse);
}
template <class ExecutorType> void _sampleError2(ExecutorType ex)
{	
	string str = "Hello";
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex,&str] () mutable
	{
		try
		{
			auto res = ::mel::tasking::waitForFutureMThread(
				execution::launch(ex,[](string& str) noexcept ->string&
				{	
					//First job
					str += " Dani.";
					return str;
				},std::ref(str))
				| mel::execution::next( [](string& str) -> string&
				{
					//Second job 
					str+= " How are you?";
					throw std::runtime_error("This is an error");
				})
				| mel::execution::next( [](string& str ) noexcept
				{
					//Third job. Will never be executed!!!
					str += "Bye!";
					return str;
				})
				| mel::execution::catchError( [](std::exception_ptr err)
				{
					return "Error caught!! ";
				})
				| mel::execution::next( [](string str ) noexcept
				{
					//Fourth job. 
					str += "See you!";
					return str;
				})
			);
			::text::info("Result value = {}",res.value());
		}
		catch(core::WaitException& e)
		{
			::text::error("Some error occured!! Code= {}, Reason: {}",(int)e.getCode(),e.what());
		}catch(std::exception& e)
		{
			::text::error("Some error occured!! Reason: {}",e.what());
		}
		::text::info("Original str = {}",str);
	},0,::tasking::Runnable::killFalse);
}
template <class ExecutorType> void _sampleErrorNoException(ExecutorType ex)
{	
	string str = "Hello";
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex,&str] () mutable
	{
		auto res = ::mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(
			execution::launch(ex,[](string& str) noexcept ->string&
			{	
				//First job
				str += " Dani.";
				return str;
			},std::ref(str))
			| mel::execution::next( [](string& str) -> string&
			{
				//Second job 
				str+= " How are you?";
				throw std::runtime_error("This is an error");
				return str;
			})
			| mel::execution::next( [](string& str ) noexcept
			{
				//Third job. Will never be executed!!!
				str += "Bye!";
				return str;
			})
			| mel::execution::next( [](string str ) noexcept
			{
				//Fourth job. 
				str += "See you!";
				return str;
			})
		);
		//need to check if some error occurred
		if (res.isValid())
			::text::info("Result value = {}",res.value());
		else
		{  
			//error branch.
			try
			{
				std::rethrow_exception(res.error());
			}
			catch(core::WaitException& e)
			{
				::text::error("Some error occured!! Code= {}, Reason: {}",(int)e.getCode(),e.what());
			}catch(std::exception& e)
			{
				::text::error("Some error occured!! Reason: {}",e.what());
			}
			::text::info("Original str = {}",str);
		}
	},0,::tasking::Runnable::killFalse);
}

template <class Ex1, class Ex2> void _sampleTransfer(Ex1 ex1,Ex2 ex2)
{	
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex1,ex2] () mutable
	{		
		try
		{
			auto res = ::mel::tasking::waitForFutureMThread(

				execution::start(ex1)
				| mel::execution::inmediate("Hello "s)
				| mel::execution::next( [](const string& str) noexcept
				{
					//Second job 
					text::info("NEXT: {}",str);
					return str + ". How are you?";
				})
				| mel::execution::transfer(ex2)
				| mel::execution::loop(0,10, [](int idx, const string& str) noexcept
				{
					text::info("Iteration {}", str + std::to_string(idx));
				})
				| mel::execution::next( [](const string& str ) noexcept
				{
					//Fourth job. 
					return "See you!";
				})
			);
			::text::info("Result value = {}",res.value());
		}
		catch(core::WaitException& e)
		{
			::text::error("Some error occured!! Code= {}, Reason: {}",(int)e.getCode(),e.what());
		}catch(std::exception& e)
		{
			::text::error("Some error occured!! Reason: {}",e.what());
		}
	},0,::tasking::Runnable::killFalse);
}
template <class ExecutorType1,class ExecutorType2> void _sampleSeveralFlows(ExecutorType1 ex1,ExecutorType2 ex2)
{	
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex1,ex2] () mutable
	{
        //First flow in one of the executors
        auto job1 = mel::execution::start(ex1)
        | mel::execution::next( []() noexcept
        {
            ::mel::tasking::Process::wait(3000); //only possible if the executor as microthreading behaviour
            mel::text::info("First job");
            return "First Job";
        });

        //Second job in the other executor
        auto job2 = mel::execution::start(ex2)
        | mel::execution::parallel( []() noexcept
        {
            ::mel::tasking::Process::wait(300); //only possible if the executor as microthreading behaviour
            mel::text::info("second job, t1");
        },
        []() noexcept
        {
            ::mel::tasking::Process::wait(100); //only possible if the executor as microthreading behaviour
            mel::text::info("second job, t2");
        }
		)
        | mel::execution::next( []() noexcept
        {
            return 10;
        });

        //Third job in the same executor as before
        auto job3 = mel::execution::start(ex2)
        | mel::execution::parallel_convert(
         []() noexcept
        {
            ::mel::tasking::Process::wait(300); //only possible if the executor as microthreading behaviour
            mel::text::info("third job, t1");
            return 5;
        },
        []() noexcept
        {
            ::mel::tasking::Process::wait(100); //only possible if the executor as microthreading behaviour
            mel::text::info("third job, t2");
            return 8.7f;
        }
		);
       
		try
		{
            //on_all need to be executed in a context of some excutor, so one of them is given
            auto res = ::mel::tasking::waitForFutureMThread(execution::on_all(ex2,job1,job2,job3));
            //the result of the job merging is as a tuple, where each elements corresponds to the job in same position
            auto& val = res.value();
            ::mel::text::info("Result value = [{},{},({},{})]",
					std::get<0>(val),  //first job result
					std::get<1>(val),   //second job result
					std::get<0>(std::get<2>(val)),std::get<1>(std::get<2>(val))  //third job result
			);
			
        }
		catch(core::WaitException& e)
		{
			::text::error("Some error occured!! Code= {}, Reason: {}",(int)e.getCode(),e.what());
		}
		
	},0,::tasking::Runnable::killFalse);
}
class MyClass
{
	public:
		float f1(float p1,float p2) noexcept
		{
			return p1+p2;
		};
		string f2(float p) noexcept
		{
			return std::to_string(p);
		}
		void operator()(const string& str) noexcept
		{
			text::info("Parallel operator() {}",str+" hi!");
		}
};

template <class ExecutorType> void _sampleCallables(ExecutorType ex)
{
	auto th = ThreadRunnable::create();
	MyClass obj;
	 using namespace std::placeholders;
	th->fireAndForget([ex,&obj] () mutable
	{

		auto res = ::mel::tasking::waitForFutureMThread(
			execution::launch(ex,
				std::bind(&MyClass::f1,&obj,6.7f,_1),10.5f)
			| mel::execution::next(std::bind(&MyClass::f2,&obj,_1))	
			| mel::execution::parallel( 
				MyClass(),
				[](const string& str) noexcept
				{
					text::info("Parallel 2. {}",str+" hi!");
				},
				[](const string& str) noexcept
				{
					text::info("Parallel 2. {}",str+" whats up!");
				}
			)
		);
		if (res.isValid())
		{
			::text::info("Result value = {}",res.value());
		}
	},0,::tasking::Runnable::killFalse);
}
//showing Perfect forwarding in action
struct SampleClass
{
	float val;
	explicit SampleClass(float v = 0.0):val(v)
	{
		::mel::text::info("SampleClass constructor");
	}	
	SampleClass(const SampleClass& ob)
	{
		val = ob.val;
		::mel::text::info("SampleClass copy constructor");
	}
	SampleClass(SampleClass&& ob)
	{
		val = ob.val;
		ob.val = -1;
		::mel::text::info("SampleClass move constructor");		
	}
	~SampleClass()
	{
		::mel::text::info("SampleClass destructor");
	}
	SampleClass& operator=(const SampleClass& ob)
	{
		val = ob.val;
		::mel::text::info("SampleClass copy operator=");
		return *this;
	}
	SampleClass& operator=(SampleClass&& ob)
	{
		val = ob.val;
		ob.val = -1;
		::mel::text::info("SampleClass move operator=");
		return *this;
	}
};
//perfect forwarding example
template <class ExecutorType> void _samplePF(ExecutorType ex)
{
	SampleClass cl(5);
	auto th = ThreadRunnable::create();
    th->fireAndForget(
		[ex,&cl]() mutable 
		{
			try
			{
				auto ref = mel::tasking::waitForFutureMThread(
					execution::launch(ex, [](SampleClass& input) -> SampleClass&
					{
						//Job 1
						input.val++;
						return input;  //return reference to input argument
					},std::ref(cl))
					| execution::next( [](SampleClass& input)
					{
						//Job 2
						input.val++;
						return input; //returns a copy, because return type is not specified in lambda
					} )
					| execution::next( [](const SampleClass& input)
					{
						//Job 3
						auto ret = input;
						ret.val++;
						return ret;
					})
				);
				mel::text::info("Result value = {}",ref.value().val);
				mel::text::info("Original value = {}",cl.val);
			}catch(...)
			{
				//...
				mel::text::error("_samplePF unknown error!!!");
			}

		},0,::tasking::Runnable::killFalse
	);
}

void test_execution::samples()
{
	text::set_level(text::level::info);
	auto th = ThreadRunnable::create(true);			
	execution::Executor<Runnable> exr(th);
	exr.setOpts({true,false});
	parallelism::ThreadPool::ThreadPoolOpts opts;
	auto myPool = std::make_shared<parallelism::ThreadPool>(opts);
	parallelism::ThreadPool::ExecutionOpts exopts;
	execution::Executor<parallelism::ThreadPool> extp(myPool);
	extp.setOpts({true,true});
    
	_sampleBasic(exr);	
	_sampleBasic(extp);
 	_sampleReference(exr);
 	_sampleError1(exr);
 	_sampleError2(extp);
	_sampleErrorNoException(extp);
 	_sampleTransfer(exr,extp);
    _sampleSeveralFlows(exr,extp);
    _sampleCallables(extp);
   	_samplePF(exr);	
}