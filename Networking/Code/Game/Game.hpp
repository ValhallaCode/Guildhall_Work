#pragma once
#include "Game/GameCommons.hpp"
#include "Game/Camera3D.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/RHI/Material.hpp"
#include "Engine/Core/CommandSystem.hpp"
#include "Engine/Network/NetAddress.hpp"
#include "Engine/Network/NetMessage.hpp"
#include <vector>

class Texture2D;
class Sampler;
class KerningFont;
class TCPSocket;
class TCPSession;
class RemoteCommandService;
class Player;
class Bullet;
class Ship;
class Asteroid;
class Landmine;
class Mesh;
class SpriteSheet;
class SpriteAnimation;
class Powerup;

const uint GAME_PORT = 8912;

enum eGameMessages : uint8_t
{
	SEND_NAME = NUM_CORE_MESSAGES,
	SYNC,
	LEAVE,
	SET_WORLD,
	SHOOT,
	INIT_JOIN,
	TOTAL_GAME_MESSAGES,
};

enum eReplicateTypes : uint8_t
{
	NETOBJECT_SHIP,
	NETOBJECT_BULLET,
	NETOBJECT_ASTEROID,
	NETOBJECT_MINE,
	NETOBJECT_PICKUP,
	TOTAL_REPLICATION_TYPES
};

enum eDMState : uint
{
	ASTEROIDS = 0,
	LANDMINES,
	SPREAD_PICKUP,
	EXPLODE_PICKUP,
	HEALTH_PICKUP,
	NUM_DM_STATES
};

struct ShipSelect
{
	bool is_selected;
};

struct Explosion
{
	Vector2 position;
	float radius;
	SpriteAnimation* anim;
};

class Game
{
public:
	Game();
	~Game();
	void Update(float deltaSeconds);
	void CreateExplosion(Vector2 pos, float radius);
	void UpdateExplosions(float deltaSeconds);
	void RenderExplosions() const;
	void SmoothShips(float deltaSeconds);
	void SmoothAsteroids(float deltaSeconds);
	void SmoothBullets();
	void HostManageRespawns();
	void UpdateShips(float deltaSeconds);
	void UpdateBullets(float deltaSeconds);
	void UpdateAsteroids(float deltaSeconds);
	void UpdateLandmines(float deltaSeconds);
	void UpdatePowerups(float deltaSeconds);
	void CheckPowerupCollisions(uint index);
	void CheckForOverlaps(Asteroid* asteroid, uint index);
	void BounceOffWorldEdge(Asteroid* asteroid);
	void CollideWithAsteroids(uint index);
	void CollideWithShip(Asteroid* asteroid);
	void LandmineCollideWithShip(uint index);
	bool CheckForBulletCollision(Bullet* bullet);
	bool IsBulletOutsideWorld(Bullet* bullet);
	void UpdateCamera(float deltaSeconds);
	void UpdateClientCamera();
	void UpdateHostCamera(float deltaSeconds);
	void UpdatePlayers(float deltaSeconds);
	void CorrectPlayerOnWorldEdge(uint player_index);
	void Render() const;
	void Start2DDraw() const;
	void DrawConsole() const;
	void Draw2DGridAndAxis() const;
	void DrawGrid2D() const;
	void DrawAxis2D() const;
	void KeyUp(unsigned char asKey);
	void KeyDown(unsigned char asKey);
	void Initialize();
	void InitializeConsole();
	uint GetFirstAvailableShipSelectIncrement(uint start_idx = 0);
	uint GetFirstAvailableShipSelectDecrement(uint start_idx = 0);
	// Game Session
	void UpdateGameSession();
	void RenderGameSessionInfo() const;
	void JoinGame(const net_address_t& addr_to_join);
	void SetUpGameMessages();
	void SendName(const std::string& name);
	void InitialSyncForPlayerInfo();
	void NetSetup();
	Ship* HostCreateShipToReplicate(uint8_t player_idx);
	void HostDestroyShipThatsReplicated(Ship *ship);
	Bullet* HostCreateBullets(uint8_t index, bool is_explosive = false);
	void HostDestroyBullet(uint index);
	Landmine* HostCreateLandmine();
	Powerup* HostCreatePowerup(uint type);
	Asteroid* HostCreateAsteroid();
	void HostDestroyAsteroid(uint index);
	void HostDestroyLandmine(uint index);
	void HostDestroyPowerup(uint index);
	// Game Data
	uint GetNumSyncedPlayers() const;
	void IncrementHostState();
	void DecrementHostState();
public:
	bool m_isQuitting;
	bool m_canDebug;
	float m_screenShakeMagnitude;
	float m_fov;
	timeConstants m_timeConst;
	ConstantBuffer* m_constBuffer;
	ShaderProgram* m_shadowBox;
	ShaderProgram* m_fontShader;
	ShaderProgram* m_skybox;
	ShaderProgram* m_defaultShader;
	Texture2D* m_defaultTex;
	Camera3D* m_camera;
	float m_totalTimePassed;
	KerningFont* m_font;

	//Network
	RemoteCommandService* m_rcs;
	TCPSession* m_gameSession;

	// Syncing
	uint m_prevConnectionCount;

	//Game Play
	std::vector<Player*> m_playerList;
	std::vector<Ship*> m_ships;
	std::vector<Bullet*> m_bullets;
	std::vector<Asteroid*> m_asteroids;
	std::vector<Landmine*> m_landmines;
	std::vector<ShipSelect> m_shipOptions;
	std::vector<Explosion*> m_explosions;
	std::vector<Powerup*> m_powerups;
	SpriteSheet* m_shipSheet;
	SpriteSheet* m_bulletSheet;
	SpriteSheet* m_explosionSheet;
	Sampler* m_defaultSampler;
	Vector2 m_worldDimensions;
	float m_zoom;
	uint m_hostCameraValue;
	Texture2D* m_background;
	Mesh* m_backQuad;
	eDMState m_hostState;

	//Client Info
	Player* m_myPlayer;
};

extern Game* g_theGame;