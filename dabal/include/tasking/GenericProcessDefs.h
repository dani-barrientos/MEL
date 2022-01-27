#pragma once

namespace tasking
{
	/**
	* @brief GenericProcess state
	*/
	enum class EGenericProcessState : char {
		 INIT,
		 RUN,
		 KILL
	 };
	 //!@brief Result from functor used in a GenericProcess
	enum class EGenericProcessResult:char{ CONTINUE,KILL};
}
