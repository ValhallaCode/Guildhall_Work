//-----------------------------------------------------------------------------------------------
// Time.hpp
//	A simple high-precision time utility function for Windows
//	based on code by Squirrel Eiserloh
#pragma once
#include <stdint.h>
#include <string>

typedef unsigned int uint;

class InternalTimeSystem
{
public:
	InternalTimeSystem();

	uint64_t start_ops;
	uint64_t ops_per_second;

	double seconds_per_op;
};

double GetCurrentTimeSeconds();
void SleepSeconds(float secondsToSleep);
std::string TimeOpCountToString(uint64_t op_count);
uint64_t TimeOpCountFrom_ms(double ms);
double TimeOpCountTo_ms(uint64_t op_count);
uint64_t TimeOpCountTo_us(uint64_t op_count);
double __fastcall TimeGetSeconds();
uint __fastcall TimeGet_us();
uint __fastcall TimeGet_ms();
uint64_t __fastcall TimeGetOpCount();
double TimeOpCountToSeconds(uint64_t op_count);
float CalculateDeltaSeconds();
