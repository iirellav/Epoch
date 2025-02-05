#pragma once

#define EPOCH_ENABLE_PROFILING _RUNTIME && !_DIST || !_RUNTIME

#if EPOCH_ENABLE_PROFILING
#include <tracy/Tracy.hpp>
#define EPOCH_PROFILE_MARK_FRAME		FrameMark
#define EPOCH_PROFILE_FUNC(...)			ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
#define EPOCH_PROFILE_SCOPE(NAME)		ZoneScoped; ZoneName(NAME, strlen(NAME))
#else
#define EPOCH_PROFILE_MARK_FRAME
#define EPOCH_PROFILE_FUNC(...)
#define EPOCH_PROFILE_SCOPE(NAME)
#endif
