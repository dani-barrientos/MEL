#pragma once
#include <parallelism/Barrier.h>
#include <deque>
namespace parallelism
{
	/**
	* barrier composde of toher barriers
	* so waiting for a ComposedBarrier is as waiting for all barriers (AND) 
	* @todo sería interesante un OR. para eso necesito lanzar n tareas paralelas esperando por ellas. Y en el momento que una sale, hacer el set
	MEDITAR BIEN SOBRE ESTA HERENCIA, PORQUE AL NO PODER HACERSE LOS CONTRUCTORES DE COPIA BIEN (YA QUE SIEMPRE DEVOLVEMOS EL BARRIER TAL CUAL EN LAS FUNCIONES, SIN TIPO)
	*/
	class ComposedBarrier;
	class ComposedBarrierData : public BarrierData
	{
		friend class ::parallelism::ComposedBarrier;
	private:
		typedef std::deque<Barrier> BarrierList;
		BarrierList mBarriers;
		ComposedBarrierData(){};
		//@remarks only Barrier common part is copied, but is enough because this only relies on wait() and waitAsMthread()
		inline void addBarrier( const Barrier& barrier )
		{
			mBarriers.push_back( barrier );
		}
		FutureData_Base::EWaitResult wait( unsigned int msecs ) const;
		FutureData_Base::EWaitResult waitAsMThread( unsigned int msecs ) const;
	};
	class FOUNDATION_API ComposedBarrier : public Barrier
	{
	public:
		/*		
		*
		*/
		ComposedBarrier( );
		inline void addBarrier( const Barrier& barrier ){ ((ComposedBarrierData*)getData())->addBarrier( barrier );}
	};

}