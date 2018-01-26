#pragma once

#include "Game/App.hpp"

const int WORLD_HEIGHT = 900;
const int WORLD_WIDTH = 1600;
const float Z_DEPTH_FROM_CAMERA = 50.0f;

extern bool g_canWeDrawCosmeticCircle;
extern bool g_canWeDrawPhysicsCircle;
extern bool g_IsTheGamePaused;

struct timeConstants
{
	float GAME_TIME;
	float SYSTEM_TIME;
	float GAME_FRAME_TIME;
	float SYSTEM_FRAME_TIME;
};


class AudioSystem;
extern AudioSystem* g_theAudioSystem;

extern App* g_theApp;