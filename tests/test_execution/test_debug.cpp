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
#include <execution/flow/Condition.h>
#include <execution/flow/While.h>
#include <execution/flow/Launch.h>
#include <execution/flow/Loop.h>
#include <vector>
using std::vector;
#include <memory>
using namespace mel;
#include <array>
/*
namespace mel
{
    namespace execution
    {      
		//la idea es que reciba una lambda que recive un exfuture y devuelve un exfuture, y se comporte como una tarea normal
		//@todo recuedo que tenía problemas por pasar una lamda generica y eso...
		template <class F> class FlowWrapper
		{
			public:
				//tengo que devolver el resultado propiemente
				template <class ExecutorAgent,class TArg> auto operator(){
					return 
				}
			private:
				F mFlow;
		}

        //for internal use by condition function
    #define CONDITION_SELECT_JOB2(idx) \
            if constexpr (tsize>idx) \
            { \
                using FlowType = std::tuple_element_t<idx,TupleType>; \
                static_assert(      std::is_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value, "execution::condition bad functor signature"); \
                launch(source.agent,[source,result,fls = std::move(fls)]() mutable noexcept \
                { \
                    if constexpr (std::is_nothrow_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value) \
                        result.assign(std::get<idx>(fls)(source)); \
                    else \
                    { \
                        try \
                        { \
                            result.assign(std::get<idx>(fls)(source)); \
                        }catch(...) \
                        { \
                            result.setError(std::current_exception()); \
                        } \
                    } \
                } \
                ); \
            }else{ \
                 launch(source.agent,[result]( ) mutable noexcept {  \
                    result.setError(std::out_of_range("execution::condition. Index '" TOSTRING(idx) "' is greater than maximum case index " TOSTRING(tsize))); \
                  }); \
            }        
        template <class ExecutorAgent,class TArg,class F,class ...Flows>
         auto condition2(ExFuture<ExecutorAgent,TArg> source, F selector,Flows... flows)
        {                

            typedef typename ExFuture<ExecutorAgent,TArg>::ValueType  ValueType;
            typedef typename ::mel::execution::_private::GetReturn<ExFuture<ExecutorAgent,TArg>,Flows...>::type ResultTuple;
            using ResultType = std::tuple_element_t<0,ResultTuple>;
            ResultType result(source.agent);                       
            source.subscribeCallback(
            //need to bind de source future to not get lost and input pointing to unknown place                

                [source,selector = std::move(selector),fls = std::make_tuple(std::move(flows)...),result](  ValueType& input) mutable noexcept(std::is_nothrow_invocable<F,TArg>::value)
                {       
                    //lanzar tarea para deteccion noexcept, ¿para todo junto? creo que sí...
                    if ( input.isValid() )
                    {  
                        using TupleType = decltype(fls);
                        //Evaluate index
                        size_t idx = selector(input.value());  
                        constexpr size_t tsize = std::tuple_size<TupleType>::value;
                        switch(idx)
                        {
                            case 0:    
                                                    
                                // //codigo a pelo para temas de depuracion                                       
                                if constexpr (tsize>0)
                                { 
                                    using FlowType = std::tuple_element_t<0,TupleType>;
                                    launch(source.agent,[source,result,fls = std::move(fls)]() mutable noexcept
                                        {
                                            if constexpr (std::is_nothrow_invocable<FlowType,ExFuture<ExecutorAgent,TArg>>::value)
                                                result.assign(std::get<0>(fls)(source)); 
                                            else
                                            {
                                                try
                                                {
                                                    result.assign(std::get<0>(fls)(source)); 
                                                }catch(...)
                                                {
                                                    result.setError(std::current_exception());   
                                                }
                                            }  
                                        }
                                     );
                                }else{ 
                                    launch(source.agent,[result]( ) mutable noexcept {  
                                        result.setError(std::out_of_range("triqui"));
                                    }); 
                                }     
                                //CONDITION_SELECT_JOB(0)
                                break;
                            case 1:
                                CONDITION_SELECT_JOB2(1)
                                break;
                            
                        }                                                                          
                    }
                    else
                    {
                        //set error as task in executor
                        std::exception_ptr err = input.error();
                        launch(source.agent,[result,err]( ) mutable noexcept
                        {
                            result.setError(std::move(err));
                        });                        
                    }                                        
                }
            );
            return result;
        }  
       
        namespace _private
        {
            template <class F, class ...FTypes> struct ApplyCondition2
            {
                template <class S,class ...Fs>
                ApplyCondition2(S&& selector,Fs&&... fs):mSelector(std::forward<F>(selector)), 
                    mFuncs(std::forward<FTypes>(fs)...)
                {
                }
                F mSelector;
                std::tuple<FTypes...> mFuncs;                
                template <class TArg,class ExecutorAgent> auto operator()(ExFuture<ExecutorAgent,TArg> inputFut)
                {                  
                    return condition2(inputFut,std::forward<F>(mSelector),std::forward<FTypes>(std::get<FTypes>(mFuncs))...);
                }
            };
          
        }
        
        ///@brief version for use with operator |
        template <class F,class ...FTypes> _private::ApplyCondition2<F,FTypes...> condition2(F&& selector,FTypes&&... functions)
        {
            return _private::ApplyCondition2<F,FTypes...>(std::forward<F>(selector),std::forward<FTypes>(functions)...);
        }
	}
}
*/
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

	// template <class T> struct RemovePointerRef
	// {
	// 	using type = std::remove_pointer_t<::remove_reference_t<T>>;
	// };
		 

int _testDebug(tests::BaseTest* test)
{

	mel::text::set_level(::mel::text::level::ELevel::debug);
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
		try
		{	
			vector<int> vec(16);
			auto finalRes =  mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(

				execution::start(exInl)
				| execution::inmediate(std::ref(vec))
				| execution::next( [&vec](vector<int>&) -> vector<int>&
					{
						vec.resize(5);
						return vec;
					}
				)
				| execution::loop(

					[](vector<int>& v) noexcept -> std::array<int,2>
					{						
						return {0,(int)v.size()};
					}
					,[](int idx,vector<int>&) 
					{
						::mel::text::info("Loop {}",idx);
					}
				)
				| execution::next( [](vector<int>& v)							
				{

				})
				| execution::loop( 
					[]() noexcept
					{
						return std::array{0,50};
					},
					[](int idx) 
					{
						mel::text::info("It {}",idx);
					}
				)
			);
			/*auto finalRes =  mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(
				execution::launch(extp,[]() noexcept{ return "hola"s;})

				| execution::flow::loop(2,9, 
					[](auto input) noexcept
					{
						return input | execution::inmediate("hola"s)
						| execution::next( [](const string& s){
							::mel::text::info(" flow::loop it");
							return 6;
						}); //return void for testing purposes
					}
				)
				
			);*/
			//mel::text::info("Final res = ({},{})",std::get<0>(finalRes.value()),std::get<1>(finalRes.value()));
			mel::text::info("Final res. vecsize = {}",vec.size());
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

		/*auto res = 	mel::core::waitForFutureThread(
			execution::launch(exr,[]()
			{
				return 2;
			})			
		);*/
	}
	{
		try
		{
			auto fl = [](auto input) noexcept
					{
						return input 
						// | execution::getExecutor( [](auto ex,const string& s)
						// {
						// 	if constexpr(execution::ExecutorTraits<decltype(ex)>::has_parallelism)
						// 		return "has parallelism";
						// 	else
						// 		return "hasn't parallelism";
						// })
						| execution::next( [](const string& s){

							::mel::text::info(s+" flow::loop -> next it");
						}); //return from flow is not used, no return void
					};
			auto finalRes =  mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(

				execution::launch(extp,[]() noexcept{ return 1.3f;})
				| execution::getExecutor( [](auto ex,float)
						{
							// if constexpr(execution::ExecutorTraits<decltype(ex)>::has_parallelism)
							// 	return "has parallelism";
							// else
							// 	return "hasn't parallelism";
						})
				| execution::next( []
					{
						return "triqui"s;	
					}
				)
				| execution::flow::loop(
					[]
					{
						return std::array{2,5};
					},
					/*[](int idx,auto input) noexcept
					{
						::mel::text::info(" flow number it {}",idx);
						return input 
						| execution::next( [idx](const string& s){
							::mel::text::info(" flow::loop -> next it {}",idx);
						}); //return from flow is not used, no return void
					}*/
					[fl](int idx,auto v)
					{
						return fl(v);
					}
					,1
				)
			);
			/*auto finalRes =  mel::core::waitForFutureThread<::mel::core::WaitErrorAsException>(
				execution::launch(extp,[]() noexcept{ return "hola"s;})

				| execution::flow::loop(2,9, 
					[](auto input) noexcept
					{
						return input | execution::inmediate("hola"s)
						| execution::next( [](const string& s){
							::mel::text::info(" flow::loop it");
							return 6;
						}); //return void for testing purposes
					}
				)
				
			);*/
			//mel::text::info("Final res = ({},{})",std::get<0>(finalRes.value()),std::get<1>(finalRes.value()));
			mel::text::info("Final res");
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

		/*auto res = 	mel::core::waitForFutureThread(
			execution::launch(exr,[]()
			{
				return 2;
			})			
		);*/
	}
	{
// @todo retomar el tema del while, sin mirar lo de las capturas y ya meterlo en los tests normales luego donde se chequeará
// me queda decidir bien qué recibe el flujo y qué recibe el selector

		MyPepe obj;
		obj.val = 8;
		int cont = 0;

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

		// me sigue sin convencer lo que recibe el predicado. en la doc puse que recibe el resulktado del flujo...y no es verdad recibe lo anterior...
		// pero creo que sí deberñia ser lo del flujo
		// y una cosa...cómo podría hacer un while de varios flujos? creo que simplemente sería un flujo que lo que hace es un launch y tiene un on_all
			| execution::flow::doWhile(
				[](auto input) noexcept
				{
					return input | next([](const string& str) noexcept 
					{				
						string result = str+ " HOLA" ;
						mel::text::info("local flow {}",result);
						::mel::tasking::Process::wait(500);
						int rn = rand()%10;
						return std::make_tuple(result,rn);
					})
					| mel::execution::flow::condition(
						[](const tuple<string,int>& v) noexcept
						{
							if ( std::get<1>(v) > 5 )
								return 0;
							else
								return 1;
						},
						[](auto input) noexcept
						{
							mel::text::info("Selected condition1");
							//throw std::runtime_error("Error en cond1");// ojo que mi idea era lanzar excepcion como parte del flujo
							return input | execution::inmediate("condition1"s);
						},
						[](auto input) noexcept
						{
							mel::text::info("Selected condition2");
							//throw std::runtime_error("Error en cond1");// ojo que mi idea era lanzar excepcion como parte del flujo
							return input | execution::inmediate("condition2"s);
						}						
					);
				}
				//localFlow
			,[cont](const string& str) mutable
				{
				// tengo que decidir qué debe devolver el while. ahora estoy devolviendo lo que devuelve el flujo interno, pero no me mola. posibilidades:
				//  - que lo devuelva esta condicion: podría ser por tanto un pair lo que devuelve
				//  - que haya otra función que se ejecute al finalizar y sea la que devuelve de verrdad. me parece una bobada, porque para eso se hace un next
					mel::text::info("Condition str = {}",str);
					// me gustaría que esta condición pudiese evaluar el resultado del flujo...
					// eso es fácil, ¿pero qué devuelvo al final?
					// POSIBILIDADES:
					//  - QUE ESTE PREDICADO RECIBA EL RESULTADO DE LO SFLUJOS->IMPLICA QUE TIENEN QUE DEVOVLER ISMO RESULTADO, PERO ES NORMAL. 					 
					//  - EL RESULTADO DEL WHILE SERÍA??
					// qué tiene que recibir el flow dentro del while? es que eso de que reciba o anterior no sé...
					if ( cont++ < 10)
						return true;	
					else	
						return false;
				}
			)	
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