#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Render/Vertex.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <vector>

typedef IntVector2 ChunkCoords;



const int SCREEN_HEIGHT = 900;
const int SCREEN_WIDTH = 1600;
const int WORLD_HEIGHT = 900;
const int WORLD_WIDTH = 1600;

const float CHUNK_MAX_RANGE = 200.f;
const int MAX_NUM_CHUNKS = 1000;
const int DESIRED_NUM_CHUNKS = 900;

const int BLOCK_SKY_MASK = 0b10000000;
const int BLOCK_DIRTY_MASK = 0b01000000;
const int BLOCK_OPAQUE_MASK = 0b00100000;
const int BLOCK_SOLID_MASK = 0b00010000;
const int BLOCK_FLAG_MASK = 0b11110000;
const int BLOCK_LIGHT_MASK = 0b00001111;

const int CHUNK_BITS_X = 4;
const int CHUNK_BITS_Y = 4;
const int CHUNK_BITS_Z = 7;
const int CHUNK_BITS_XY = CHUNK_BITS_X + CHUNK_BITS_Y;

const int CHUNK_WIDTH_X = 1 << CHUNK_BITS_X;
const int CHUNK_DEPTH_Y = 1 << CHUNK_BITS_Y;
const int CHUNK_HEIGHT_Z = 1 << CHUNK_BITS_Z;
const int BLOCKS_PER_LAYER = 1 << CHUNK_BITS_XY;

const int CHUNK_X_MASK = CHUNK_WIDTH_X - 1;
const int CHUNK_Y_MASK = (CHUNK_DEPTH_Y - 1) << CHUNK_BITS_X;
const int CHUNK_Z_MASK = (CHUNK_HEIGHT_Z - 1) << CHUNK_BITS_XY;

const float GRAVITY = -9.8f;

extern bool g_canWeDrawCosmeticCircle;
extern bool g_canWeDrawPhysicsCircle;
extern bool g_IsTheGamePaused;
extern bool g_playerPlacedBlock;
extern bool g_playerDestroyedBlock;
extern bool g_IsDebugModeOn;
extern bool g_IsPlayerWalking;
extern bool g_playerJumped;
extern bool g_cameraFirstPerson;
extern bool g_cameraFromBehind;
extern bool g_cameraFixedAngle;
extern bool g_cameraNoClip;
extern int g_selectedBlockIndex;

class Input;
extern Input* g_theInputSystem;

class AudioSystem;
extern AudioSystem* g_theAudioSystem;

extern std::vector<Vertex3_PC> m_debugPoints;
void AddDebugPoint(const Vector3& position, const Rgba& color); // push_back in vector
void DrawDebugPoints(float pointSize, bool enableDepthTesting); // fastest using VAs!
void ClearDebugPoints(); // clear the (global) vector
Vector3 GetWorldCenterOfChunkCoords(const ChunkCoords& chunkCoords);




inline Vector3 GetWorldCenterOfChunkCoords(const ChunkCoords& chunkCoords)
{
	float x = (float)CHUNK_WIDTH_X * (0.5f + (float) chunkCoords.x);
	float y = (float)CHUNK_DEPTH_Y * (0.5f + (float) chunkCoords.y);
	float z = 0.5f * (float)CHUNK_HEIGHT_Z;
	return Vector3(x, y, z);
}


