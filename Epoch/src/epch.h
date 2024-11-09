#pragma once

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <shellapi.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#include <CommonUtilities/Math/Vector/Vector2.hpp>
#include <CommonUtilities/Math/Vector/Vector3.hpp>
#include <CommonUtilities/Math/Vector/Vector4.hpp>

#include <CommonUtilities/Math/Matrix/Matrix3x3.hpp>
#include <CommonUtilities/Math/Matrix/Matrix4x4.hpp>

#include <CommonUtilities/Math/Quaternion.hpp>

#include <CommonUtilities/Math/Random.h>
#include <CommonUtilities/Math/CommonMath.hpp>

#include <CommonUtilities/StringUtils.h>


#include "Epoch/Core/Base.h"
#include "Epoch/Debug/Log.h"
#include "Epoch/Debug/Timer.h"
#include "Epoch/Debug/Instrumentor.h"

#include "Epoch/Utils/FileSystem.h"
#include "Epoch/Utils/YAMLSerializationHelpers.h"
