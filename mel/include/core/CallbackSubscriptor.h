
#pragma once
/*
 * SPDX-FileCopyrightText: 2005,2022 Daniel Barrientos <danivillamanin@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */
#include <core/Callback.h>
#include <algorithm>
using std::for_each;
#include <mpl/deleter.h>
using mel::mpl::del_ptr;
#include <list>
using std::list;
#include <mutex>
namespace mel
{
	namespace core {
		enum SubscriptionEmplacement {
			SE_BACK,
			SE_FRONT
		};
		/**
		 * @brief Type resturned by callbacks subscribed to \ref ::CallbackSubscriptor "CallbackSubscriptors"
		 * 
		 */
		enum class ECallbackResult : uint8_t{ NO_UNSUBSCRIBE,UNSUBSCRIBE};
		struct CSMultithreadPolicy{};
		struct CSNoMultithreadPolicy{};
	}
}
#include <deque>
#include <memory>
#include <functional>
#include <mpl/IsSame.h>
#include <text/logger.h>
#undef INCLUDE_PATH
#define INCLUDE_PATH <core/CallbackSubscriptor_Impl.h>
#include <mpl/VarArgs.h>

