#pragma once

namespace mel
{
	namespace mpl
	{
		/**
		* create new type from Int
		*/
		template <int valor>
		struct Int2Type
		{
			enum { value = valor };
		//	bool operator==( const Int2Type& ob2 ) const { return true;}
		};

	}
}