#pragma once
#include <functional>
#include "Epoch/Scene/Entity.h"

namespace Epoch
{
	enum class PhysicsEventType : int8_t { None = -1, CollisionEnter, CollisionExit, TriggerEnter, TriggerExit };
	using PhysicsEventCallbackFn = std::function<void(PhysicsEventType, Entity, Entity)>;
}
