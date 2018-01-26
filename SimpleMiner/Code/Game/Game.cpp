#include "Game/Game.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Render/SpriteSheet.hpp"
#include "Game/BlockDefinition.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include <string>
#include <stdio.h>

//-----------------------------------------------------------------------------------------------
Game* g_theGame = nullptr;
Renderer* g_myRenderer = nullptr;

//-----------------------------------------------------------------------------------------------
Game::Game()
	:m_isQuitting(false)
	, m_screenShakeMagnitude(0.f)
	, m_canWeShake(true)
	, m_jumpTimer(0.f)
	, m_mouseYaw(0.f)
	,m_mousePitch(0.f)
	,m_playBackground(true)
	,m_soundTimer(0.f)
	, m_reelTimer(0.f)
{
	g_theInputSystem->SetMouseHiddenWhenFocused(true);
	CreatePlayer();
	ReadPlayerInfoFromMemory();
	m_world = new World();
}


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
	WritePlayerInfoToMemory();
	m_world->~World();
}


//-----------------------------------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	Vector3 playerBottomCenter = m_player->m_position - Vector3(0.f, 0.f, 0.98f);
	BlockInfo blockAtPlayersBottomCenter = g_theGame->m_world->GetBlockInfoAtWorldPosition(playerBottomCenter);
	if (g_IsPlayerWalking && (m_player->m_velocity.z != 0.f || !blockAtPlayersBottomCenter.IsBlockSolid()))
		g_theGame->m_player->m_velocity.z += GRAVITY * deltaSeconds;

	if(m_playBackground)
	{
		AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(2);
		SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/Background.mp3");
		g_theAudioSystem->PlaySound(pauserSound, 0.02f, channel);
		m_playBackground = false;
	}

	m_soundTimer += deltaSeconds;
	if (m_soundTimer > 40.f)
	{
		m_soundTimer = 0.f;
		m_playBackground = true;
	}

	UpdateSelectedBlock();
	m_world->Update(deltaSeconds);
	UpdatePlayer(deltaSeconds);
	UpdateCameraMovement(deltaSeconds);

	// Time scale (bullet-time)!
	if (g_theInputSystem->WasKeyJustPressed('T'))
		deltaSeconds *= 0.1f;

	if (g_IsTheGamePaused == true)
	{
		deltaSeconds *= 0.0f;
	}

}

void Game::UpdateSelectedBlock()
{
	if (g_theInputSystem->WasMouseWheelScrollingDown())
		++g_selectedBlockIndex;

	if (g_theInputSystem->WasMouseWheelScrollingUp())
		--g_selectedBlockIndex;

	if (g_selectedBlockIndex > 8)
		g_selectedBlockIndex = 0;

	if (g_selectedBlockIndex < 0)
		g_selectedBlockIndex = 8;
}

void Game::UpdatePlayer(float deltaSeconds)
{
	if (m_player != nullptr)
	{
		ClampPlayerVelocity();
		m_player->Update(deltaSeconds);
	}
}

void Game::ClampPlayerVelocity()
{
	ClampWithin(m_player->m_velocity.x, 15.f, -15.f);
	ClampWithin(m_player->m_velocity.y, 15.f, -15.f);
	ClampWithin(m_player->m_velocity.z, 15.f, -15.f);
}

void Game::UpdateCameraMovement(float deltaSeconds) 
{
	if(g_theInputSystem->DoesAppHaveFocus())
	{
		UpdateCameraView();
		UpdateCameraKeyboardMovement(deltaSeconds);
	}
}

void Game::UpdateCameraView() //#TODO: Add inverted y-axis option
{
	const float YAW_RATE = -0.05f;
	const float PITCH_RATE = 0.05f;

	Vector2 screenSize = g_theInputSystem->GetScreenSize();
	Vector2 centerPos = screenSize * .5f;
	Vector2 mousePos = g_theInputSystem->GetMouseScreenCoords();
	Vector2 displacement = mousePos - centerPos;

	g_theInputSystem->SetMouseCursorPosition(centerPos);
	m_camera.m_yawDegreesAboutZ += YAW_RATE * displacement.x;
	m_camera.m_pitchDegreesAboutY += PITCH_RATE * displacement.y;
	m_camera.m_pitchDegreesAboutY = ClampWithin(m_camera.m_pitchDegreesAboutY, 89.9f, -89.9f);
}

void Game::UpdateCameraKeyboardMovement(float deltaSeconds)
{
	const float CAMERA_MOVE_SPEED_WALK = 4.5f;
	float runningSpeed = 1.f;
	Vector3 moveDirection(0.f, 0.f, 0.f);
	Vector3 playerDirection(0.f, 0.f, 0.f);

	SetCameraAndPlayerMoveDirections(moveDirection, playerDirection, deltaSeconds, runningSpeed);

	float movementRate = runningSpeed * CAMERA_MOVE_SPEED_WALK * deltaSeconds;
	moveDirection.Normalize();
	playerDirection.Normalize();
	Vector3 moveThisFrame = moveDirection * movementRate;
	Vector3 playerMoveThisFrame = playerDirection * movementRate;
	m_player->m_velocity += playerMoveThisFrame;

	SetCameraMode();
	ApplyCameraMode(moveThisFrame);

	bool hookReeled = false;
	if (g_theInputSystem->IsKeyDown(RIGHT_MOUSE) && m_player->m_firedHookShot != nullptr)
	{
		m_world->PullPlayerTowardHookShot(deltaSeconds);
		hookReeled = true;
	}
	else if (g_theInputSystem->WasKeyJustPressed(RIGHT_MOUSE))
	{
		g_playerDestroyedBlock = true;
	}

	m_reelTimer += deltaSeconds;
	if (hookReeled && m_reelTimer > 5.f)
	{
		AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(1);
		SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/HookShotPull.mp3");
		g_theAudioSystem->PlaySound(pauserSound, 0.05f, channel);
		m_reelTimer = 0.f;
	}

	if (g_theInputSystem->WasKeyJustPressed(LEFT_MOUSE))
	{
		g_playerPlacedBlock = true;
		if (m_player->m_blockList[g_selectedBlockIndex].m_blockTypeIndex == BlockType::HOOKSHOT)
		{
			m_world->FireHookShot();
			AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(1);
			SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/HookShotFired.wav");
			g_theAudioSystem->PlaySound(pauserSound, 0.05f, channel);
		}
	}
}

void Game::ApplyCameraMode(const Vector3& cameraMoveThisFrame)
{
	if (g_cameraNoClip)
	{
		m_camera.m_position += cameraMoveThisFrame;
		m_player->m_velocity = Vector3(0.f, 0.f, 0.f);
	}
	else if (g_cameraFirstPerson)
	{
		m_camera.m_position = m_player->m_position + Vector3(0.f, 0.f, 0.69f);
	}
	else if (g_cameraFromBehind)
	{
		float rayCastDist = m_world->RaycastDistanceCanTravel(m_player->m_position + Vector3(0.f, 0.f, 0.69f), -1.f * m_camera.GetForwardXYZ());
		rayCastDist = ClampWithin(rayCastDist, 4.f, -1.5f);
		m_camera.m_position = (m_player->m_position + Vector3(0.f, 0.f, 0.69f)) + (rayCastDist * -1.f * m_camera.GetForwardXYZ());
	}
	else if (g_cameraFixedAngle)
	{
		m_camera.m_yawDegreesAboutZ = 45.f;
		m_camera.m_pitchDegreesAboutY = 20.f;
		m_camera.m_rollDegreesAboutX = 0.f;
		float rayCastDist = m_world->RaycastDistanceCanTravel(m_player->m_position + Vector3(0.f, 0.f, 0.69f), -1.f * m_camera.GetForwardXYZ());
		rayCastDist = ClampWithin(rayCastDist, 3.f, -1.5f);
		m_camera.m_position = (m_player->m_position + Vector3(0.f, 0.f, 0.69f)) + (rayCastDist * -1.f * m_camera.GetForwardXYZ());
	}
}

void Game::SetCameraAndPlayerMoveDirections(Vector3& cameraMoveDirection, Vector3& playerDirection, float deltaSeconds, float runningSpeed)
{
	if (g_theInputSystem->IsKeyDown(KEY_SHIFT))
		runningSpeed = 8.f;

	if (!g_IsPlayerWalking && g_theInputSystem->IsKeyDown(KEY_SPACEBAR))
	{
		cameraMoveDirection.z += runningSpeed * 6.0f;
		m_player->m_velocity.z += runningSpeed * 1.0f;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_SPACEBAR) && g_IsPlayerWalking)
	{
		g_playerJumped = true;
		BlockInfo blockBelowPlayer = m_world->GetBlockInfoAtWorldPosition(m_player->m_position + Vector3(0.f, 0.f, -1.2f));
		if(blockBelowPlayer.IsBlockSolid())
			m_player->m_velocity.z += 6.0f;
	}

	if (g_theInputSystem->IsKeyDown(KEY_CONTROL))
	{
		cameraMoveDirection.z -= runningSpeed * 10.f;
		m_player->m_velocity.z -= runningSpeed * 1.0f;
	}

	Vector3 cameraFWD = m_camera.GetForwardXY();
	Vector3 cameraLeft = m_camera.GetLeftXY();
	if (g_theInputSystem->IsKeyDown('W'))
	{
		if (!g_cameraFixedAngle)
			playerDirection += cameraFWD;
		else
			playerDirection += GetMouseForwardXYZ();
		cameraMoveDirection += cameraFWD;
	}
	if (g_theInputSystem->IsKeyDown('S'))
	{
		if (!g_cameraFixedAngle)
			playerDirection -= cameraFWD;
		else
			playerDirection -= GetMouseForwardXYZ();
		cameraMoveDirection -= cameraFWD;
	}
	if (g_theInputSystem->IsKeyDown('A'))
	{
		if (!g_cameraFixedAngle)
			playerDirection -= cameraLeft;
		else
			playerDirection -= GetMouseLeftXY();
		cameraMoveDirection -= cameraLeft;
	}
	if (g_theInputSystem->IsKeyDown('D'))
	{
		if (!g_cameraFixedAngle)
			playerDirection += cameraLeft;
		else
			playerDirection += GetMouseLeftXY();
		cameraMoveDirection += cameraLeft;
	}
}

void Game::SetCameraMode()
{
	if (g_theInputSystem->WasKeyJustPressed(KEY_F5))
	{
		if (g_cameraFirstPerson)
		{
			g_cameraFirstPerson = false;
			g_cameraFromBehind = true;
			g_cameraFixedAngle = false;
			g_cameraNoClip = false;
		}
		else if (g_cameraFromBehind)
		{
			g_cameraFirstPerson = false;
			g_cameraFromBehind = false;
			g_cameraFixedAngle = true;
			g_cameraNoClip = false;
		}
		else if (g_cameraFixedAngle)
		{
			g_cameraFirstPerson = false;
			g_cameraFromBehind = false;
			g_cameraFixedAngle = false;
			g_cameraNoClip = true;
		}
		else if (g_cameraNoClip)
		{
			g_cameraFirstPerson = true;
			g_cameraFromBehind = false;
			g_cameraFixedAngle = false;
			g_cameraNoClip = false;
		}
	}
}

//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	// Set up the coordinate system we want (0,0 to 1600,900) - technically unrelated to
	//	the actual size of the window, but we DO want the same (16:9) aspect ratio.
	Vector2 bottomLeft(0.f, 0.f);
	Vector2 topRight(1600.f, 900.f);
	g_myRenderer->SetOrtho2D(bottomLeft, topRight);

	// Clear the screen
	g_myRenderer->EnableDepthTestAndWrite();
	Rgba clearColor( 126, 192, 238, 255 );
	g_myRenderer->ClearScreen(clearColor);
	g_myRenderer->EnableBackFaceCulling();

	g_myRenderer->SetPerspective(60.f, (16.f / 9.f), 0.01f, 1000.f);

	//Put +X Forward, +Z Up, and +Y Left
	g_myRenderer->RotateDrawing(-90.f, 1.f, 0.f, 0.f);
	g_myRenderer->RotateDrawing(90.f, 0.f, 0.f, 1.f);

	//Anti-rotate camera yaw, pitch, roll
	g_myRenderer->RotateDrawing(-1.f * m_camera.m_rollDegreesAboutX, 1.f, 0.f, 0.f);
	g_myRenderer->RotateDrawing(-1.f * m_camera.m_pitchDegreesAboutY, 0.f, 1.f, 0.f);
	g_myRenderer->RotateDrawing(-1.f * m_camera.m_yawDegreesAboutZ, 0.f, 0.f, 1.f);

	//Anti-translate to camera
	g_myRenderer->TranslateDrawing3D(m_camera.m_position * -1.f);


	float axisLength = 5.f;
	g_myRenderer->StartManipulatingTheDrawnObject();
	//Draw World
	m_world->Render();
	//Draw Axis
	g_myRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(axisLength, 0.f, 0.f), Rgba(255, 0, 0), Rgba(255, 0, 0), 3.f);
	g_myRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(0.f, axisLength, 0.f), Rgba(0, 255, 0), Rgba(0, 255, 0), 3.f);
	g_myRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, axisLength), Rgba(0, 0, 255), Rgba(0, 0, 255), 3.f);

	g_myRenderer->DisableDepthTestAndWrite();

	g_myRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(axisLength, 0.f, 0.f), Rgba(255, 0, 0, 100), Rgba(255, 0, 0, 100), 1.f );
	g_myRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(0.f, axisLength, 0.f), Rgba(0, 255, 0, 100), Rgba(0, 255, 0, 100), 1.f );
	g_myRenderer->DrawLine3D(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, axisLength), Rgba(0, 0, 255, 100), Rgba(0, 0, 255, 100), 1.f );
	g_myRenderer->EndManipulationOfDrawing();
	//DrawPlayer
	//DrawPlaceAndDigLength();
	RenderPlayer();

	//Start HUD and UI Here
	DrawHudElements(bottomLeft, topRight);
}

void Game::DrawPlaceAndDigLength() const
{
	g_myRenderer->EnableDepthTestAndWrite();
	float rayCastDist = m_world->RaycastDistanceCanTravel(m_player->m_position + Vector3(0.f, 0.f, 0.69f), m_camera.GetForwardXYZ());
	rayCastDist = ClampWithin(rayCastDist, 8.f, 0.f);
	Vector3 startingPoint = m_player->m_position + Vector3(0.f, 0.f, 0.69f);
	Vector3 endPoint = (rayCastDist * m_camera.GetForwardXYZ()) + m_player->m_position;
	g_myRenderer->DrawLine3D(startingPoint, endPoint, Rgba(255, 255, 0), Rgba(255, 255, 0), 2.f);
	g_myRenderer->DisableDepthTestAndWrite();
}

void Game::DrawHudElements(const Vector2& bottomLeftOfOrtho, const Vector2& topRightOfOrtho) const
{
	g_myRenderer->SetOrtho2D(bottomLeftOfOrtho, topRightOfOrtho);
	BitmapFont* font = g_myRenderer->CreateOrGetFont("PixelFont");

	Vector2 startBottomLeft(10.f, SCREEN_HEIGHT - 20.f);

	DrawCameraPositionText(startBottomLeft, font);
	DrawCameraOrientationText(startBottomLeft, font);
	DrawCurrentPlayerChunkText(startBottomLeft, font);
	DrawPlayerVelocityText(startBottomLeft, font);
	DrawPlayerPositionText(startBottomLeft, font);
	DrawMovementModeText(startBottomLeft, font);
	DrawCameraModeText(startBottomLeft, font);
	DrawCrossHairs();
	DrawSelectedBlock();
	DrawPlayerBlockList();
}

void Game::DrawSelectedBlock() const
{
	AABB2D bounds;
	bounds.mins = Vector2(SCREEN_WIDTH / 3.f - 50.f + (82.5f * g_selectedBlockIndex), 20.f);
	bounds.maxs = Vector2(SCREEN_WIDTH / 3.f + 30.f + (82.5f * g_selectedBlockIndex), 100.f);

	g_myRenderer->SetLineWidth(5.f);
	g_myRenderer->DrawAABB2D(bounds, Rgba(255, 255, 255), PRIMITIVE_LINE_LOOP);
}

void Game::DrawPlayerBlockList() const 
{
	Rgba white(255, 255, 255);

	BlockDefinition block1 = BlockDefinition(m_player->m_blockList[0].m_blockTypeIndex);
	block1.m_tint = white;
	BlockDefinition block2 = BlockDefinition(m_player->m_blockList[1].m_blockTypeIndex);
	block2.m_tint = white;
	BlockDefinition block3 = BlockDefinition(m_player->m_blockList[2].m_blockTypeIndex);
	block3.m_tint = white;
	BlockDefinition block4 = BlockDefinition(m_player->m_blockList[3].m_blockTypeIndex);
	block4.m_tint = white;
	BlockDefinition block5 = BlockDefinition(m_player->m_blockList[4].m_blockTypeIndex);
	block5.m_tint = white;
	BlockDefinition block6 = BlockDefinition(m_player->m_blockList[5].m_blockTypeIndex);
	block6.m_tint = white;
	BlockDefinition block7 = BlockDefinition(m_player->m_blockList[6].m_blockTypeIndex);
	block7.m_tint = white;
	BlockDefinition block8 = BlockDefinition(m_player->m_blockList[7].m_blockTypeIndex);
	block8.m_tint = white;
	BlockDefinition block9 = BlockDefinition(m_player->m_blockList[8].m_blockTypeIndex);
	block9.m_tint = white;

	block1.m_front.m_vOne.m_position = Vector3(SCREEN_WIDTH / 3.f - 50.f, 100.f, 0.f);
	block1.m_front.m_vTwo.m_position = Vector3(SCREEN_WIDTH / 3.f - 50.f, 20.f, 0.f);
	block1.m_front.m_vThree.m_position = Vector3(SCREEN_WIDTH / 3.f + 30.f, 20.f, 0.f);
	block1.m_front.m_vFour.m_position = Vector3(SCREEN_WIDTH / 3.f + 30.f, 100.f, 0.f);

	block2.m_front.m_vOne.m_position = Vector3(block1.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block2.m_front.m_vTwo.m_position = Vector3(block1.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block2.m_front.m_vThree.m_position = Vector3(block1.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block2.m_front.m_vFour.m_position = Vector3(block1.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);
	
	block3.m_front.m_vOne.m_position = Vector3(block2.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block3.m_front.m_vTwo.m_position = Vector3(block2.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block3.m_front.m_vThree.m_position = Vector3(block2.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block3.m_front.m_vFour.m_position = Vector3(block2.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	block4.m_front.m_vOne.m_position = Vector3(block3.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block4.m_front.m_vTwo.m_position = Vector3(block3.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block4.m_front.m_vThree.m_position = Vector3(block3.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block4.m_front.m_vFour.m_position = Vector3(block3.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	block5.m_front.m_vOne.m_position = Vector3(block4.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block5.m_front.m_vTwo.m_position = Vector3(block4.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block5.m_front.m_vThree.m_position = Vector3(block4.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block5.m_front.m_vFour.m_position = Vector3(block4.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	block6.m_front.m_vOne.m_position = Vector3(block5.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block6.m_front.m_vTwo.m_position = Vector3(block5.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block6.m_front.m_vThree.m_position = Vector3(block5.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block6.m_front.m_vFour.m_position = Vector3(block5.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	block7.m_front.m_vOne.m_position = Vector3(block6.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block7.m_front.m_vTwo.m_position = Vector3(block6.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block7.m_front.m_vThree.m_position = Vector3(block6.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block7.m_front.m_vFour.m_position = Vector3(block6.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	block8.m_front.m_vOne.m_position = Vector3(block7.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block8.m_front.m_vTwo.m_position = Vector3(block7.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block8.m_front.m_vThree.m_position = Vector3(block7.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block8.m_front.m_vFour.m_position = Vector3(block7.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	block9.m_front.m_vOne.m_position = Vector3(block8.m_front.m_vOne.m_position.x + 82.5f, 100.f, 0.f);
	block9.m_front.m_vTwo.m_position = Vector3(block8.m_front.m_vTwo.m_position.x + 82.5f, 20.f, 0.f);
	block9.m_front.m_vThree.m_position = Vector3(block8.m_front.m_vThree.m_position.x + 82.5f, 20.f, 0.f);
	block9.m_front.m_vFour.m_position = Vector3(block8.m_front.m_vFour.m_position.x + 82.5f, 100.f, 0.f);

	g_myRenderer->DrawTexturedQuad3D(block1.m_front.m_vOne, block1.m_front.m_vTwo, block1.m_front.m_vThree, block1.m_front.m_vFour, *block1.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block2.m_front.m_vOne, block2.m_front.m_vTwo, block2.m_front.m_vThree, block2.m_front.m_vFour, *block2.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block3.m_front.m_vOne, block3.m_front.m_vTwo, block3.m_front.m_vThree, block3.m_front.m_vFour, *block3.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block4.m_front.m_vOne, block4.m_front.m_vTwo, block4.m_front.m_vThree, block4.m_front.m_vFour, *block4.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block5.m_front.m_vOne, block5.m_front.m_vTwo, block5.m_front.m_vThree, block5.m_front.m_vFour, *block5.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block6.m_front.m_vOne, block6.m_front.m_vTwo, block6.m_front.m_vThree, block6.m_front.m_vFour, *block6.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block7.m_front.m_vOne, block7.m_front.m_vTwo, block7.m_front.m_vThree, block7.m_front.m_vFour, *block7.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block8.m_front.m_vOne, block8.m_front.m_vTwo, block8.m_front.m_vThree, block8.m_front.m_vFour, *block8.m_front.m_faceTexture);
	g_myRenderer->DrawTexturedQuad3D(block9.m_front.m_vOne, block9.m_front.m_vTwo, block9.m_front.m_vThree, block9.m_front.m_vFour, *block9.m_front.m_faceTexture);
}

void Game::DrawCameraModeText(Vector2& startBottomLeft, BitmapFont* font) const
{
	if (g_cameraFirstPerson) 
	{
		g_myRenderer->DrawText2D(startBottomLeft, "Camera Mode: First Person", 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
		startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	}

	if (g_cameraFixedAngle) 
	{
		g_myRenderer->DrawText2D(startBottomLeft, "Camera Mode: Fixed Angle", 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
		startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	}

	if (g_cameraFromBehind)
	{
		g_myRenderer->DrawText2D(startBottomLeft, "Camera Mode: Third Person", 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
		startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	}

	if (g_cameraNoClip)
	{
		g_myRenderer->DrawText2D(startBottomLeft, "Camera Mode: No Clip", 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
		startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	}
}

void Game::DrawCrossHairs() const
{
	g_myRenderer->DrawLine2D(Vector2((SCREEN_WIDTH * 0.5f) - 10.f, (SCREEN_HEIGHT * 0.5f)), Vector2((SCREEN_WIDTH * 0.5f) + 10.f, (SCREEN_HEIGHT * 0.5f)), Rgba(255, 255, 255, 200), Rgba(255, 255, 255, 200), 3.f);
	g_myRenderer->DrawLine2D(Vector2((SCREEN_WIDTH * 0.5f), (SCREEN_HEIGHT * 0.5f) - 10.f), Vector2((SCREEN_WIDTH * 0.5f), (SCREEN_HEIGHT * 0.5f) + 10.f), Rgba(255, 255, 255, 200), Rgba(255, 255, 255, 200), 3.f);
}

void Game::DrawCameraPositionText(Vector2& startBottomLeft, BitmapFont* font) const
{
	g_myRenderer->DrawText2D(startBottomLeft, "Camera X: " + std::to_string(m_camera.m_position.x), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	g_myRenderer->DrawText2D(startBottomLeft, "Camera Y: " + std::to_string(m_camera.m_position.y), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	g_myRenderer->DrawText2D(startBottomLeft, "Camera Z: " + std::to_string(m_camera.m_position.z), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
}

void Game::DrawCameraOrientationText(Vector2& startBottomLeft, BitmapFont* font) const
{
	g_myRenderer->DrawText2D(startBottomLeft, "Orientation Yaw: " + std::to_string(m_camera.m_yawDegreesAboutZ), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	g_myRenderer->DrawText2D(startBottomLeft, "Orientation Pitch: " + std::to_string(m_camera.m_pitchDegreesAboutY), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
	g_myRenderer->DrawText2D(startBottomLeft, "Orientation Roll: " + std::to_string(m_camera.m_rollDegreesAboutX), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);
	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
}

void Game::DrawCurrentPlayerChunkText(Vector2& startBottomLeft, BitmapFont* font) const
{
	ChunkCoords currentChunk = m_world->ConvertWorldPositionToChunkPosition(m_player->m_position);
	g_myRenderer->DrawText2D(startBottomLeft, "Current ChunkCoords: " + std::to_string(currentChunk.x) + ", " + std::to_string(currentChunk.y), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);

	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
}

void Game::DrawPlayerVelocityText(Vector2& startBottomLeft, BitmapFont* font) const
{
	g_myRenderer->DrawText2D(startBottomLeft, "Player Velocity: " + std::to_string(m_player->m_velocity.x) + ", " + std::to_string(m_player->m_velocity.y) + ", " + std::to_string(m_player->m_velocity.z), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);

	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
}

void Game::DrawPlayerPositionText(Vector2& startBottomLeft, BitmapFont* font) const
{
	g_myRenderer->DrawText2D(startBottomLeft, "Player Position: " + std::to_string(m_player->m_position.x) + ", " + std::to_string(m_player->m_position.y) + ", " + std::to_string(m_player->m_position.z), 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);

	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
}

void Game::DrawMovementModeText(Vector2& startBottomLeft, BitmapFont* font) const
{
	g_myRenderer->DrawText2D(startBottomLeft, g_IsPlayerWalking ? "Walk Mode" : "Fly Mode", 16.f, Rgba(255, 255, 255, 255), 0.5625f, font);

	startBottomLeft = Vector2(startBottomLeft.x, startBottomLeft.y - 18.f);
}

void Game::RenderPlayer() const 
{
	if(m_player != nullptr)
		m_player->Render();
}

void Game::KeyUp(unsigned char keyThatWasJustReleased)
{
	if(keyThatWasJustReleased == KEY_ESCAPE)
	{
		m_isQuitting = true;
	}
	g_theInputSystem->OnKeyUp(keyThatWasJustReleased);
}

void Game::KeyDown(unsigned char keyThatWasJustPressed)
{
	if (keyThatWasJustPressed == 'C' && g_theInputSystem->IsKeyDown(keyThatWasJustPressed))
	{
		g_canWeDrawCosmeticCircle = !g_canWeDrawCosmeticCircle;
	}

	if (keyThatWasJustPressed == 'V' && g_theInputSystem->IsKeyDown(keyThatWasJustPressed)) 
	{
		g_canWeDrawPhysicsCircle = !g_canWeDrawPhysicsCircle;
	}

	if (keyThatWasJustPressed == 'P' && g_theInputSystem->IsKeyDown(keyThatWasJustPressed))
	{
		g_IsTheGamePaused = !g_IsTheGamePaused;
	}

	if (keyThatWasJustPressed == KEY_F3)
	{
		g_IsDebugModeOn = !g_IsDebugModeOn;
	}

	if (keyThatWasJustPressed == KEY_F6)
	{
		g_IsPlayerWalking = !g_IsPlayerWalking;
	}

	g_theInputSystem->OnKeyDown(keyThatWasJustPressed);
}

void Game::WritePlayerInfoToMemory()
{
	FILE* playerFile = fopen(Stringf("Data/Save/PlayerInfo.chocolate").c_str(), "wb");

	fwrite(&m_player->m_position.x, sizeof(float), 1, playerFile);
	fwrite(&m_player->m_position.y, sizeof(float), 1, playerFile);
	fwrite(&m_player->m_position.z, sizeof(float), 1, playerFile);

	fwrite(&m_camera.m_rollDegreesAboutX, sizeof(float), 1, playerFile);
	fwrite(&m_camera.m_pitchDegreesAboutY, sizeof(float), 1, playerFile);
	fwrite(&m_camera.m_yawDegreesAboutZ, sizeof(float), 1, playerFile);

	fclose(playerFile);
}

void Game::ReadPlayerInfoFromMemory()
{
	FILE* playerFile = fopen(Stringf("Data/Save/PlayerInfo.chocolate").c_str(), "rb");

	if(playerFile != nullptr)
	{
		fread(&m_player->m_position.x, 1, sizeof(float), playerFile);
		fread(&m_player->m_position.y, 1, sizeof(float), playerFile);
		fread(&m_player->m_position.z, 1, sizeof(float), playerFile);

		fread(&m_camera.m_rollDegreesAboutX, 1, sizeof(float), playerFile);
		fread(&m_camera.m_pitchDegreesAboutY, 1, sizeof(float), playerFile);
		fread(&m_camera.m_yawDegreesAboutZ, 1, sizeof(float), playerFile);
		
		fclose(playerFile);
	}
	else
	{
		m_camera.m_position = Vector3(-3.f, 0.5f, 75.f);
	}
}

void Game::CreatePlayer()
{
	Vector3 playerCenter(0.f, 0.f, 75.f);
	m_player = new Player(playerCenter);
}

Vector3 Game::GetMouseForwardXYZ()
{
	const float YAW_RATE = -0.05f;
	const float PITCH_RATE = 0.05f;

	Vector2 screenSize = g_theInputSystem->GetScreenSize();
	Vector2 centerPos = screenSize * .5f;
	Vector2 mousePos = g_theInputSystem->GetMouseScreenCoords();
	Vector2 displacement = mousePos - centerPos;

	g_theInputSystem->SetMouseCursorPosition(centerPos);
	m_mouseYaw += YAW_RATE * displacement.x;
	m_mousePitch += PITCH_RATE * displacement.y;
	m_mousePitch = ClampWithin(m_mousePitch, 89.9f, -89.9f);

	float x = CosInDegrees(m_mouseYaw) * CosInDegrees(m_mousePitch);
	float y = SinInDegrees(m_mouseYaw) * CosInDegrees(m_mousePitch);
	float z = -SinInDegrees(m_mousePitch);
	return Vector3(x, y, z);
}

Vector3 Game::GetMouseLeftXY()
{
	Vector3 fwdVector = GetMouseForwardXYZ();
	float x = fwdVector.y;
	float y = -1.f * fwdVector.x;
	return Vector3(x, y, 0.f);
}