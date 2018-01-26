#include "Game/HookShot.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Game/Game.hpp"
#include "Engine/Audio/AudioSystem.hpp"

HookShot::HookShot(BlockInfo anchor, Vector3 startPosition, Vector3 direction)
	:m_firedPosition(startPosition)
	,m_currentPosition(startPosition)
	,m_anchorBlock(anchor)
	,m_direction(direction)
	,m_endPosition(0.f,0.f,0.f)
	,m_velocity(0.f,0.f,0.f)
{
	m_velocity = 12.f * direction;
}

void HookShot::Update(float deltaSeconds)
{
	Vector3 dispToEnd = m_endPosition - m_currentPosition;
	Vector3 dispToFired = m_firedPosition - m_currentPosition;
	float distToEnd = dispToEnd.CalcLengthSquared();
	float distToFired = dispToFired.CalcLengthSquared();

	bool hookImpact;
	if (distToFired < distToEnd)
	{
		m_firedPosition += m_velocity * deltaSeconds;
		hookImpact = false;
	}
	else
	{
		m_firedPosition = m_endPosition;
		if (!hookImpact)
		{
			AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(1);
			SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/HookShotImpact.wav");
			g_theAudioSystem->PlaySound(pauserSound, 0.05f, channel);
		}
		hookImpact = true;
	}
	
	m_currentPosition = g_theGame->m_player->m_position;


}

void HookShot::Render() const
{
	g_myRenderer->StartManipulatingTheDrawnObject();
	g_myRenderer->DrawLine3D(m_currentPosition, m_firedPosition, Rgba(255, 255, 255), Rgba(255, 255, 255), 5.f);
	g_myRenderer->EndManipulationOfDrawing();
}
