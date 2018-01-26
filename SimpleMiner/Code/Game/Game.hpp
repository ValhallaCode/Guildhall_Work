#pragma once
#include "Game/GameCommons.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Game/Camera3D.hpp"
#include "Game/World.hpp"
#include "Game/Player.hpp"
// Code help from Squirrel Eiserloh

const float CAMERA_TO_PLAYER_HEIGHT_DIFF = 0.37f;

class Game
{
public:
	Game();
	~Game();
	void Update(float deltaSeconds);
	void UpdateSelectedBlock();
	void UpdatePlayer(float deltaSeconds);
	void ClampPlayerVelocity();
	void UpdateCameraMovement(float deltaSeconds);
	void UpdateCameraView();
	void UpdateCameraKeyboardMovement(float deltaSeconds);
	void ApplyCameraMode(const Vector3& cameraMoveThisFrame);
	void SetCameraAndPlayerMoveDirections(Vector3& cameraMoveDirection, Vector3& playerDirection, float deltaSeconds, float runningSpeed);
	void SetCameraMode();
	void Render() const;
	void DrawPlaceAndDigLength() const;
	void DrawHudElements(const Vector2& bottomLeftOfOrtho, const Vector2& topRightOfOrtho) const;
	void DrawSelectedBlock() const;
	void DrawPlayerBlockList() const;
	void DrawCameraModeText(Vector2& startBottomLeft, BitmapFont* font) const;
	void DrawCrossHairs() const;
	void DrawCameraPositionText(Vector2& startBottomLeft, BitmapFont* font) const;
	void DrawCameraOrientationText(Vector2& startBottomLeft, BitmapFont* font) const;
	void DrawCurrentPlayerChunkText(Vector2& startBottomLeft, BitmapFont* font) const;
	void DrawPlayerVelocityText(Vector2& startBottomLeft, BitmapFont* font) const;
	void DrawPlayerPositionText(Vector2& startBottomLeft, BitmapFont* font) const;
	void DrawMovementModeText(Vector2& startBottomLeft, BitmapFont* font) const;
	void RenderPlayer() const;
	void KeyUp(unsigned char asKey);
	void KeyDown(unsigned char asKey);
	void WritePlayerInfoToMemory();
	void ReadPlayerInfoFromMemory();
	void CreatePlayer();
	Vector3 GetMouseForwardXYZ();
	Vector3 GetMouseLeftXY();

public:
	bool m_isQuitting;
	bool m_canWeShake;
	float m_screenShakeMagnitude;
	float m_jumpTimer;
	float m_mouseYaw;
	float m_mousePitch;
	float m_soundTimer;
	float m_reelTimer;
	bool m_playBackground;
	Camera3D m_camera;
	World* m_world;
	Player* m_player;
	IntVector3 m_raytrace;
};

extern Game* g_theGame;