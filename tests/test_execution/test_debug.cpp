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
#include <memory>
using namespace mel;
namespace test_execution
{
//funcion para pruebas a lo cerdo
struct MyPepe
{
	int val;
	MyPepe()
	{
		mel::text::info("MyPepe");
	}
	~MyPepe()
	{
		val = -2;
		mel::text::info("MyPepe destructor {}",(void*)this);
	}
	MyPepe( const MyPepe& ob2)
	{
		mel::text::info("MyPepe(const MyPepe& ob2)");
		val = ob2.val;
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
		val = ob2.val;
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

static tests::BaseTest* sCurrentTest = nullptr;
//calling this code breask on msvc < 19.31
auto f = [](auto exec)
		{
			auto g = [](std::exception_ptr){};			
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

	template <class T> struct RemovePointerRef
	{
		using type = std::remove_pointer_t<::remove_reference_t<T>>;
	};

	template <class Flow,class Predicate,class ExecutorAgent,class TArg,class FlowResult> class WhileImpl : public std::enable_shared_from_this<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>>
	{
		using SourceType = ExFuture<ExecutorAgent,TArg>; 
		public:
			template <class F,class P> static std::shared_ptr<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>> create(F&& f,P&& p,SourceType source,FlowResult result)
			{				
				auto ptr = new WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>(std::forward<F>(f),std::forward<P>(p),source,result);
				return std::shared_ptr<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>>(ptr);
				//return std::make_shared<WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>>(std::forward<F>(f),std::forward<P>(p),source);
			}		
			void execute()
			{
				auto fut = mFlow(mSource);				
				auto _this = WhileImpl<Flow,Predicate,ExecutorAgent,TArg,FlowResult>::shared_from_this();
				fut.subscribeCallback( [_this,fut](auto v)
				{
					_this->_callback(fut);
				});
			}
			~WhileImpl()  //para depurar, quitarlo
			{
				mel::text::info("WhileImpl destructor");
			}		
		private:
			//typename RemovePointerRef<Flow>::type mFlow;
			//typename RemovePointerRef<Predicate>::type mPred;
			Flow 		mFlow;
			Predicate 	mPred;
			SourceType 	mSource;
			FlowResult 	mResult;

			template <class F,class P>
			WhileImpl(F&& f,P&& p,SourceType s,FlowResult r):mFlow(std::forward<F>(f)),mPred(std::forward<P>(p)),mSource(s),mResult(r)
			{				
			}
			void _callback(FlowResult res)
			{
				if ( res.getValue().isValid())
				{
					if ( mPred() )
						execute(); 
					else 
						mResult.assign(res);
				}else
					mResult.setError(std::move(res.getValue().error()));				
			}
	};

	template <class ExecutorAgent,class TArg,class Flow,class Predicate>
    //     auto doWhile(ExFuture<ExecutorAgent,TArg> source, Flow&& flow, Predicate&& p)
		 auto doWhile(ExFuture<ExecutorAgent,TArg> source, Flow flow, Predicate p)
		 {			 			 
			static_assert( std::is_invocable<Flow,ExFuture<ExecutorAgent,TArg>>::value, "execution::doWhile bad flow signature");
            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            typedef std::invoke_result_t<Flow,ExFuture<ExecutorAgent,TArg>> TRet;
            TRet result(source.agent);
            source.subscribeCallback(
                //need to bind de source future to not get lost and input pointing to unknown place                

/*lo que quiero es quitar el ref de p si lo tiene¿¿???
POSIBILDIADES:
 - USAR pREDICATE PARA QUE SEA POR COPIA Y LUEGO HACER MOVE
 -


			tengo la impresion que no es correcto este forwar, porque si p fuese referecia, eso es lo que se bindea
			yo quisiera aquí hacer un move . el problema es que el tipo original puede ser una referencia.. 
			no forwardear directamente dsde el Apply no vale, porque si yo tengo una rvalue reference, y por tanto ,que puedo mover, eso lo pierde

*/

                //[source,flow = std::forward<Flow>(flow),p = std::forward<Predicate>(p),result](ValueType& input) mutable
				[source,flow = std::move(flow),p = std::move(p),result](ValueType& input) mutable
                {   
					if ( input.isValid() )
					{
						auto _while = WhileImpl<Flow,Predicate,ExecutorAgent,TArg,TRet>::create(std::move(flow),std::move(p),source,result);
						_while->execute();											
					}else
					{
						launch(source.agent,[result,err = std::move(input.error())]( ) mutable noexcept {  
							result.setError(std::move(err));
						}); 
					}
				
                }
            );
            return result;
		 }
 	namespace _private
        {
            template <class Flow,class Predicate> struct ApplyWhile
            {
				template <class F,class P>
                ApplyWhile(F&& flow,P&& p):mFlow(std::forward<F>(flow)),mPred(std::forward<P>(p))
				{					
				}				
				// ~ApplyWhile() //PARA DEPURACION
				// {
				// 	mel::text::info("Destructor ApplyWhile");
				// }
                Flow mFlow;              
				Predicate mPred;  
				template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
				{
					return doWhile(inputFut,std::forward<Flow>(mFlow),std::forward<Predicate>(mPred));
				}
            };
        }
        
    //template <class Flow,class Predicate> _private::ApplyWhile<std::decay_t<Flow>,std::decay_t<Predicate>> doWhile(Flow&& flow,Predicate&& pred)
	template <class Flow,class Predicate> _private::ApplyWhile<Flow,Predicate> doWhile(Flow&& flow,Predicate&& pred)
        {
            return _private::ApplyWhile<Flow,Predicate>(std::forward<Flow>(flow),std::forward<Predicate>(pred));
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
// @todo retomar el tema del while, sin mirar lo de las capturas y ya meterlo en los tests normales luego donde se chequeará
// me queda decidir bien qué recibe el flujo y qué recibe el selector

		MyPepe obj;
		obj.val = 8;
		int cont = 0;
//preparar buenos tests para ver copias es capturas

		auto localFlow = [obj](auto input)
		{
			mel::text::info("Obj.val = {}. Obj = {}",obj.val,(void*)(&obj));
			return input | next([](const string& str) noexcept->auto
			{				
				string result = str+ " HOLA" ;
				mel::text::info("local flow {}",result);
				return result;
			}
			);
		};
		auto cond1 = [](auto input) noexcept
		{
			//throw std::runtime_error("Error en cond1");// ojo que mi idea era lanzar excepcion como parte del flujo
			return input | execution::inmediate("condition1"s);
		};	
		auto cond2 = [](auto input) noexcept
		{
			return input | execution::inmediate("condition2"s) 
			
			//msvc < 19.31 has bug in lambda processing and using directly std::exception_ptr raises compilation error
			| execution::catchError( [](const std::exception_ptr& err) noexcept
			{
				//rethrow_exception(err);
				return "err1";
			})
			//si no pongo esto el catchError provoca problemas al ejecutar el flujo.¿¿??
			| execution::next( [](const string& s)
			{
				return s;
			})
			;
		};
		
		// auto lNext = [](const string& s) noexcept
		// 		{
		// 			mel::text::info("Next before wait");
		// 			mel::tasking::Process::wait(2000);
		// 			mel::text::info("Next after wait");
		// 			return s;
		// 		};
		auto lPar = [](auto s) noexcept
		{
			mel::text::info("par {}",s);
		};
		auto res = 	mel::core::waitForFutureThread(
			execution::start(exr)
			| execution::inmediate("Dani"s)
			| execution::next(
				 [](const string& s) noexcept
				{
					mel::text::info("Next before wait");
					mel::tasking::Process::wait(2000);
					mel::text::info("Next after wait");
					return s;
				}
				//lNext
			)
			| execution::parallel( 
				lPar,
				[](const string& s)
				{
					mel::text::info("Par2");
				}
			)
			| mel::execution::condition(
						[](const string& v) noexcept
						{
							text::info("condition {}",v);
							return 1;
						},
						cond1,cond2
					)			
			/*| doWhile(
				// [obj](auto input)
				// {
				// 	return input | next([](const string& str) noexcept->auto
				// 	{				
				// 		string result = str+ " HOLA" ;
				// 		mel::text::info("local flow {}",result);
				// 		return result;
				// 	}
				// 	);
				// }
				localFlow

			,[cont,obj]() mutable
				{
					//mel::text::info("Predicado: {}, {}",cont++,obj.val++);
					if ( cont < 30)
						return true;	
					else	
						return false;
					// mel::text::info("Predicado: {}",++obj.val);
					// if ( obj.val < 30)
					// 	return true;	
					// else	
					// 	return false;
				}
			)	*/		
		);
		mel::text::info("Value = {}",res.value());
	}
	// {		
	// 	auto flow_lambda_local = [](auto source) ->auto //es un ExFuture<ExecutorAgent,string>    can only be templated in C++ 20
	// 	{
	// 		return execution::next(source, [](const string& val) noexcept
	// 		{
				
	// 			::mel::text::info("Flow launch");
	// 			return val + " flow_lambda_local";
	// 		})
	// 		| execution::loop(0,10,[](int idx, const string& str)
	// 		{
	// 			::mel::text::info("Flow launch loop {}",idx);
	// 		})
	// 		| execution::catchError( [](std::exception_ptr err)
	// 			{
	// 				return "flow_lambda_local catch error"s;
	// 			}
	// 		)
	// 		| execution::next( []( string str)
	// 		{
	// 			return "Next"s;
	// 		}
	// 		)
	// 		;
	// 	};
	
	// 	auto cond1 = [](auto input) noexcept
	// 	{
	// 		//throw std::runtime_error("Error en cond1");// ojo que mi idea era lanzar excepcion como parte del flujo
	// 		return input | execution::inmediate(1.5f) 
	// 		/*| execution::next([](float) ->float
	// 		{
	// 			throw std::runtime_error("Error en next de cond1");			
	// 		}
	// 		)*/
	// 		;
	// 	};
	// 	auto cond2 = [](auto input) noexcept
	// 	{
	// 		return input | execution::inmediate(2.5f) 
	// 		//msvc < 19.31 has bug in lambda processing and using directly std::exception_ptr raises compilation error
	// 		| execution::catchError( [](const std::exception_ptr& err) noexcept
	// 		{
	// 			//rethrow_exception(err);
	// 			return 3.5f;
	// 		})
	// 		| execution::catchError( [](auto err) noexcept
	// 		{
	// 			return 3.5f;
	// 		})
	// 		;
	// 	};

	// 	auto futres = execution::launch(exr,[]()
	// 		{
	// 			//throw "error";
	// 			return 1.8f;
	// 		}		
	// 	) 
	// //	| createFlow(flow1_2) _> el problema de esto es que los flows son templates (o auto con lambda, pero ya vi que da problemas)
	// 	;
		
	// 	try
	// 	{
	// 		auto res = mel::core::waitForFutureThread(
	// 			flow_template(futres) //como es template no puedo hacerlo de otra forma???
	// 			| execution::next( [](const string& str)
	// 				{
	// 					//throw std::runtime_error("Error!!. hola amigos");
	// 					return str + " dani";		
	// 				}
	// 			)
	// 			| flow_lambda
	// 			| [](auto input)
	// 				{						
	// 					return input
	// 					| mel::execution::next( [](const string& v){return v + " Carrera";}
	// 					)
	// 					| mel::execution::catchError( [](std::exception_ptr err) noexcept(true) ->string
	// 						{
	// 							//rethrow_exception(err);
	// 							//throw std::runtime_error("Error!!. hola amigos 2");
	// 							return "catchError!!!";
	// 						})
	// 					;
	// 				}				
	// 			| mel::execution::condition(
	// 					[](const string& v)
	// 					{
	// 						text::info("condition {}",v);
	// 						return 0;
	// 					},
	// 					cond1,cond2
	// 				)
	// 		);				
	// 		mel::text::info("Result = {}",res.value());
	// 	}
	// 	catch(std::exception& e)
	// 	{
	// 		mel::text::info("ERROR. Cause = {}",e.what());
	// 	}
	// 	catch(...)
	// 	{
	// 		mel::text::info("ERROR");
	// 	}
	// 	mel::text::info("FIN");
	// 	return 0;
	// }

	

	Thread::sleep(5000);
	return 0;
}
}