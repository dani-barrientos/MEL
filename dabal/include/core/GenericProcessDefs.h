#pragma once

namespace core
{
	/**
	* some definitions used by GenericProcess
	*/
	enum class EGenericProcessState : char { INIT,RUN,KILL};
	enum class EGenericProcessResult:char{ CONTINUE,KILL};
}
