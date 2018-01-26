#include "Engine/Render/Motion.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Input/BinaryStream.hpp"
#include <cmath>
#include <locale>

Motion::Motion()
	:m_framerate(0.1f)
{

}

Motion::~Motion()
{

}

void Motion::SetName(std::string& newName)
{
	m_name = newName;
}

void Motion::SetDuration(float time)
{
	m_poses.resize(1 + (int)(ceil(time / m_framerate)));
}

void Motion::SetFrameRate(float newRate)
{
	m_framerate = newRate;
}

float Motion::GetDuration() const
{
	return (m_poses.size() - 1) / m_framerate;
}

unsigned int Motion::GetFrameCount()
{
	return m_poses.size();
}

void Motion::Evaluate(Pose *out, float time, ePlayMode playMode /*= FORWARD_LOOP*/) const
{
	float evalFrame = CalculateTimeInFrameFromPlayMode(time, playMode);

	int firstFrame = CalculateFirstFrameIndexFromEvaluatedFrameTime(evalFrame, playMode, time);
	int lastFrame = CalculateLastFrameIndexFromEvaluatedFrameTime(evalFrame, playMode, time);

	Pose first = m_poses[firstFrame]; 
	Pose last = m_poses[lastFrame];

	float interVal = CalculateInterpolationValue(evalFrame, firstFrame, time, playMode);

	for(unsigned int index = 0; (index < first.m_localTransforms.size()) || (index < last.m_localTransforms.size()); ++index)
	{
		Transform trans;
		trans.position = Interpolate(first.m_localTransforms[index].position, last.m_localTransforms[index].position, interVal); // evalFrame - firstFrame
		trans.scale = Interpolate(first.m_localTransforms[index].scale, last.m_localTransforms[index].scale, interVal);
		trans.rotation = SLERP(first.m_localTransforms[index].rotation, last.m_localTransforms[index].rotation, interVal);
		trans.rotation.Normalize();
		out->m_localTransforms.push_back(trans);
	}
}

float Motion::CalculateInterpolationValue(float evalTime, int firstFrame, float time, ePlayMode playMode) const
{
	if (playMode == FORWARD_SINGLE || playMode == FORWARD_LOOP)
	{
		return evalTime - (float)firstFrame;
	}
	else if (playMode == REVERSE_SINGLE || playMode == REVERSE_LOOP)
	{
		return (float)firstFrame - evalTime;
	}
	else if (playMode == PINGPONG)
	{
		float duration = GetDuration();
		float doubleDuration = 2.0f * duration;
		float timeInDouble = std::fmod(time, doubleDuration);

		if (timeInDouble <= duration)
			return evalTime - (float)firstFrame;
		else
			return (float)firstFrame - evalTime;
	}
	else
	{
		return evalTime - (float)firstFrame;
	}
}

int Motion::CalculateFirstFrameIndexFromEvaluatedFrameTime(float evalTime, ePlayMode playMode, float time) const
{
	if (playMode == FORWARD_SINGLE || playMode == FORWARD_LOOP)
	{
		return (int)(floor(evalTime));
	}
	else if (playMode == REVERSE_SINGLE || playMode == REVERSE_LOOP)
	{
		return (int)(ceil(evalTime));
	}
	else if (playMode == PINGPONG)
	{
		float duration = GetDuration();
		float doubleDuration = 2.0f * duration;
		float timeInDouble = std::fmod(time, doubleDuration);

		if (timeInDouble <= duration)
			return (int)(floor(evalTime));
		else
			return (int)(ceil(evalTime));
	}
	else
	{
		return (int)(floor(evalTime));
	}
}

int Motion::CalculateLastFrameIndexFromEvaluatedFrameTime(float evalTime, ePlayMode playMode, float time) const
{
	if (playMode == FORWARD_SINGLE || playMode == FORWARD_LOOP)
	{
		return (int)(ceil(evalTime));
	}
	else if (playMode == REVERSE_SINGLE || playMode == REVERSE_LOOP)
	{
		return (int)(floor(evalTime));
	}
	else if (playMode == PINGPONG)
	{
		float duration = GetDuration();
		float doubleDuration = 2.0f * duration;
		float timeInDouble = std::fmod(time, doubleDuration);

		if (timeInDouble <= duration)
			return (int)(ceil(evalTime));
		else
			return (int)(floor(evalTime));
	}
	else
	{
		return (int)(ceil(evalTime));
	}
}

float Motion::CalculateTimeInFrameFromPlayMode(float time, ePlayMode playMode) const
{
	if (playMode == FORWARD_SINGLE)
	{
		return ClampForwardTime(time) * m_framerate;
	}
	else if (playMode == FORWARD_LOOP)
	{
		return LoopForwardTime(time) * m_framerate;
	}
	else if (playMode == REVERSE_SINGLE)
	{
		return ClampReverseTime(time) * m_framerate;
	}
	else if (playMode == REVERSE_LOOP)
	{
		return LoopReverseTime(time) * m_framerate;
	}
	else if (playMode == PINGPONG)
	{
		float duration = GetDuration();
		float doubleDuration = 2.0f * duration;
		float timeInDouble = std::fmod(time, doubleDuration);

		if(timeInDouble <= duration)
			return LoopForwardTime(time) * m_framerate;
		else
		{
			float diffTime = (duration - timeInDouble);
			float absDiff = std::fabsf(diffTime);
			float calcTime = std::fmod(absDiff, duration);

			float evalTime = duration - calcTime;

			return evalTime * m_framerate;
		}
	}
	else
	{
		return (time * m_framerate);
	}
}

float Motion::ClampForwardTime(float time) const
{
	float duration = GetDuration();

	if (time < 0.0f)
		return 0.0f;
	else if (time > duration)
		return duration;
	else
		return time;
}

float Motion::LoopForwardTime(float time) const
{
	float duration = GetDuration();
	return std::fmod(time, duration);
}

float Motion::ClampReverseTime(float time) const
{
	float duration = GetDuration();

	if (time < 0.0f)
		return duration;
	else if (time > duration)
		return 0.0f;
	else
		return (duration - time);
}

float Motion::LoopReverseTime(float time) const
{
	float duration = GetDuration();
	float diffTime = (duration - time);
	float absDiff = std::fabsf(diffTime);
	float calcTime = std::fmod(absDiff, duration);

	float evalTime;
	if (absDiff > duration)
		evalTime = duration - calcTime;
	else
		evalTime = calcTime;


	return evalTime;
}

Pose* Motion::GetPose(unsigned int index)
{
	return &m_poses[index];
}

void Motion::WriteToStream(BinaryStream* stream)
{
	stream->write(m_name.size());
	stream->write(m_name);

	stream->write(m_framerate);

	stream->write(m_poses.size());
	for (uint poseIndex = 0; poseIndex < m_poses.size(); ++poseIndex)
	{
		Pose& pose = m_poses[poseIndex];
		stream->write(pose.m_localTransforms.size());
		for (uint transIndex = 0; transIndex < pose.m_localTransforms.size(); ++transIndex)
		{
			Transform& trans = pose.m_localTransforms[transIndex];
			stream->write(trans.position);
			stream->write(trans.rotation);
			stream->write(trans.scale);
		}
	}
}

void Motion::ReadFromStream(BinaryStream* stream)
{
	size_t nameSize;
	stream->read(&nameSize);
	m_name.resize(nameSize);
	stream->read(&m_name);

	stream->read(&m_framerate);

	size_t poseSize;
	stream->read(&poseSize);
	m_poses.reserve(poseSize);
	for (uint poseIndex = 0; poseIndex <poseSize; ++poseIndex)
	{
		Pose pose;
		size_t localSize;
		stream->read(&localSize);
		pose.m_localTransforms.reserve(localSize);
		for (uint transIndex = 0; transIndex < localSize; ++transIndex)
		{
			Transform trans;
			stream->read(&trans.position);
			stream->read(&trans.rotation);
			stream->read(&trans.scale);
			pose.m_localTransforms.push_back(trans);
		}
		m_poses.push_back(pose);
	}
}

ePlayMode ConvertStringToPlayMode(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (casedType == "forward_loop")
	{
		return FORWARD_LOOP;
	}
	else if (casedType == "forward_single")
	{
		return FORWARD_SINGLE;
	}
	else if (casedType == "reverse_loop")
	{
		return REVERSE_LOOP;
	}
	else if (casedType == "reverse_single")
	{
		return REVERSE_SINGLE;
	}
	else
	{
		return PINGPONG;
	}
}
