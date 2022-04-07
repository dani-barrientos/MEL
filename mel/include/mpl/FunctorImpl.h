namespace mel
{
	namespace mpl
	{
		/**
		* @class Chainer
		* Chain two functors
		*/
	#if VARIABLE_NUM_ARGS == VARIABLE_MAX_ARGS
		///@cond HIDDEN_SYMBOLS
		template < class Fun1, class Fun2 >
		class Chainer_Base
		{
		public:
			Chainer_Base(  const Fun1& f1, const Fun2& f2 ):mF1( f1 ), mF2( f2 ){};
			template <class F>
			bool operator == ( const F& ob2 ) const
			{
				return equality( Int2Type< Conversion<F, Chainer_Base<Fun1,Fun2> >::exists >(),ob2 );
			}
		private:
			template <class F>
			bool equality( Int2Type<false>,const F&) const
			{
				return false;
			}
			bool equality( Int2Type<true >,const Chainer_Base<Fun1,Fun2>& ob2) const
			{
				//return mF1 == ob2.mF1 && mF2 == ob2.mF2;
				return mel::mpl::equal<true>( mF1, ob2.mF1 ) &&::mel::mpl::equal<true>( mF2,ob2.mF2 );
			}


		protected:
			Fun1 mF1;
			Fun2 mF2;
		};
		///@endcond
		template < class Fun1, class Fun2,VARIABLE_ARGS > class Chainer : public Chainer_Base<Fun1,Fun2>
		{
		public:
			Chainer( const Fun1& f1, const Fun2& f2 ): Chainer_Base<Fun1,Fun2>(f1,f2)
			{
			}
			//TODO �que devuelvo?
			void operator()( VARIABLE_ARGS_IMPL )
			{
				Chainer_Base<Fun1,Fun2>::mF1( VARIABLE_ARGS_USE );
				Chainer_Base<Fun1,Fun2>::mF2( VARIABLE_ARGS_USE );
			}
		};
		///@cond HIDDEN_SYMBOLS
		//specialization for void arguments
		template <class Fun1, class Fun2>
		class Chainer<Fun1,Fun2> : public Chainer_Base<Fun1,Fun2>
		{
		public:
			Chainer(  const Fun1& f1,  const Fun2& f2 ):  Chainer_Base<Fun1,Fun2>(f1,f2)
			{
			}
			void operator()(  )
			{
				Chainer_Base<Fun1,Fun2>::mF1(  );
				Chainer_Base<Fun1,Fun2>::mF2(  );
			}
		};
		///@endcond
		template <VARIABLE_ARGS_NODEFAULT,class F1, class F2>
			Chainer<F1,F2,VARIABLE_ARGS_DECL> chain( F1 f1, F2 f2 )
			{
				return Chainer<F1,F2,VARIABLE_ARGS_DECL>(f1,f2);
			}
	#else

		template < class Fun1, class Fun2,VARIABLE_ARGS >
		class Chainer<Fun1,Fun2,VARIABLE_ARGS_DECL,void> : public Chainer_Base<Fun1,Fun2>
		{
		public:
			Chainer( const Fun1& f1, const Fun2& f2 ): Chainer_Base<Fun1,Fun2>(f1,f2)
			{
			}
			//TODO �que devuelvo?
			void operator()( VARIABLE_ARGS_IMPL )
			{
				Chainer_Base<Fun1,Fun2>::mF1( VARIABLE_ARGS_USE );
				Chainer_Base<Fun1,Fun2>::mF2( VARIABLE_ARGS_USE );
			}
		};
		template <VARIABLE_ARGS,class F1, class F2>
		Chainer<F1,F2,VARIABLE_ARGS_DECL> chain( F1 f1, F2 f2 )
		{
			return Chainer<F1,F2,VARIABLE_ARGS_DECL>(f1,f2);
		}


	#endif
	}
}