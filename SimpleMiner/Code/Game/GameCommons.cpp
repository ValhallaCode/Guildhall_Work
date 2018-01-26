#include "Game/GameCommons.hpp"
#include "Engine/Render/Renderer.hpp"

bool g_canWeDrawCosmeticCircle = false;
bool g_canWeDrawPhysicsCircle = false;
bool g_IsTheGamePaused = false;
bool g_playerPlacedBlock = false;
bool g_playerDestroyedBlock = false;
bool g_IsDebugModeOn = false;
bool g_IsPlayerWalking = true;
bool g_playerJumped = false;
bool g_cameraFirstPerson = true;
bool g_cameraFromBehind = false;
bool g_cameraFixedAngle = false;
bool g_cameraNoClip = false;
int g_selectedBlockIndex = 0;

Input* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
std::vector<Vertex3_PC> m_debugPoints;

void AddDebugPoint(const Vector3& position, const Rgba& color) // push_back in vector
{
	Vertex3_PC vertexToAdd(position, color);
	m_debugPoints.push_back(vertexToAdd);
}

void DrawDebugPoints(float pointSize, bool enableDepthTesting) // fastest using VAs!
{
	if (enableDepthTesting)
		g_myRenderer->EnableDepthTestAndWrite();
	else
		g_myRenderer->DisableDepthTestAndWrite();
	
	g_myRenderer->SetPointSize(pointSize);
	g_myRenderer->DrawVertexArray3D_PC(&m_debugPoints[0], (int)m_debugPoints.size(), PRIMITIVE_POINTS);
}

void ClearDebugPoints() // clear the (global) vector
{
	m_debugPoints.clear();
}