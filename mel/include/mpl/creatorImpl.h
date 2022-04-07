
namespace mel
{
	namespace mpl
	{
		/**
		* @class Creator
		* functor class for creation through new
		*/
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		template <class T, VARIABLE_ARGS>
		class Creator
		{
		public:
			T* operator()( VARIABLE_ARGS_IMPL )
			{
				return new T( VARIABLE_ARGS_USE );
			};
		};
		//specialization for void arguments
		template <class T>
		class Creator<T,void>
		{
		public:
			T* operator()(  )
			{
				return new T(  );
			};
		};
		


	#else
		template <class T,VARIABLE_ARGS>
		class Creator<T,VARIABLE_ARGS_DECL,void> 
		{
		public:
			T* operator()( VARIABLE_ARGS_IMPL )
			{
				return new T( VARIABLE_ARGS_USE );
			};

		};


	#endif
	}
}