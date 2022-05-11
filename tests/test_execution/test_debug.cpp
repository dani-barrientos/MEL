#include "test_debug.h"
#include <tasking/ThreadRunnable.h>
using mel::tasking::ThreadRunnable;
using namespace std;
#include <TestManager.h>
using tests::TestManager;
#include <text/logger.h>
#include <CommandLine.h>
#include <mpl/LinkArgs.h>
#include <mpl/Ref.h>
#include <string>
#include <tasking/Process.h>
using mel::tasking::Process;
#include <tasking/utilities.h>
#include <execution/InlineExecutor.h>
#include <execution/NaiveInlineExecutor.h>
#include <execution/RunnableExecutor.h>
#include <execution/ThreadPoolExecutor.h>
#include <execution/Flow.h>
#include <vector>
using std::vector;

using namespace mel;
namespace test_execution
{
//funcion para pruebas a lo cerdo
struct MyPepe
{
	int val = 10;
	MyPepe()
	{
		mel::text::info("MyPepe");
	}
	MyPepe( const MyPepe& ob2)
	{
		mel::text::info("MyPepe(const MyPepe& ob2)");
	}
	MyPepe( MyPepe&& ob2)
	{
		mel::text::info("MyPepe(MyPepe&& ob2)");
		val = ob2.val;
		ob2.val = -1;
	}
	MyPepe& operator=(const MyPepe& ob2)
	{
		mel::text::info("operator= (const MyPepe& ob2)");
		return *this;
	}
	MyPepe& operator=(MyPepe&& ob2)
	{
		mel::text::info("operator= (MyPepe&& ob2)");
		val = ob2.val;
		ob2.val = -1;
		return *this;
	}
};
namespace _private
{
	template <class F> struct ApplyFlow
	{
		ApplyFlow(F&& f):mFlow(std::forward<F>(f)){}
		ApplyFlow(const F& f):mFlow(f){}
		F mFlow;
		template <class TArg,class ExecutorAgent> auto operator()(execution::ExFuture<ExecutorAgent,TArg> input)
		{
			mFlow(input);
		}		
	};
}
		using execution::ExFuture;
		using execution::Executor;
		template <class F> class Flow
		{
			public:
				Flow(F&& f):mFunct(std::move(f)){
				}
				Flow(const F& f):mFunct(f){
				}
				//cuado lo tenga claro, ya poner tipos bien
				//template <class TRet,class SourceType,class TArg> TRet operator()(SourceType source, TArg&& v,bool directExecution = false )				
				template <class TArg,class ExecutorAgent> std::invoke_result_t<F,ExFuture<ExecutorAgent,TArg>> operator()(ExFuture<ExecutorAgent,TArg> source)
				{
					return mFunct(source);
					//static_assert( std::is_invocable<F,TArg>::value, "execution::Flow bad functor signature");
					//typedef std::invoke_result_t<F,Executor<ExecutorAgent>,TArg> TRet;  //it's an ExFuture
					//typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;            
					// TRet result(source.agent);
					// //@todo no tengo claro ahora por qué necesito el function!!!
					// source.subscribeCallback(std::function<void( ValueType&)>(
					// 	[source,result,this]( ValueType& input) mutable 
					// 	{
					// 		if ( input.isValid() )
					// 		{
					// 			//@todo no tengo nada claro que pueda hacerlo sin tarea
					// 			result.assign(mFunct(source.agent,source.getValue().value()));
					// 			/*launch(source.agent,[result,source,this]( ) mutable noexcept
					// 			{
					// 				auto val = mFunct(source.agent,source.getValue().value());
					// 				result.assign(std::move(val));
					// 			});
					// 			*/
					// 		}else
					// 		{
                    //             //set error as task in executor
					// 			launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept
					// 			{
					// 				result.setError(std::move(err));
					// 			});
								
					// 		}				
					// 	})
					// );
					// return result;
				}
				// template <class SourceType> auto operator()(SourceType source)
				// {
				// 	return ::_private::ApplyFlow()
				// }
				//template <class TRet,class SourceType,class TArg> TRet operator()(SourceType source, TArg&& v,bool directExecution = false )
			private:		
				F mFunct;
		};
		// template <class F> Flow flow(F&& functor)
		// {
		// 	return Flow(std::forward(functor));
		// }

static tests::BaseTest* sCurrentTest = nullptr;
auto f = [](auto exec)
		{
		/*	auto res = execution::launch(exec,[]() 
				{
				//	throw std::runtime_error("Err en launch");		
					return 5.2f;
				})    
				| execution::next( [](float v)
				{
					return std::to_string(v);
				})
				// | execution::catchError2( [](std::exception_ptr) noexcept
				// {
				// 	return "cachiporrez"s;
				// })
				| mel::execution::next( [](const string& str) noexcept
					{
						//throw std::runtime_error("Err en next");		
						return str + " fin";
					}
				) 
				| mel::execution::next( [](const string& str)
					{
						return 7.8f;
					}
				);
				*/
			auto g = [](std::exception_ptr){};
			//mel::execution::catchError2(res,[](std::exception_ptr){});
			//return res;
		};

template <class ExecutorAgent,class TArg> auto flow_template (ExFuture<ExecutorAgent,TArg> source) 
{
	return execution::next(source, [](float val) noexcept
	{
		
		::mel::text::info("Flow launch");
		return std::to_string(val) + " dani";
	})
	| execution::catchError( [](std::exception_ptr err)
		{
			return "pepito por error"s;
		}
	)
	| execution::next( []( string str)
	{
		return "Next"s;
	}
	)
	;
};

auto flow_lambda  = [](auto source) ->auto //es un ExFuture<ExecutorAgent,const string&>
{
	return execution::next(source, [](const string& val) noexcept
	{
		
		::mel::text::info("flow_lambda launch");
		return val + " flow_lambda";
	})
	// | execution::catchError( [](std::exception_ptr err)
	// 	{
	// 		return "flow_lambda catch error"s;
	// 	}
	// )
	| execution::next( []( const string& str)
	{
		return str+ " Next";
	}
	)
	;
};
//creo que esto no hace falta
template <class F> _private::ApplyFlow<F> createFlow( F&& f)
{
	return _private::ApplyFlow<F>(std::forward<F>(f));
}


int _testDebug(tests::BaseTest* test)
{

	int result = 0;	
	auto th1 = ThreadRunnable::create(true);	
	//auto th2 = ThreadRunnable::create(true);	
	execution::Executor<Runnable> exr(th1);		
	exr.setOpts({true,false});
	//now executor for threadpool
	parallelism::ThreadPool::ThreadPoolOpts opts;
	auto myPool = make_shared<parallelism::ThreadPool>(opts);
	parallelism::ThreadPool::ExecutionOpts exopts;
	execution::Executor<parallelism::ThreadPool> extp(myPool);
 	extp.setOpts({true,true});   
	sCurrentTest = test;
	execution::InlineExecutor exInl;
	execution::NaiveInlineExecutor exNaive;	
	{	
		auto flow_lambda_local = [](auto source) ->auto //es un ExFuture<ExecutorAgent,string>    can only be templated in C++ 20
		{
			return execution::next(source, [](const string& val) noexcept
			{
				
				::mel::text::info("Flow launch");
				return val + " flow_lambda_local";
			})
			| execution::loop(0,10,[](int idx, const string& str)
			{
				::mel::text::info("Flow launch loop {}",idx);
			})
			| execution::catchError( [](std::exception_ptr err)
				{
					return "flow_lambda_local catch error"s;
				}
			)
			| execution::next( []( string str)
			{
				return "Next"s;
			}
			)
			;
		};
		auto flow2 = Flow([](auto input)
		{
			//mel::tasking::Process::wait(1000);
			// ::mel::text::info("Option 2. {} ",v.val);
			// throw std::runtime_error("Err opt 2");
			// return 6.8f;
			return input
			| execution::next( [](auto v){return "Barrientos"s;}
			)
			| execution::catchError( [](std::exception_ptr err)
				{
					return "cachiporrez"s;
				})
			;
		});
		auto cond1 = [](auto input) noexcept
		{
			//throw std::runtime_error("Error en cond1");// ojo que mi idea era lanzar excepcion como parte del flujo
			return input | execution::inmediate(1.5f) 
			/*| execution::next([](float) ->float
			{
				throw std::runtime_error("Error en next de cond1");			
			}
			)*/
			;
		};
		auto cond2 = [](auto input) noexcept
		{
			return input | execution::inmediate(2.5f) 
			| execution::catchError( [](std::exception_ptr err) noexcept
			{
				return 3.5f;
			});
		};
		auto futres = execution::launch(exr,[]()
			{
				//throw "error";
				return 1.8f;
			}		
		);
	//	| createFlow(flow1_2) _> el problema de esto es que los flows son templates (o auto con lambda, pero ya vi que da problemas)
		;
		
		try
		{
			auto res = mel::core::waitForFutureThread(
				flow_template(futres) //como es template no puedo hacerlo de otra forma???
				| execution::next( [](const string& str)
					{
						//throw std::runtime_error("Error!!. hola amigos");
						return str + " dani";		
					}
				)
				| flow_lambda
				| [](auto input)
					{						
						return input
						| execution::next( [](const string& v){return v + " Carrera";}
						)
						| execution::catchError( [](std::exception_ptr err) noexcept(true) ->string
							{
								//rethrow_exception(err);
								//throw std::runtime_error("Error!!. hola amigos 2");
								return "catchError!!!";
							})
						;
					}
				//| flow_lambda_local
				/*| Flow(flow_lambda_local)
				| flow2
				| Flow([](auto input)
					{						
						return input
						| execution::next( [](const string& v){return v + " Carrera";}
						)
						| execution::catchError( [](std::exception_ptr err)
							{
								return "cachiporrez"s;
							})
						;
					}
				)*/
				| mel::execution::condition(
						[](const string& v)
						{
							text::info("condition {}",v);
							return 0;
						},
						cond1,cond2
					)
			);				
			mel::text::info("Result = {}",res.value());
		}
		catch(std::exception& e)
		{
			mel::text::info("ERROR. Cause = {}",e.what());
		}
		catch(...)
		{
			mel::text::info("ERROR");
		}
		mel::text::info("FIN");
		return 0;
	}
	{
		auto res = mel::core::waitForFutureThread(
				execution::start(exr) 
				| mel::execution::next([]
				{				
					return vector<MyPepe>();
				})
				| mel::execution::next([](vector<MyPepe> v) noexcept
					{				
						//fill de vector with a simple case for the result to be predecible
						//I don't want out to log the initial constructions, oncly constructons and after this function
						v.resize(100);	
						for(auto& elem:v)
						{
							elem.val = 2;
						}
						return std::move(v); 
					})
					| mel::execution::next([](vector<MyPepe> v) noexcept
					{
					//@todo ahora estos ejemplos no vale, porque no es referencia
						size_t s = v.size();
						for(auto& elem:v)
							++elem.val;						
						return std::move(v);	
					})| mel::execution::parallel(
						[](vector<MyPepe> v) 
						{								
							
						},
						[](const vector<MyPepe>& v) noexcept
						{
						
						})
		);
	}
	
	
    {
		
        
        float a = 1.f;
		MyPepe mp;	
		//yo l oque quiero es que esto ahora sea una cadena 	
		/*auto flow1 = [](auto ex,const MyPepe& v)
		{
			return execution::start(ex)
			|execution::inmediate(1.4f);
		};*/
		/*auto flow1 = [](const MyPepe& v)
		{
			::mel::text::info("Option 1. {} ",v.val);
			return 1.4f;
		};*/
		auto flow2 = [](auto ex, const MyPepe& v)
		{
			//mel::tasking::Process::wait(1000);
			// ::mel::text::info("Option 2. {} ",v.val);
			// throw std::runtime_error("Err opt 2");
			// return 6.8f;
			return execution::start(ex)
			|execution::inmediate(6.8);
		};

        // execution::launch(ex,[]()->float
		// {
		// 	//throw std::runtime_error("Err en launch");
		// 	return 2.f;
		// });
        // execution::launch(ex,[]() noexcept
		// {
		// 	//throw std::runtime_error("Err en launch");
        //     ::mel::text::info("UNO");
		// });
        // execution::launch(ex,[](float& v)
		// {
		// 	//throw std::runtime_error("Err en launch");
		// 	v+= 2.f;
		// },std::ref(a));
        
        auto res = execution::launch(exr,[](float& v) noexcept -> float&
		{
			//throw std::runtime_error("Err en launch");
			v+= 2.f;
			return v;
		},std::ref(a))        
        | execution::next([](float& res) ->float&
		{

//			throw std::runtime_error("err en next");
			mel::text::info("Primer next {}",res);
            res += 2.f;
			return res;
        })
         | execution::next([](float& res) noexcept
		{

//			throw std::runtime_error("err en next");
			mel::text::info("Primer next {}",res);
            res += 2.f;
			return res+1;
        })
	
		//version quitando el input parameter
        /*| execution::next( [](float&){})
        | execution::next( [](){})
        | execution::loop(0,10,[](int idx)  noexcept
        {            
            mel::text::info("Loop. It {}",idx);            
            //throw std::runtime_error("err en loop");
         })
        | execution::parallel(
            []() noexcept
            {
                mel::text::info("T1");
            },
            []() 
            {
        //        throw std::runtime_error("err en T2");
                mel::text::info("T2");
            }
        )
        | execution::parallel_convert<std::tuple<int,string>>(
			[]() noexcept
			{
				mel::text::info("T1");
                return 11;
			},
            []()
			{
                //throw std::runtime_error("T2");
				mel::text::info("T2");
                return "dani2"s;
			})    
        */
        //version with input parameter
        | execution::loop(0,10,[](int idx,float v) noexcept
        {            
            //throw std::runtime_error("err en loop");
            mel::text::info("Loop. It {}",idx);            
            //v++;
        })
        | execution::parallel(
            [](float v) noexcept
            {
                mel::text::info("T1 {}",v);
            },
            [](const float& v) 
            {
               // throw std::runtime_error("err en T2");
                mel::text::info("T2 {}",v);
            }
        )
        | execution::parallel_convert(
			[](const float& v) noexcept
			{
				mel::text::info("T1 {}",v);
                //v++;
                return 10;
			},
            [](float v) 
			{
             //   throw std::runtime_error("T2");
				mel::text::info("T2 {}",v);
                v++;
                return "dani"s;
			}
		)                           
        ;
/*
	//la idea del pair es poder hacer que los jobs reciban otra cosa->¿por qué no uso mismo tipo que la entrada?->podría hacerlo de forma que cada job empezase con 
	//YA VERÉ, IGUAL LO QUITO

        auto res2 = mel::execution::condition<float>(res,[](std::tuple<int,string>& v)
		{
			return std::make_pair(0,MyPepe());
		},
		//[&mp](MyPepe v) noexcept    esta version no detecta el noexcept
		[&mp](const MyPepe& v) noexcept
		{
			//el problema es que  con argumento por copia no piulla el noexcept no pilla el noexcept..
		//	::mel::text::info("Option 1. {} {}",std::get<0>(v),std::get<1>(v));
			::mel::text::info("Option 1. {} ",v.val);
			return 1.4f;
		},
		[](const MyPepe& v)
		{
			//mel::tasking::Process::wait(1000);
			::mel::text::info("Option 2. {} ",v.val);
			throw std::runtime_error("Err opt 2");
			return 6.8f;
		}
		);*/
        try
		{
			auto val = mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(res);            
           // mel::text::info("Value = {} {}",std::get<0>(val.value()),std::get<1>(val.value()));
		//	mel::text::info("Value = {}",val.value());
			auto& value = val.value();
			mel::text::info("Value = {}");
			mel::text::info("Original Value = {}",a);
		}
		catch(std::exception& e)
		{
			mel::text::error("inlineExecutor {}",e.what());
		}
		catch(...)
		{
			mel::text::error("inlineExecutor. Unknown error");
		}
		mel::text::info("HECHO");
    }
	{
		execution::NaiveInlineExecutor ex;        

		float a = 1.f;
		auto res = execution::launch(exr,[](float& v) noexcept -> float& 
		{
			//throw std::runtime_error("HOLA");
			v = 2.f;
			return v;
		},std::ref(a))
		// auto res = execution::launch(ex,[]() noexcept
		// {
		// 	//throw std::runtime_error("HOLA");
		// 	mel::text::info("launch");
		// 	return 1;
		// })
		| execution::next([](float& res) ->float&
		{
		//	throw std::runtime_error("HOLA");
			mel::text::info("next {}",res);
            res += 2.f;
			return res;
		})
        // | execution::next( [](float&){} )  //para quitar el retorno anterior
        // | execution::parallel(
		// 	[]() noexcept
		// 	{
		// 		mel::text::info("T1");
        //         //v++;
		// 	},
        //     []() 
		// 	{
        //         throw std::runtime_error("T2");
		// 		mel::text::info("T2");
		// 	})
		| execution::parallel(
			[](float& v) noexcept
			{
				mel::text::info("T1 {}",v);
                v++;
			},
            [](float& v) 
			{
               // throw std::runtime_error("T2");
				mel::text::info("T2 {}",v);
                v++;
			})            
        | execution::loop(0,10,[](int idx,float& v) noexcept
        {            
            mel::text::info("Loop. It {}",idx);            
            v++;
        })
        //  | execution::next( [](float&){} )  //para quitar el retorno anterior         
        //  | execution::parallel_convert<std::tuple<int,string>>(
		// 	[]() noexcept
		// 	{
		// 		mel::text::info("T1");
        //         return 10;
		// 	},
        //     []() 
		// 	{
        //         throw std::runtime_error("T2");
		// 		mel::text::info("T2");
        //         return "dani"s;
		// 	})  
        | execution::parallel_convert(
			[](float& v) noexcept
			{
				mel::text::info("T1 {}",v);
                v++;
                return 10;
			},
            [](float& v) 
			{
             //   throw std::runtime_error("T2");
				mel::text::info("T2 {}",v);
                v++;
                return "dani"s;
			})    
		;
		try
		{
			auto val = mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(res);
            mel::text::info("Value = {} {}",std::get<0>(val.value()),std::get<1>(val.value()));
			//mel::text::info("Value = {}",val.value());
			//mel::text::info("Value = {}");
			mel::text::info("Original Value = {}",a);
		}
		catch(std::exception& e)
		{
			mel::text::error("inlineExecutor {}",e.what());
		}
		catch(...)
		{
			mel::text::error("inlineExecutor. Unknown error");
		}
		mel::text::info("HECHO");
	}
	
	
	
	
	// {
	
	// 	vector<float> vec = {1.0f,20.0f,36.5f};
		
	// 	auto kk0 = mel::execution::next(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v) -> vector<float>&
	// 	{
	// 		auto& val = v.value();
	// 		val[1] = 1000.7;
	// 		//return std::ref(val);
	// 		return val;
	// 	}
	// 	);
	// 	//auto kk1 = mel::execution::parallel(execution::inmediate(execution::start(exr),std::ref(vec)),[](auto& v)
	// 	auto kk1 = mel::execution::parallel(kk0,[](auto& v)
	// 	{
	// 		auto idx = v.index();
	// 		//::tasking::Process::switchProcess(true);
	// 		if ( v.isValid() )	
	// 			text::info("Runnable Bulk 1 Value = {}",v.value()[0]);
	// 		else
	// 			text::info("Runnable Bulk 1 Error = {}",v.error().errorMsg);
	// 		++v.value()[0];
	// 	},[](auto& v)
	// 	{
	// 		if ( v.isValid() )	
	// 			text::info("Runnable Bulk 2 Value = {}",v.value()[1]);
	// 		else
	// 			text::info("Runnable Bulk 2 Error = {}",v.error().errorMsg);
	// 		++v.value()[1];
	// 	},
	// 	[](auto& v)
	// 	{
	// 		if ( v.isValid() )	
	// 			text::info("Runnable Bulk 3 Value = {}",v.value()[2]);
	// 		else
	// 			text::info("Runnable Bulk 3 Error = {}",v.error().errorMsg);
	// 		++v.value()[2];
	// 	}
	// 	);	
	// 	core::waitForFutureThread(kk1);
	// 	// auto kk1_1 = mel::execution::next(kk1,[](auto& v)
	// 	// {
	// 	// 	text::info("After Bulk");			
	// 	// 	if ( v.isValid() )
	// 	// 	{
	// 	// 		text::info("Value = {}",v.value()[1]);
	// 	// 		v.value()[1] = 9.7f;
	// 	// 		//text::info("After parallel value ({},{},{})",std::get<0>(val),std::get<1>(val),std::get<2>(val));
	// 	// 	}else
	// 	// 		text::info("After parallel err {}",v.error().errorMsg);
	// 	// });
	// 	// mel::core::waitForFutureThread(kk1_1);
	// 	text::info("After wait");
	// 	// auto kk2 = mel::execution::loop(kk1,idx0,loopSize,
	// 	// 	[](int idx,const auto& v)
	// 	// 	{
	// 	// 		::tasking::Process::wait(1000);
	// 	// 		if ( v.isValid() )
	// 	// 		{
	// 	// 			const auto& val = v.value();
	// 	// 			text::info("It {}. Value = {}",idx,std::get<0>(val));				
	// 	// 		}
	// 	// 		else
	// 	// 			text::info("It {}. Error = {}",idx,v.error().errorMsg);											
	// 	// 	}
	// 	// );
	// 	// auto kk3 = mel::execution::next(kk2,[](const auto& v)->int
	// 	// {								
	// 	// 	text::info("Launch waiting");
	// 	// 	if ( ::mel::tasking::Process::wait(5000) != mel::tasking::Process::ESwitchResult::ESWITCH_KILL )
	// 	// 	{
	// 	// 		//throw std::runtime_error("Error1");
	// 	// 		text::info("Launch done");
	// 	// 	}else
	// 	// 		text::info("LauncstartIdx
	// 	// ::mel::core::waitForFutureThread(kk3);
	// 	text::info("Done!!");
	// }		
	

	Thread::sleep(5000);
	return 0;
}
}