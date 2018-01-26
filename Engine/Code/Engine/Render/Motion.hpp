#pragma once
#include "Engine/Render/Pose.hpp"
#include <string>
#include <vector>

class BinaryStream;

enum ePlayMode {
	FORWARD_SINGLE,
	REVERSE_SINGLE,
	FORWARD_LOOP,
	REVERSE_LOOP,
	PINGPONG,
	NUM_PLAYMODES
};

class Motion
{
public:
	Motion();
	~Motion();
	void SetName(std::string& newName);
	void SetDuration(float time);
	void SetFrameRate(float newRate);
	float GetDuration() const;
	unsigned int GetFrameCount();
	void Evaluate(Pose *out, float time, ePlayMode playMode = FORWARD_LOOP) const;
	float CalculateInterpolationValue(float evalTime, int firstFrame, float time, ePlayMode playMode) const;
	int CalculateFirstFrameIndexFromEvaluatedFrameTime(float evalTime, ePlayMode playMode, float time) const;
	int CalculateLastFrameIndexFromEvaluatedFrameTime(float evalTime, ePlayMode playMode, float time) const;
	float CalculateTimeInFrameFromPlayMode(float time, ePlayMode playMode) const;
	float ClampForwardTime(float time) const;
	float LoopForwardTime(float time) const;
	float ClampReverseTime(float time) const;
	float LoopReverseTime(float time) const;
	Pose* GetPose(unsigned int index);
	void WriteToStream(BinaryStream* stream);
	void ReadFromStream(BinaryStream* stream);
public:
	std::string m_name;
	float m_framerate; 
	std::vector<Pose> m_poses;
};

ePlayMode ConvertStringToPlayMode(const std::string& string);
