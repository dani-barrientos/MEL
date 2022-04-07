#pragma once
namespace mel
{
	namespace tasking
	{
		//!@brief Result from functor used in a GenericProcess
		enum class EGenericProcessResult:char{
			CONTINUE, //!< Continue executing
			KILL //!< Kill the process
			};
	}
}