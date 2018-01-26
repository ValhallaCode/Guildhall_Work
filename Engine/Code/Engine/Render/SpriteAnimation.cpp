#include "Engine/Render/SpriteAnimation.hpp"
#include "Engine/Math/MathUtils.hpp"


SpriteAnimation::SpriteAnimation(const SpriteSheet& spriteSheet, float durationSeconds, SpriteAnimMode playbackMode, int startSpriteIndex, int endSpriteIndex)
	:m_isPlaying(true)
	, m_isFinished(false)
	, m_elapsedSeconds(0.f)
	, m_spriteSheet(spriteSheet)
{
	m_durationSeconds = durationSeconds;
	m_playMode = playbackMode;
	m_startIndex = startSpriteIndex;
	m_endIndex = endSpriteIndex;
}

SpriteAnimation::~SpriteAnimation()
{
}

void SpriteAnimation::Update(float deltaSeconds)
{
	if (m_isPlaying)
	{
		m_elapsedSeconds += deltaSeconds;
		if (m_elapsedSeconds >= m_durationSeconds)
		{
			if (m_playMode == SPRITE_ANIM_MODE_LOOPING)
			{
				m_elapsedSeconds = 0.f;
			}

			if (m_playMode == SPRITE_ANIM_MODE_PLAY_TO_END)
			{
				m_isPlaying = false;
				m_isFinished = true;
			}
		}
	}
}

AABB2D SpriteAnimation::GetCurrentTexCoords() const 
{
	int currentIndex = (int)RangeMapFloat(m_elapsedSeconds, 0.f, m_durationSeconds, (float)m_startIndex, (float)m_endIndex + 1.f);
	return m_spriteSheet.GetTexCoordsForSpriteIndex(currentIndex);
}

Texture* SpriteAnimation::GetTextureGL() const
{
	return m_spriteSheet.GetSpritesheetTextureGL();
}

Texture2D* SpriteAnimation::GetTextureDX() const
{
	return m_spriteSheet.GetSpritesheetTextureDX();
}

void SpriteAnimation::Pause()
{
	m_isPlaying = false;
}

void SpriteAnimation::Resume()
{
	m_isPlaying = true;
}

void SpriteAnimation::Reset()
{
	m_elapsedSeconds = 0.f;
}

float SpriteAnimation::GetSecondsRemaining() const
{
	return m_durationSeconds - m_elapsedSeconds;
}

float SpriteAnimation::GetFractionElapsed() const
{
	return (m_elapsedSeconds / m_durationSeconds);
}

float SpriteAnimation::GetFractionRemaining() const
{
	return (GetSecondsRemaining() / m_durationSeconds);
}

void SpriteAnimation::SetSecondsElapsed(float secondsElapsed)
{
	m_elapsedSeconds = secondsElapsed;
}

void SpriteAnimation::SetFractionElapsed(float fractionElapsed)
{
	m_elapsedSeconds = fractionElapsed * m_durationSeconds;
}

