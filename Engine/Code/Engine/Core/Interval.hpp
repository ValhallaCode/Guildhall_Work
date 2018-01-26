#pragma once

typedef unsigned int uint;

class Interval
{
public:
	void SetSeconds(float seconds);
	void SetFrequency(float hz);
	bool Check();
	bool CheckAndDecrement();
	bool CheckAndReset();
	uint DecrementAll();
	void Reset();

public:
	float interval_time;
	float target_time;

};