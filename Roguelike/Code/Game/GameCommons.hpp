#pragma once

#include "Game/App.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/Input/Input.hpp"

const int WORLD_HEIGHT = 18;
const int WORLD_WIDTH = 32;
const float Z_DEPTH_FROM_CAMERA = 50.0f;

extern bool g_canWeDrawCosmeticCircle;
extern bool g_canWeDrawPhysicsCircle;
extern bool g_IsTheGamePaused;
extern bool g_debug;

struct timeConstants
{
	float GAME_TIME;
	float SYSTEM_TIME;
	float GAME_FRAME_TIME;
	float SYSTEM_FRAME_TIME;
};

class Input;
extern Input* g_theInputSystem;

class AudioSystem;
extern AudioSystem* g_theAudioSystem;

extern App* g_theApp;


extern SimpleRenderer* g_simpleRenderer;

class Game;
extern Game* g_theGame;