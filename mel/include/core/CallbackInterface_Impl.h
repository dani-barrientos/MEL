

#include <core/Type.h>
using ::mel::core::Type;


///@cond HIDDEN_SYMBOLS
namespace mel
{
	namespace core
	{

		/**
		* Base class for callback functions
		* @version 1.0
		*/
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		template<class TRet, VARIABLE_ARGS>
		class   CallbackInterface_Base
		{
			MEL_CORE_OBJECT_TYPEINFO_ROOT
		public:
			virtual ~CallbackInterface_Base(){};
			/**
			*
			* @param ev2    ev2
			*/
			virtual bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const =0;
			virtual CallbackInterface_Base<TRet,VARIABLE_ARGS_DECL> * clone() const =0;


		};
		template<class TRet, VARIABLE_ARGS >
		class   CallbackInterface : public CallbackInterface_Base<TRet,VARIABLE_ARGS_DECL>
		{

		public:
			virtual TRet operator()( VARIABLE_ARGS_IMPL ) =0;
		};

		template <class TRet,VARIABLE_ARGS_NODEFAULT>
		MEL_CORE_OBJECT_TYPEINFO_IMPL_ROOT(CallbackInterface_Base<TRet coma VARIABLE_ARGS_DECL>);

		///@cond HIDDEN_SYMBOLS
		//specialization for void arguments
		template<class TRet >
		class   CallbackInterface<TRet> : public CallbackInterface_Base<TRet,void>
		{

		public:
			virtual TRet operator()( ) =0;
		};
	#else
		template<class TRet, VARIABLE_ARGS>
		class   CallbackInterface_Base<TRet,VARIABLE_ARGS_DECL,void>
		{
			MEL_CORE_OBJECT_TYPEINFO_ROOT;
		public:
			virtual ~CallbackInterface_Base(){};
			/**
			*
			* @param ev2    ev2
			*/
			virtual bool operator==(const CallbackInterface_Base<TRet, VARIABLE_ARGS_DECL>& ev2) const =0;
			virtual CallbackInterface_Base<TRet,VARIABLE_ARGS_DECL> * clone() const =0;


		};
		///@endcond
		template<class TRet, VARIABLE_ARGS>
		class   CallbackInterface<TRet,VARIABLE_ARGS_DECL,void> : public CallbackInterface_Base<TRet,VARIABLE_ARGS_DECL>
		{

		public:
			virtual TRet operator()( VARIABLE_ARGS_DECL ) =0;
		};
		template <class TRet,VARIABLE_ARGS>
		MEL_CORE_OBJECT_TYPEINFO_IMPL_ROOT(CallbackInterface_Base<TRet coma VARIABLE_ARGS_DECL>);

	#endif

	}
	///@endcond
}
