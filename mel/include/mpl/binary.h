#pragma once

namespace mel
{
	namespace mpl
	{
		template <unsigned long number>
		struct binary
		{
			static unsigned const value
				= binary<number/10>::value << 1   
				| number%10;                 
		};

		template <>                          
		struct binary<0>                     
		{
			static unsigned const value = 0;
		};

	}
}