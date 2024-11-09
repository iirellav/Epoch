#pragma once
#include "Log.h"

#define DEBUG_BREAK __debugbreak()

#define ASSERT_MESSAGE_INTERNAL(...)  ::Epoch::Log::PrintAssertMessage("Assertion Failed" __VA_OPT__(,) __VA_ARGS__)

#define EPOCH_ASSERT(condition, ...) { if(!(condition)) { ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); DEBUG_BREAK; } }
