#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
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