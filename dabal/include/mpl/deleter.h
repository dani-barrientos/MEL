#pragma once

namespace mpl
{
	/**
	* @brief generic functor to delete pointers
	* @todo hacer version para arrays
	*/
	template <class T> void del_ptr( T* object )
	{
		delete object;
	}
}