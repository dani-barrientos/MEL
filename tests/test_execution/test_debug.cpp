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

/*
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
					// 			
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
			private:		
				F mFunct;
		};
		*/

static tests::BaseTest* sCurrentTest = nullptr;
//calling this code breask on msvc < 19.31
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

template <class ExecutorAgent,class TArg> auto flow_template ( execution::ExFuture<ExecutorAgent,TArg> source) 
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
	using namespace mel::execution;
	template <class ExecutorAgent,class TArg,class Flow,class Predicate>
         auto doWhile(ExFuture<ExecutorAgent,TArg> source, Flow&& flow, Predicate&& p)
		 {
			static_assert( std::is_invocable<Flow,ExFuture<ExecutorAgent,TArg>>::value, "execution::doWhile bad flow signature");
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            typedef std::invoke_result_t<Flow,ExFuture<ExecutorAgent,TArg>> TRet;
            TRet result(source.agent);
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                
                std::function<void( ValueType&)>([source,flow = std::forward<Flow>(flow),p = std::forward<Predicate>(p),result](  ValueType& input) mutable
                {   
					if ( input.isValid())
					{
						// launch(source.agent,[source,result,p = std::forward<Predicate>(p),flow = std::forward<Flow>(flow)]( ) mutable noexcept 
						// {  
						// 	auto ret = flow(source);		
						// 	problemas gordos:
						// 	 - subscripcion, necesito un function
						// 	ret.subscribeCallback(                
                		// 		std::function<void(auto&)>([ret,p = std::forward<Predicate>(p),result](auto& input) mutable
						// 		{
						// 			//check error
									
						// 			// //this callback comes always from current executor, so it's not needed to launch task
						// 			// bool repeat = p();
						// 			// if ( !repeat )
						// 			// {						
						// 			// 	result.assign();
						// 			// }
						// 		}
						// 		));
						// }); 
					}else
					{
						launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept {  
							result.setError(std::move(err));
						}); 
					}
				
                })
            );
            return result;
		 }
 	namespace _private
        {
            template <class Flow,class Predicate> struct ApplyWhile
            {
				template <class F,class P>
                ApplyWhile(F&& flow,P&& p):mFlow(std::forward<F>(flow),mPred(std::forward<P>(p))){}
                Flow mFlow;              
				Predicate mPred;  
				template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
				{
					return doWhile(inputFut,mFlow,mPred);
				}
            };
        }
        
        
    template <class Flow,class Predicate> _private::ApplyWhile<std::decay_t<Flow>,std::decay_t<Predicate>> doWhile(Flow&& flow,Predicate&& pred)
        {
            return _private::ApplyWhile<std::decay_t<Flow>,std::decay_t<Predicate>>(std::forward<Flow>(flow),std::forward<Predicate>(pred));
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
		auto localFlow = [](auto input)
		{
			return input | next([](const string& str) noexcept->auto
			{
				string result = str+ " HOLA" ;
				return result;
			}
			);
		};
		auto res = 
		mel::core::waitForFutureThread(
			execution::start(exr)
			| execution::inmediate("Dani"s)
			// | doWhile(localFlow,[]()
			// 	{
			// 		//@todo ¿deberia pasar algo al while, como un contador?
			// 		//@todo cómo sé que el while termina? 					
			// 		return true;	
			// 	}
			// )
			// | doWhile([](auto input){
			// 	return input | next([](const string& str) noexcept ->auto
			// 	{
			// 		return "HOLA"s;
			// 	}
			// 	);
			// })
		);
		mel::text::info("Value = {}",res.value());
	}
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
			//msvc < 19.31 has bug in lambda processing and using directly std::exception_ptr raises compilation error
			| execution::catchError( [](const std::exception_ptr& err) noexcept
			{
				//rethrow_exception(err);
				return 3.5f;
			})
			| execution::catchError( [](auto err) noexcept
			{
				return 3.5f;
			})
			;
		};

		auto futres = execution::launch(exr,[]()
			{
				//throw "error";
				return 1.8f;
			}		
		) 
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
						| mel::execution::next( [](const string& v){return v + " Carrera";}
						)
						| mel::execution::catchError( [](std::exception_ptr err) noexcept(true) ->string
							{
								//rethrow_exception(err);
								//throw std::runtime_error("Error!!. hola amigos 2");
								return "catchError!!!";
							})
						;
					}				
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

	

	Thread::sleep(5000);
	return 0;
}
}