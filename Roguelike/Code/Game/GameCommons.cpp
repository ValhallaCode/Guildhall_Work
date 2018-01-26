#include "Game/GameCommons.hpp"
#include "Engine/Math/Vector3.hpp"

bool g_canWeDrawCosmeticCircle = false;
bool g_canWeDrawPhysicsCircle = false;
bool g_IsTheGamePaused = false;
bool g_debug = false;
Input* g_theInputSystem = nullptr;
AudioSystem* g_theAudioSystem = nullptr;
App* g_theApp = nullptr;
SimpleRenderer* g_simpleRenderer = nullptr;
Game* g_theGame = nullptr;