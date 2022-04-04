
#pragma once
#include <core/Callback.h>
#include <algorithm>
using std::for_each;
#include <mpl/deleter.h>
using mpl::del_ptr;
#include <list>
using std::list;
#include <core/CriticalSection.h>
using core::CriticalSection; //preferir�a no hacer esto y proporcionar el objeto "bloqueador" como argumento, pero no me da tiempo
using core::Lock;

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
#include <deque>
#include <memory>
#include <functional>
#include <mpl/IsSame.h>
#include <text/logger.h>
#undef INCLUDE_PATH
#define INCLUDE_PATH <core/CallbackSubscriptor_Impl.h>
#include <mpl/VarArgs.h>

