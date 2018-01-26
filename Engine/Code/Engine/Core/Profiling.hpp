#pragma once
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Logging.hpp"

#define PROFILE_LOG_SCOPE(s) ProfileLogScope __pscope_##__LINE__##(s)
#define PROFILE_SCOPE_FUNCTION() PROFILE_LOG_SCOPE(__FUNCTION__)



void ProfilerPush(char const *tag);
void ProfilerPop();

void RegisterProfilerCommands();
void ProfilerShutdown();
void SetFrameEndTime(uint64_t op_count);
void SetFrameStartTime(uint64_t op_count);
void ProfilerStartup();
uint64_t GetTotalFrameTime();


class ProfileLogScope
{
public:
	ProfileLogScope(char const *tag)
	{
		ProfilerPush(tag);
	}

	~ProfileLogScope()
	{
		ProfilerPop();
	}
};
