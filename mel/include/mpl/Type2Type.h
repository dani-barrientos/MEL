#pragma once

namespace mel
{
	namespace mpl
	{
		/**
		* create new type from Int
		*/
		template <class T>
		struct Type2Type
		{
			typedef T Result;
		};
	}
}