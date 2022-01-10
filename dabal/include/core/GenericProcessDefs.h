#pragma once

namespace core
{
	/**
	* some definitions used by GenericProcess
	*/
	enum class EGenericProcessState { INIT,RUN,KILL};
	enum class EGenericProcessResult { CONTINUE,KILL};
}
using core::EGenericProcessState;