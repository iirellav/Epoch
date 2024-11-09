#pragma once

namespace Epoch
{
	void InitializeCore();
	void ShutdownCore();
}

#define BIT(x) (1u << x)

#include "Epoch/Debug/Assert.h"
