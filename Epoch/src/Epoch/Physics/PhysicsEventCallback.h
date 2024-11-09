#pragma once
#include <functional>
#include "Epoch/Scene/Entity.h"

namespace Epoch
{
	enum class EventType : int8_t { None = -1, CollisionEnter, CollisionExit, TriggerEnter, TriggerExit };
	using EventCallbackFn = std::function<void(EventType, Entity, Entity)>;
}
