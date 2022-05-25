#include "test_samples.h"
#include <tasking/utilities.h>
#include <execution/RunnableExecutor.h>
#include <execution/ThreadPoolExecutor.h>
#include <execution/InlineExecutor.h>
#include <execution/NaiveInlineExecutor.h>
#include <execution/flow/Condition.h>
#include <execution/flow/While.h>
#include <execution/flow/Launch.h>
#include <execution/flow/Loop.h>
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
            ::mel::tasking::Process::wait(3000); //only possible if the executor Has microthreading behaviour
            mel::text::info("First job");
            return "First Job";
        });

        //Second job in the other executor
        auto job2 = mel::execution::start(ex2)
        | mel::execution::parallel( []() noexcept
        {
            ::mel::tasking::Process::wait(300); //only possible if the executor Has microthreading behaviour
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
			//don't return anything, so ignore this tuple element (but it exists with an empty type)   
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
            ::mel::text::info("Result value = [{},{},(void,{})]",
					std::get<0>(val),  //first job result
					std::get<1>(val),   //second job result
					std::get<1>(std::get<2>(val))  //third job result
			);
			
        }
		catch(core::WaitException& e)
		{
			::text::error("Some error occured!! Code= {}, Reason: {}",(int)e.getCode(),e.what());
		}
		catch( mel::execution::OnAllException& e)
		{
			try
			{
				rethrow_exception( e.getCause() );
			}catch(std::exception& e)
			{
				mel::text::error("Error {}",e.what());
			}catch(...)
			{
				mel::text::error("OnAllException. unknown error");
			}
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
template <class ExecutorType> void _sampleFlows1(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();

	//create a flow. input paramter is an ExFuture depending on given executor
	auto flow1 = [](/*execution::ExFuture<ExecutorType,string> */auto input)
	{
		return input | execution::next(
			[](const string& str)
			{
				return str+" Dani";
			}
		);
	};

	th->fireAndForget([ex,flow1] () mutable
	{
        auto res = mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(
				execution::launch(ex,[]()
				{
					return "Hello"s;
				}
			)
			| flow1
			| execution::next([](const string& str)
				{
					return 	str + " Barrientos";
				}
			)
			| 
			[](auto input)
			{
				return input | execution::next([](const string& str)
					{
						return str + ". Bye!!";
					}
				);
			}
		);
		if ( res.isValid())
		{
			text::info("Result = {}",res.value());
		}else
		{
			try
			{
				std::rethrow_exception(res.error());				
			}
			catch(...)
			{
				::text::error("Some error occured!!");
			}
		}
		
	},0,::tasking::Runnable::killFalse);
}
template <class ExecutorType> void _sampleFlowsCondition(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();

	//create a flow. input paramter is an ExFuture depending on given executor
	auto flow0 = [](/*execution::ExFuture<ExecutorType,string> */auto input)
	{
		return input | execution::next(
			[](int val)
			{
				return "Flow0";
			}
		);
	};
	auto flow1 = [](auto input)
	{
		return input | execution::next(
			[](int val)
			{
				return "Flow1";
			}
		);
	};

	th->fireAndForget([ex,flow0,flow1] () mutable
	{
		srand(time(NULL));
        auto res = mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(
				execution::launch(ex,[]()
				{
					return rand()%10;
				}
			)
			| execution::flow::condition(
				[](int val)
				{
					int result = val<5?0:1;
					text::info("Input value = {}. Selecting flow {}",val,result);
					return result;
				},
				flow0,flow1
			)
			| execution::next( [](const string& str)
			{
				return str+" End!";
			})
			
		);
		if ( res.isValid())
		{
			text::info("Result = {}",res.value());
		}else
		{
			try
			{
				std::rethrow_exception(res.error());				
			}
			catch(...)
			{
				::text::error("Some error occured!!");
			}
		}
		
	},0,::tasking::Runnable::killFalse);
}
//flow launching sample
template <class ExecutorType> void _sampleFlowLaunch(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();

	th->fireAndForget([ex] () mutable
	{
		try
		{
			auto finalRes =  mel::tasking::waitForFutureMThread<::mel::core::WaitErrorAsException>(
				execution::launch(ex,[]() noexcept{ return "Starting job!!"s;})
				| execution::parallel_convert(
					[](const string& str)  noexcept
					{
						mel::text::info("parallel_convert task 1, returning void");
					},
					[](const string& str) noexcept
					{
						return str+" Second parallel task"s;
					}
				) 
				| execution::flow::launch( 
					[](auto input) noexcept
					{
						return input | execution::inmediate("hola"s)
						| execution::next( [](const string& s)
							{
								if ( rand()%10 < 5 )
									throw std::runtime_error("parallel:_convert second task. Throwing exception!!");
							}
						); //return void for testing purposes
					},
					[](auto input) noexcept
					{
						return input | execution::inmediate(7.8f);
					}
					,
					[](auto input) noexcept
					{
						return input | execution::next( [](const std::tuple<mel::execution::VoidType,string>& v) noexcept
							{
								return std::get<1>(v).size();
							}
						 );
					}
				)
				| mel::execution::flow::on_all(ex)
			);							
			auto& finalValue = finalRes.value();
			mel::text::info("Final res = (void,{},{})",std::get<1>(finalValue),std::get<2>(finalValue));
		}catch( mel::execution::OnAllException& e)
		{
			try
			{
				rethrow_exception( e.getCause() );
			}catch(std::exception& e)
			{
				mel::text::error("Error {}",e.what());
			}catch(...)
			{
				mel::text::error("OnAllException. unknown error");
			}
		}
		catch(std::exception& e)
		{
			mel::text::error(e.what());
		}catch(...)
		{
			mel::text::error("Unknown error!!!");
		}
		
	},0,::tasking::Runnable::killFalse);
}
//simple doWhile example
template <class ExecutorType> void _sampleWhile(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex] () mutable
	{
		srand(time(NULL));
		int idx = 0;
        auto res = mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(
				execution::start(ex)
				| execution::flow::doWhile( 
					[]( auto input ) noexcept
					{
						return input | execution::next( []() noexcept -> int
						{
							return rand()%10;
						})
						| execution::next( [](int v ) noexcept
						{
							mel::text::info(" new value = {}. Now waiting",v);
							if constexpr(execution::ExecutorTraits<decltype(ex)>::has_microthreading)
							{
								mel::tasking::Process::wait(2500);
							}
							else
								mel::text::info("Current executor doesn't support true parallelism, wait not done");
							mel::text::info(" new value = {}. After wait",v);
						});
					},
					[idx]() mutable noexcept
					{
						if ( ++idx == 4 )
							return false; //finish while
						else
							return true; //continue iterating
					}
				
			)						
		);
		if ( res.isValid())
		{
			text::info("Finished");
		}else
		{
			try
			{
				std::rethrow_exception(res.error());				
			}
			catch(...)
			{
				::text::error("Some error occured!!");
			}
		}
		
	},0,::tasking::Runnable::killFalse);
}
//simple flow::loop example
template <class ExecutorType> void _sampleFlowLoop(ExecutorType ex)
{	
	auto th = ThreadRunnable::create();
	th->fireAndForget([ex] () mutable
	{
		srand(time(NULL));
		int idx = 0;
        auto res = mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(
				execution::start(ex)
				| execution::flow::loop( 0,4,
					[]( int idx, auto input ) noexcept
					{
						return input | execution::next( []() noexcept -> int
						{
							return rand()%10;
						})
						| execution::next( [](int v ) noexcept
						{
							mel::text::info(" new value = {}. Now waiting",v);
							if constexpr(execution::ExecutorTraits<decltype(ex)>::has_microthreading)
							{
								mel::tasking::Process::wait(2500);
							}
							else
								mel::text::info("Current executor doesn't support true parallelism, wait not done");
							mel::text::info(" new value = {}. After wait",v);
						});
					}				
				)
				| execution::next( []{
					mel::text::info(" Flow finished!!");
				})
		);
		if ( res.isValid())
		{
			text::info("Finished");
		}else
		{
			try
			{
				std::rethrow_exception(res.error());				
			}
			catch(...)
			{
				::text::error("Some error occured!!");
			}
		}
		
	},0,::tasking::Runnable::killFalse);
}

template <class ExecutorType> void _sampleFlowChart(ExecutorType ex)
{	
/*	auto th = ThreadRunnable::create();

COSAS:
 - NO CREAR EL VECTOR EN EL LAUNCH, PORQUE LUEGO GENERARÁ COPIA POR CADA ITERACION. CREARLO Y PASARLO POR REF O PUNTERO
 - SI USO EL DOWHILE PARA ITERAR EL VECTOR, ESTO ES SECUENCIAL. ¿TIENE SENTIDO UN "LOOP" PARA FLUJOS? LO QUE MÁS ME JOROBA DE TODO ESTO ES QUE PARECEN COSAS REPETIDAS DEL execution
		PODRÍA USAR EL DOwILE PARA ESO? EL TEMA ES QUE LLAMARLE LOOP SUENA IGUAL QUE EL DOWHILE->POSIBILDIADES:
			- PARALLEL -> EN EL FONDO EL LAUNCH TAMBIÉN ES UN parallelism
			- SOBRECARGAS DEL LAUNCH-> UN PROBLEMA GORDO CON TODO ESTO ES EL RESULTADO...YA NO TIENE SENTIDO DEVOLVER UNA TUPLA
			EN CUALQUIER CASO:
				- DADO QUE CADA FLUJO TERMINARÁ CUANDO SEA...¿CÓMO ESPERAR A TODOS? -> CREO QUE SERÁ FACIL, MÁS O MENOS COMO EL LOOP NORMAL, CON UNA BARRERA
				- EL RESULTADO DEL FLUJO YA NO TENDRÍA SENTIDO...¿IGNORARLO?

		



	th->fireAndForget([ex] () mutable
	{
		srand(time(NULL));
		int idx = 0;
        auto res = mel::tasking::waitForFutureMThread<::mel::core::WaitErrorNoException>(
				execution::launch(ex,[]()
					{
						//create random vector with random size
						size_t vecSize = rand()%20+10;
						auto result = std::vector<int>(vecSize);
						for( auto& v:result)
							v= rand()%5;
						return result;
					}
				)
				si hago esto así, el vector se copiará en cada flujo
				ademñas esto no lanza flujos paralelos.... pero me parece que hacer un  loop para los flows es una burrada...

				| execution::flow::doWhile( 
					[]( auto input ) noexcept
					{
						return input | execution::next( []() noexcept -> int
						{
							return rand()%10;
						})
						| execution::next( [](int v ) noexcept
						{
							mel::text::info(" new value = {}",v);
							if constexpr(execution::ExecutorTraits<decltype(ex)>::has_microthreading)
							{
								mel::tasking::Process::wait(2500);
							}
							else
								mel::text::info("Current executor supports true parallelism. wait not done");
						});
					},
					[idx]() mutable noexcept
					{
						if ( ++idx == 4 )
							return false; //finish while
						else
							return true; //continue iterating
					}
				
			)						
		);
		if ( res.isValid())
		{
			text::info("Finished");
		}else
		{
			try
			{
				std::rethrow_exception(res.error());				
			}
			catch(...)
			{
				::text::error("Some error occured!!");
			}
		}
		
	},0,::tasking::Runnable::killFalse);
	*/
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
	execution::InlineExecutor exInl;
	execution::NaiveInlineExecutor exNaive;	
    
	// _sampleBasic(exr);	
	// _sampleBasic(extp);
 	// _sampleReference(exr);
 	// _sampleError1(exr);
 	// _sampleError2(extp);
	// _sampleErrorNoException(extp);
 	// _sampleTransfer(exr,extp);
    // _sampleSeveralFlows(exr,extp);
    // _sampleCallables(extp);
   	// _samplePF(exr);	
	// _sampleFlows1(extp);
	// _sampleFlowsCondition(extp);
	//_sampleFlowLaunch(extp);
	//_sampleWhile(extp);
	_sampleFlowLoop(extp);
	 //_sampleFlowChart(exr);
}