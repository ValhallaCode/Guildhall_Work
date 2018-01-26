#include "Game/Player.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/BlockInfo.hpp"
#include "Game/Block.hpp"
#include "Engine/Audio/AudioSystem.hpp"

Player::Player(Vector3 position)
	:m_position(position)
	,m_velocity(0.f,0.f,0.f)
	,m_orientationInDegrees(90.f, 0.f, 0.f) //Yaw,Pitch,Roll
	, m_firedHookShot(nullptr)
	, m_swingTimer(0.f)
{
	Vector3 bodyMins(m_position.x - 0.3f, m_position.y - 0.3f, m_position.z - 0.63f);
	Vector3 bodyMaxs(m_position.x + 0.3f, m_position.y + 0.3f, m_position.z + 0.63f);
	m_body = AABB3D(bodyMins, bodyMaxs);
	GeneratePlayerBlockList();
}

Player::~Player()
{

}

void Player::Update(float deltaSeconds)
{
	SwitchFromHookShot();
	HookShotMovement(deltaSeconds);

	m_position += m_velocity * deltaSeconds;

	Vector3 bodyMins(m_position.x - 0.3f, m_position.y - 0.3f, m_position.z - 0.93f);
	Vector3 bodyMaxs(m_position.x + 0.3f, m_position.y + 0.3f, m_position.z + 0.93f);
	m_body = AABB3D(bodyMins, bodyMaxs);

	if (m_firedHookShot != nullptr)
		m_firedHookShot->Update(deltaSeconds);

	CheckCollisionInHookLine();
}

void Player::SwitchFromHookShot()
{
	if (m_firedHookShot == nullptr)
		return;

	if (m_blockList[g_selectedBlockIndex].m_blockTypeIndex == BlockType::HOOKSHOT)
		return;

	AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(1);
	SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/HookShotSnap.wav");
	g_theAudioSystem->PlaySound(pauserSound, 0.05f, channel);

	delete m_firedHookShot;
	m_firedHookShot = nullptr;
}

void Player::Render() const
{
	Rgba playerWireColor(255, 128, 0);
	g_myRenderer->EnableDepthTestAndWrite();

	if (m_firedHookShot != nullptr)
		m_firedHookShot->Render();

	g_myRenderer->StartManipulatingTheDrawnObject();
	g_myRenderer->SetLineWidth(2.f);
	g_myRenderer->DrawAABB3D(m_body, playerWireColor, PRIMITIVE_QUADS);

	g_myRenderer->DisableDepthTestAndWrite();
	playerWireColor.a = 50;
	g_myRenderer->DrawAABB3D(m_body, playerWireColor, PRIMITIVE_QUADS);
	g_myRenderer->EndManipulationOfDrawing();
}

void Player::GeneratePlayerBlockList()
{
	m_blockList[0] = Block(HOOKSHOT, 0b00010000);
	m_blockList[1] = Block(STONE, 0b01110000);
	m_blockList[2] = Block(GLOWSTONE, 0b01111100);
	m_blockList[3] = Block(SAND, 0b01110000);
	m_blockList[4] = Block(GRASS, 0b01110000);
	m_blockList[5] = Block(WOOD, 0b01110000);
	m_blockList[6] = Block(BRICK, 0b01110000);
	m_blockList[7] = Block(STONE_BRICK, 0b01110000);
	m_blockList[8] = Block(MOSS_STONE_BRICK, 0b01110000);
}

void Player::HookShotMovement(float deltaSeconds)
{
	if (m_firedHookShot == nullptr)
		return;

	if (m_firedHookShot->m_firedPosition != m_firedHookShot->m_endPosition)
		return;

	Vector3 playerBottomCenter = m_position - Vector3(0.f, 0.f, 0.93f);
	BlockInfo blockAtPlayersBottomCenter = g_theGame->m_world->GetBlockInfoAtWorldPosition(playerBottomCenter);

	if (blockAtPlayersBottomCenter.IsBlockSolid())
		return;

	Vector3 directionFromHook = m_position - m_firedHookShot->m_endPosition;
	directionFromHook.Normalize();
	float speedAwayFromAnchor = DotProduct(m_velocity, directionFromHook);

	if(speedAwayFromAnchor > 0.f) //#TODO: Swing sound
	{
		Vector3 velocityOppositeHook = directionFromHook * speedAwayFromAnchor;
		m_velocity -= (velocityOppositeHook);

		
		if(m_swingTimer == 0.f)
		{
			AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(4);
			SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/Wind.wav");
			g_theAudioSystem->PlaySound(pauserSound, 0.12f, channel);
			
		}
		m_swingTimer += deltaSeconds;
		if(m_swingTimer > 24.f)
			m_swingTimer = 0.f;
	}
}

void Player::CheckCollisionInHookLine()
{
	if (m_firedHookShot == nullptr)
		return;

	const int Num_Steps = 1000;
	Vector3 forwardVec = m_firedHookShot->m_endPosition - m_position;
	Vector3 displacement = forwardVec * 50.f;
	Vector3 singleStep = displacement / (float)Num_Steps;
	Vector3 currentWorldPos = m_firedHookShot->m_currentPosition;
	Vector3 startPosition = currentWorldPos;
	BlockInfo currentBlockInfo = g_theGame->m_world->GetBlockInfoAtWorldPosition(currentWorldPos);

	for (int step = 0; step < Num_Steps; step++)
	{
		currentWorldPos = startPosition + (singleStep * (float)step);
		currentBlockInfo = g_theGame->m_world->GetBlockInfoAtWorldPosition(currentWorldPos);
			
		if (currentBlockInfo.m_chunk == m_firedHookShot->m_anchorBlock.m_chunk && currentBlockInfo.m_blockIndex == m_firedHookShot->m_anchorBlock.m_blockIndex)
			return;

		if (currentBlockInfo.IsBlockSolid())
		{
			AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(1);
			SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/HookShotSnap.wav");
			g_theAudioSystem->PlaySound(pauserSound, 0.05f, channel);

			delete m_firedHookShot;
			m_firedHookShot = nullptr;
			return;
		}
		
	}
}