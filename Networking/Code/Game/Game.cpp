#include "Game/Game.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GameCommons.hpp"
#include "Game/App.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Config.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/RenderMesh.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Network/TCPConnection.hpp"
#include "Engine/Network/TCPSession.hpp"
#include "Engine/Network/NetMessage.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/NetMessageDefinition.hpp"
#include "Engine/Network/RemoteCommandService.hpp"
#include "Engine/Network/NetObject.hpp"
#include "Engine/Math/Disc2D.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Bullet.hpp"
#include "Game/Player.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Ship.hpp"
#include "Game/Landmine.hpp"
#include "Game/Powerup.hpp"
#include "Engine/Render/SpriteSheet.hpp"
#include "Engine/Render/SpriteAnimation.hpp"
#include <sstream>
#include <iomanip>


Game* g_theGame = nullptr;
Config* g_config = nullptr;

//*******************************************************************
void ConsoleClear(void*)
{
	g_console->m_logHistory.clear();
	//g_console->m_cursorY = WORLD_HEIGHT;
}

//*******************************************************************
void ConsoleHelp(void* data)
{
	arguments args_list = *(arguments*)data;

	if (args_list.arg_list.empty())
	{
		for (auto iterate : g_console->m_commands)
		{
			std::string description = iterate.second.short_desc;

			std::string print = iterate.second.command + ": " + description;

			//command_cb commandInfo;
			//commandInfo.cmd_color = Rgba(0, 255, 0, 255);
			//commandInfo.command = print;
			//g_console->m_logHistory.push_back(commandInfo);
			g_console->ConsolePrintf(Rgba(0, 255, 0, 255), "%s", print.c_str());
			//g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight; //change to renderer
		}
	}
	else
	{
		for (std::string& command : args_list.arg_list)
		{
			std::string description = "";
			if(g_console->FindCommand(command))
				description = g_console->m_commands.find(command)->second.long_desc;

			std::string print = command + ": " + description;

			//command_cb commandInfo;
			//commandInfo.cmd_color = Rgba(0, 255, 0, 255);
			//commandInfo.command = print;
			//g_console->m_logHistory.push_back(commandInfo);
			g_console->ConsolePrintf(Rgba(0, 255, 0, 255), "%s", print.c_str());
			//g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight; //change to renderer
		}
	}
}

//*******************************************************************
void ConsoleQuit(void*)
{
	g_theGame->m_isQuitting = true;
	g_theApp->OnExitRequested();
}

Game::Game()
	:m_isQuitting(false)
	, m_screenShakeMagnitude(0.0f)
	, m_canDebug(false)
	, m_camera(nullptr)
	, m_defaultTex(nullptr)
	, m_fov(0.0f)
	, m_totalTimePassed(0.0f)
	, m_defaultShader(nullptr)
	, m_rcs(nullptr)
	, m_prevConnectionCount(0)
	, m_myPlayer(nullptr)
	, m_worldDimensions(3000.0f, 1500.0f)
	, m_zoom(1.0f)
	, m_hostCameraValue(0)
	, m_hostState(ASTEROIDS)
{	
	m_gameSession = new TCPSession();
	m_playerList.resize(m_gameSession->m_maxConnectionCount + 1);
	m_ships.resize(m_gameSession->m_maxConnectionCount + 1);
	SetUpGameMessages();
	NetSetup();
	m_camera = new Camera3D();
}

Game::~Game()
{
	m_playerList.clear();

	NetObjectCleanup();

	delete m_gameSession;
	m_gameSession = nullptr;

	delete m_camera;
	m_camera = nullptr;
}

void Game::Update(float deltaSeconds)
{
	m_timeConst.GAME_TIME += deltaSeconds;
	m_constBuffer->Update(g_simpleRenderer->m_context, &m_timeConst);

	if (g_IsTheGamePaused == true)
	{
		deltaSeconds *= 0.0f;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_F1))
	{
		m_canDebug = !m_canDebug;
	}

	if (!g_console->IsActive() || m_gameSession->AmIHost())
	{
		//Game Logic Goes Here

		if (m_gameSession->AmIHost() && g_theInputSystem->IsKeyDown('Q'))
		{
			m_zoom += 0.1f * deltaSeconds;
			m_zoom = ClampWithin(m_zoom, 1.5f, 0.4f);
		}

		if (m_gameSession->AmIHost() && g_theInputSystem->IsKeyDown('E'))
		{
			m_zoom -= 0.1f * deltaSeconds;
			m_zoom = ClampWithin(m_zoom, 1.5f, 0.4f);
		}

		if (m_gameSession->AmIHost() && g_theInputSystem->IsMouseWheelScrollingUp())
		{
			// go down dm list
			IncrementHostState();
		}

		if (m_gameSession->AmIHost() && g_theInputSystem->IsMouseWheelScrollingDown())
		{
			// go up dm list
			DecrementHostState();
		}

		if (!g_console->IsActive() && m_gameSession->AmIHost() && g_theInputSystem->WasKeyJustPressed('R'))
		{
			if (m_hostState == ASTEROIDS) 
			{
				Asteroid* asteroid = HostCreateAsteroid();
				m_asteroids.push_back(asteroid);
			}
			else if (m_hostState == LANDMINES)
			{
				Landmine* mine = HostCreateLandmine();
				m_landmines.push_back(mine);
			}
			else if (m_hostState == SPREAD_PICKUP)
			{
				Powerup* pwr = HostCreatePowerup(SPREAD);
				m_powerups.push_back(pwr);
			}
			else if (m_hostState == EXPLODE_PICKUP)
			{
				Powerup* pwr = HostCreatePowerup(EXPLODE);
				m_powerups.push_back(pwr);
			}
			else if (m_hostState == HEALTH_PICKUP)
			{
				Powerup* pwr = HostCreatePowerup(HEAL);
				m_powerups.push_back(pwr);
			}
		}

		NetObjectSystemStep();
		
		if (m_gameSession->AmIClient()) 
		{
			m_myPlayer->Update(deltaSeconds);

			for (uint index = 0; index < m_ships.size(); ++index)
			{
				if (!m_ships[index])
					continue;

				m_ships[index]->Update(deltaSeconds);
				CorrectPlayerOnWorldEdge(index);
			}

			for (uint index = 0; index < m_bullets.size(); ++index)
			{
				m_bullets[index]->Update(deltaSeconds);
			}

			UpdateAsteroids(deltaSeconds);
			UpdateLandmines(deltaSeconds);
			UpdatePowerups(deltaSeconds);

			SmoothShips(deltaSeconds);
			SmoothBullets();
			SmoothAsteroids(deltaSeconds);
			UpdateExplosions(deltaSeconds);
			UpdateClientCamera();
		}
		else if (m_gameSession->AmIHost())
		{
			HostManageRespawns();
			UpdatePlayers(deltaSeconds);
			UpdateCamera(deltaSeconds);
			UpdateBullets(deltaSeconds);
			UpdateAsteroids(deltaSeconds);
			UpdateShips(deltaSeconds);
			UpdateLandmines(deltaSeconds);
			UpdatePowerups(deltaSeconds);
			UpdateExplosions(deltaSeconds);
		}
	}

	if (m_rcs)
		m_rcs->Update();

}

void Game::CreateExplosion(Vector2 pos, float radius)
{
	Explosion* explosion = new Explosion();
	explosion->position = pos;
	explosion->radius = radius;
	explosion->anim = new SpriteAnimation(*m_explosionSheet, 0.75f, SPRITE_ANIM_MODE_PLAY_TO_END, 0, 15);

	m_explosions.push_back(explosion);
}

void Game::UpdateExplosions(float deltaSeconds)
{
	for (uint index = 0; index < m_explosions.size(); ++index)
	{
		if (m_explosions[index]->anim->IsFinished())
		{
			delete m_explosions[index]->anim;

			std::swap(m_explosions.back(), m_explosions[index]);
			delete m_explosions.back();
			m_explosions.pop_back();

			continue;
		}

		m_explosions[index]->anim->Update(deltaSeconds);
	}
}

void Game::RenderExplosions() const
{
	for (Explosion* explosion : m_explosions)
	{
		g_simpleRenderer->SetShaderProgram(m_defaultShader);
		g_simpleRenderer->SetSampler(m_defaultSampler);

		g_simpleRenderer->MakeModelMatrixIdentity();
		Matrix4 transform;
		transform.Translate(explosion->position);
		g_simpleRenderer->SetModelMatrix(transform);

		Texture2D* curr_tex = explosion->anim->GetTextureDX();

		AABB2D bound_box(Vector2(0.0f, 0.0f), explosion->radius, explosion->radius);
		AABB2D tex_box = explosion->anim->GetCurrentTexCoords();
		g_simpleRenderer->DrawTexturedSprite(bound_box, *curr_tex, tex_box, Rgba(255, 255, 255, 255), BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	}
}

void Game::SmoothShips(float deltaSeconds)
{
	if (!g_theGame->m_gameSession->AmIClient())
		return;

	Ship* my_ship = m_ships[m_myPlayer->m_connectionIndex];

	for (Ship* ship : m_ships)
	{
		if (!ship)
			continue;

		float max_distance_to_teleport;
		float acceptance_for_position;
		float acceptance_for_angle;
		float max_angle_to_teleport;

		if (ship == my_ship)
		{
			//be lenient
			max_distance_to_teleport = 100.0f;
			acceptance_for_position = 0.1f;
			acceptance_for_angle = 1.0f;
			max_angle_to_teleport = 90.0f;
		}
		else
		{
			//be more strict
			max_distance_to_teleport = 50.0f;
			acceptance_for_position = 0.01f;
			acceptance_for_angle = 0.5f;
			max_angle_to_teleport = 15.0f;
		}

		// Check if too far away
		float distance_to_position = (ship->m_deadReckon.position - ship->m_position).CalcLength();
		if (distance_to_position > max_distance_to_teleport)
		{
			ship->m_position = ship->m_deadReckon.position;
		}
		else
		{
			Vector2 ship_pos = ship->m_position;
			ship_pos.Normalize();
			Vector2 dead_rek_pos = ship->m_deadReckon.position;
			dead_rek_pos.Normalize();

			float interp_val = DotProduct(ship_pos, dead_rek_pos);

			if (IsEquivalent(interp_val, 1.0f, acceptance_for_position))
				break;

			ship->m_position = Interpolate(ship->m_position, ship->m_deadReckon.position, interp_val);
		}

		if (IsEquivalent(ship->m_deadReckon.orientation_in_degrees, ship->m_orientationInDegrees, acceptance_for_angle))
			continue;
		else if (!IsEquivalent(ship->m_deadReckon.orientation_in_degrees, ship->m_orientationInDegrees, max_angle_to_teleport))
		{
			ship->m_orientationInDegrees = ship->m_deadReckon.orientation_in_degrees;
			continue;
		}

		//float interp_angle = ship->m_orientationInDegrees / ship->m_deadReckon.orientation_in_degrees;
		ship->m_orientationInDegrees = LERP(ship->m_orientationInDegrees, ship->m_deadReckon.orientation_in_degrees, 0.5f * deltaSeconds);
	}
}

void Game::SmoothAsteroids(float deltaSeconds)
{
	if (!g_theGame->m_gameSession->AmIClient())
		return;

	for (Asteroid* asteroid : m_asteroids)
	{
		float max_distance_to_teleport = 100.0f;
		float acceptance_for_position = 0.1f;
		float acceptance_for_angle = 0.5f;
		float max_angle_to_teleport = 90.0f;


		// Check if too far away
		float distance_to_position = (asteroid->m_deadReckon.position - asteroid->m_position).CalcLength();
		if (distance_to_position > max_distance_to_teleport)
		{
			asteroid->m_position = asteroid->m_deadReckon.position;
		}
		else
		{
			Vector2 asteroid_pos = asteroid->m_position;
			asteroid_pos.Normalize();
			Vector2 dead_rek_pos = asteroid->m_deadReckon.position;
			dead_rek_pos.Normalize();

			float interp_val = DotProduct(asteroid_pos, dead_rek_pos);

			if (IsEquivalent(interp_val, 1.0f, acceptance_for_position))
				break;

			asteroid->m_position = Interpolate(asteroid->m_position, asteroid->m_deadReckon.position, interp_val);
		}

		if (IsEquivalent(asteroid->m_deadReckon.orientation_in_degrees, asteroid->m_orientationInDegrees, acceptance_for_angle))
			continue;
		else if (!IsEquivalent(asteroid->m_deadReckon.orientation_in_degrees, asteroid->m_orientationInDegrees, max_angle_to_teleport))
		{
			asteroid->m_orientationInDegrees = asteroid->m_deadReckon.orientation_in_degrees;
			continue;
		}

		//float interp_angle = ship->m_orientationInDegrees / ship->m_deadReckon.orientation_in_degrees;
		asteroid->m_orientationInDegrees = LERP(asteroid->m_orientationInDegrees, asteroid->m_deadReckon.orientation_in_degrees, 0.5f * deltaSeconds);
	}
}

void Game::SmoothBullets()
{
	if (!g_theGame->m_gameSession->AmIClient())
		return;

	for (Bullet* bullet : m_bullets)
	{
		if (!bullet)
			continue;

		// Check if too far away
		float distance_to_position = (bullet->m_deadReckon.position - bullet->m_position).CalcLength();
		if (distance_to_position > 200.0f)
		{
			bullet->m_position = bullet->m_deadReckon.position;
			continue;
		}

		Vector2 bullet_pos = bullet->m_position;
		bullet_pos.Normalize();
		Vector2 dead_rek_pos = bullet->m_deadReckon.position;
		dead_rek_pos.Normalize();

		float interp_val = DotProduct(bullet_pos, dead_rek_pos);

		if (IsEquivalent(interp_val, 1.0f, 0.01f))
			continue;

		bullet->m_position = Interpolate(bullet->m_position, bullet->m_deadReckon.position, interp_val);
	}
}

void Game::HostManageRespawns()
{
	for (uint index = 0; index < m_playerList.size(); ++index)
	{
		if (!m_playerList[index])
			continue;

		if (m_ships[index])
			continue;

		if (m_playerList[index]->m_timeDead >= 2.0f)
		{
			m_playerList[index]->m_timeDead = 0.0f;
			m_ships[index] = HostCreateShipToReplicate((uint8_t)index);
			m_playerList[index]->m_ship = m_ships[index];
		}
	}
}

void Game::UpdateShips(float deltaSeconds)
{
	for (uint index = 0; index < m_ships.size(); ++index)
	{
		if (!m_ships[index])
			continue;

		if (m_ships[index]->m_health <= 0)
		{
			HostDestroyShipThatsReplicated(m_ships[index]);
			m_ships[index] = nullptr;
			m_playerList[index]->m_ship = nullptr;
			continue;
		}

		Ship* current = m_ships[index];
		current->Update(deltaSeconds);
	}
}

void Game::UpdateBullets(float deltaSeconds)
{
	for (uint index = 0; index < m_bullets.size(); ++index)
	{
		Bullet* current = m_bullets[index];

		if (current->m_ownerID != INVALID_CONNECTION_INDEX && !m_playerList[current->m_ownerID])
			m_bullets[index]->m_ownerID = INVALID_CONNECTION_INDEX;

		current->Update(deltaSeconds);
		
		if (CheckForBulletCollision(current) || IsBulletOutsideWorld(current) || current->m_timeAlive >= 3.0f)
		{
			HostDestroyBullet(index);
		}
	}
}

void Game::UpdateAsteroids(float deltaSeconds)
{
	for (uint index = 0; index < m_asteroids.size(); ++index)
	{
		Asteroid* asteroid = m_asteroids[index];

		if (asteroid->m_health <= 0 && m_gameSession->AmIHost())
		{
			HostDestroyAsteroid(index);
			continue;
		}

		asteroid->Update(deltaSeconds);

		CheckForOverlaps(asteroid, index);
	}
}

void Game::UpdateLandmines(float deltaSeconds)
{
	for (uint index = 0; index < m_landmines.size(); ++index)
	{
		m_landmines[index]->Update(deltaSeconds);
		LandmineCollideWithShip(index);
	}
}

void Game::UpdatePowerups(float deltaSeconds)
{
	for (uint index = 0; index < m_powerups.size(); ++index)
	{
		m_powerups[index]->Update(deltaSeconds);
		CheckPowerupCollisions(index);
	}
}

void Game::CheckPowerupCollisions(uint index)
{
	for (Ship* ship : m_ships)
	{
		if (!ship)
			continue;

		Disc2D ship_disc(ship->m_position, ship->m_radius);
		Disc2D powerup_disc(m_powerups[index]->m_position, m_powerups[index]->m_radius);

		if (!DoDiscsOverlap(ship_disc, powerup_disc))
			continue;

		ePowerUpAbility type = m_powerups[index]->m_type;

		if (type == EXPLODE)
		{
			ship->m_fireState = EXPLOSIVE;
		}
		else if (type == SPREAD)
		{
			ship->m_fireState = SPREAD_SHOT;
		}
		else if (type == HEAL)
		{
			if (ship->m_health == 10)
				continue;

			ship->m_health += 3;
			ship->m_health = ClampWithin(ship->m_health, 10, 0);
		}

		if (m_gameSession->AmIHost())
			HostDestroyPowerup(index);
	}
}

void Game::CheckForOverlaps(Asteroid* asteroid, uint index)
{
	CollideWithShip(asteroid);
	CollideWithAsteroids(index);
	BounceOffWorldEdge(asteroid);
}

void Game::BounceOffWorldEdge(Asteroid* asteroid)
{
	Vector2 asteroid_pos = asteroid->m_position;
	float asteroid_radius = asteroid->m_radius;

	if (asteroid_pos.x - asteroid_radius <= -(m_worldDimensions.x * 0.5f))
	{
		//Left
		float left_most = asteroid_pos.x - asteroid_radius;
		asteroid->m_position.x += -(m_worldDimensions.x * 0.5f) - left_most;
		asteroid->m_velocity.x *= -1.0f;
	}

	if (asteroid_pos.x + asteroid_radius >= (m_worldDimensions.x * 0.5f))
	{
		//Right
		float right_most = asteroid_pos.x + asteroid_radius;
		asteroid->m_position.x += (m_worldDimensions.x * 0.5f) - right_most;
		asteroid->m_velocity.x *= -1.0f;
	}

	if (asteroid_pos.y + asteroid_radius >= (m_worldDimensions.y * 0.5f))
	{
		//Top
		float top_most = asteroid_pos.y + asteroid_radius;
		asteroid->m_position.y += (m_worldDimensions.y * 0.5f) - top_most;
		asteroid->m_velocity.y *= -1.0f;
	}

	if (asteroid_pos.y - asteroid_radius <= -(m_worldDimensions.y * 0.5f))
	{
		//Bottom
		float bttm_most = asteroid_pos.y - asteroid_radius;
		asteroid->m_position.y += -(m_worldDimensions.y * 0.5f) - bttm_most;
		asteroid->m_velocity.y *= -1.0f;
	}
}

void Game::CollideWithAsteroids(uint index)
{
	Asteroid* curr_aster = m_asteroids[index];

	for (uint loop_idx = index + 1; loop_idx < m_asteroids.size(); ++loop_idx)
	{
		Disc2D collide_astr_disc(m_asteroids[loop_idx]->m_position, m_asteroids[loop_idx]->m_radius);
		Disc2D current_astr_disc(curr_aster->m_position, curr_aster->m_radius);

		if (!DoDiscsOverlap(collide_astr_disc, current_astr_disc))
			continue;

		Vector2 velocity_astr = curr_aster->m_velocity;
		Vector2 velocity_ship = m_asteroids[loop_idx]->m_velocity;
		float elasticity = 0.8f * 0.8f;

		BounceBothDiscs2D(current_astr_disc, collide_astr_disc, velocity_astr, velocity_ship, elasticity);

		curr_aster->m_position = current_astr_disc.m_center;
		curr_aster->m_velocity = velocity_astr;
		m_asteroids[loop_idx]->m_position = collide_astr_disc.m_center;
		m_asteroids[loop_idx]->m_velocity = velocity_ship;
	}

	for (uint loop_index = 0; loop_index < m_landmines.size(); ++loop_index)
	{
		Disc2D landmine_disc(m_landmines[loop_index]->m_position, m_landmines[loop_index]->m_radius);
		Disc2D astr_disc(curr_aster->m_position, curr_aster->m_radius);

		if (!DoDiscsOverlap(landmine_disc, astr_disc))
			continue;

		curr_aster->m_health -= 3;

		Vector2 velocity_astr = curr_aster->m_velocity;

		Vector2 temp_vel = curr_aster->m_velocity;
		temp_vel.Normalize();
		Vector2 velocity_mine = -1.0f * m_landmines[loop_index]->m_force * temp_vel;

		float elasticity = 0.8f * 0.8f;

		BounceBothDiscs2D(astr_disc, landmine_disc, velocity_astr, velocity_mine, elasticity);

		curr_aster->m_position = astr_disc.m_center;
		curr_aster->m_velocity = velocity_astr;

		HostDestroyLandmine(loop_index);
	}
}

void Game::CollideWithShip(Asteroid* asteroid)
{
	for (Ship* ship : m_ships)
	{
		if (!ship)
			continue;

		Disc2D ship_disc(ship->m_position, ship->m_radius);
		Disc2D asteroid_disc(asteroid->m_position, asteroid->m_radius);

		if (!DoDiscsOverlap(ship_disc, asteroid_disc))
			continue;

		ship->m_health -= 1;

		Vector2 velocity_astr = asteroid->m_velocity;
		Vector2 velocity_ship = ship->m_velocity;
		float elasticity = 0.8f * 0.8f;

		BounceBothDiscs2D(asteroid_disc, ship_disc, velocity_astr, velocity_ship, elasticity);

		asteroid->m_position = asteroid_disc.m_center;
		asteroid->m_velocity = velocity_astr;
		ship->m_position = ship_disc.m_center;
		ship->m_velocity = velocity_ship;
	}
}

void Game::LandmineCollideWithShip(uint index)
{
	for (Ship* ship : m_ships)
	{
		if (!ship)
			continue;

		Disc2D ship_disc(ship->m_position, ship->m_radius);
		Disc2D landmine_disc(m_landmines[index]->m_position, m_landmines[index]->m_radius);

		if (!DoDiscsOverlap(ship_disc, landmine_disc))
			continue;

		ship->m_health -= 3;

		Vector2 temp_vel = ship->m_velocity;
		temp_vel.Normalize();
		Vector2 velocity_mine = -1.0f * m_landmines[index]->m_force * temp_vel;
		Vector2 velocity_ship = ship->m_velocity;
		float elasticity = 0.8f * 0.8f;

		BounceBothDiscs2D(landmine_disc, ship_disc, velocity_mine, velocity_ship, elasticity);

		if(m_gameSession->AmIHost())
			HostDestroyLandmine(index);

		ship->m_position = ship_disc.m_center;
		ship->m_velocity = velocity_ship;
	}
}

bool Game::CheckForBulletCollision(Bullet* bullet)
{
	for (uint index = 0; index < m_playerList.size(); ++index)
	{
		if (!m_playerList[index])
			continue;

		if (!m_playerList[index]->m_ship)
			continue;

		if (!m_playerList[index]->m_hasLockedInShip)
			continue;

		Disc2D player_disc(m_playerList[index]->m_ship->m_position, m_playerList[index]->m_ship->m_radius);
		Disc2D bullet_disc(bullet->m_position, bullet->m_radius);

		bool overlap = DoDiscsOverlap(player_disc, bullet_disc);

		if(overlap)
		{
			if(m_playerList[index]->m_ship && m_playerList[index]->m_connectionIndex != bullet->m_ownerID)
			{
				m_playerList[index]->m_ship->m_health -= bullet->m_damage;
				return true;
			}
			return false;
		}
	}

	for (uint index = 0; index < m_asteroids.size(); ++index)
	{
		Disc2D asteroid_disc(m_asteroids[index]->m_position, m_asteroids[index]->m_radius);
		Disc2D bullet_disc(bullet->m_position, bullet->m_radius);

		bool overlap = DoDiscsOverlap(asteroid_disc, bullet_disc);

		if (overlap)
		{
			m_asteroids[index]->m_health -= bullet->m_damage;
			return true;
		}
	}

	for (uint index = 0; index < m_landmines.size(); ++index)
	{
		Disc2D landmine_disc(m_landmines[index]->m_position, m_landmines[index]->m_radius);
		Disc2D bullet_disc(bullet->m_position, bullet->m_radius);

		bool overlap = DoDiscsOverlap(landmine_disc, bullet_disc);

		if (overlap)
		{
			HostDestroyLandmine(index);
			return true;
		}
	}

	return false;
}

bool Game::IsBulletOutsideWorld(Bullet* bullet)
{
	Vector2 bullet_pos = bullet->m_position;
	float bullet_radius = bullet->m_radius;

	if (bullet_pos.x + bullet_radius <= -(m_worldDimensions.x * 0.5f))
	{
		return true;
	}
	else if (bullet_pos.x - bullet_radius >= (m_worldDimensions.x * 0.5f))
	{
		return true;
	}
	else if (bullet_pos.y - bullet_radius >= (m_worldDimensions.y * 0.5f))
	{
		return true;
	}
	else if (bullet_pos.y + bullet_radius <= -(m_worldDimensions.y * 0.5f))
	{
		return true;
	}

	return false;
}

void Game::UpdateCamera(float deltaSeconds)
{
	UpdateHostCamera(deltaSeconds);
}

void Game::UpdateClientCamera()
{
	if (!m_gameSession->AmIClient() || !m_myPlayer->m_ship)
		return;

	AABB2D world_box(Vector2(0.0f, 0.0f), m_worldDimensions.x * 0.5f, m_worldDimensions.y * 0.5f);
	AABB2D cam_bounds_base_on_player(Vector2(m_myPlayer->m_ship->m_position.x, m_myPlayer->m_ship->m_position.y), WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f);

	if (world_box.mins.x < cam_bounds_base_on_player.mins.x && world_box.mins.y < cam_bounds_base_on_player.mins.y
		&& world_box.maxs.x > cam_bounds_base_on_player.maxs.x && world_box.maxs.y > cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position = Vector3(m_myPlayer->m_ship->m_position.x, m_myPlayer->m_ship->m_position.y, 0.0f);
	}


	//Region Right
	if (cam_bounds_base_on_player.mins.x < world_box.mins.x)
	{
		m_camera->m_position.x = world_box.mins.x + (WORLD_WIDTH * 0.5f);
		m_camera->m_position.y = m_myPlayer->m_ship->m_position.y;
	}
	//Region Left
	if (world_box.maxs.x < cam_bounds_base_on_player.maxs.x)
	{
		m_camera->m_position.x = world_box.maxs.x - (WORLD_WIDTH * 0.5f);
		m_camera->m_position.y = m_myPlayer->m_ship->m_position.y;
	}
	//Region Bottom
	if (cam_bounds_base_on_player.mins.y < world_box.mins.y)
	{
		m_camera->m_position.y = world_box.mins.y + (WORLD_HEIGHT * 0.5f);
		m_camera->m_position.x = m_myPlayer->m_ship->m_position.x;
	}
	//Region Top
	if (world_box.maxs.y < cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position.y = world_box.maxs.y - (WORLD_HEIGHT * 0.5f);
		m_camera->m_position.x = m_myPlayer->m_ship->m_position.x;
	}

	//Region Top Right
	if (cam_bounds_base_on_player.mins.x < world_box.mins.x && world_box.maxs.y < cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position = Vector3(world_box.mins.x + (WORLD_WIDTH * 0.5f), world_box.maxs.y - (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region Bottom Right
	if (cam_bounds_base_on_player.mins.x < world_box.mins.x && cam_bounds_base_on_player.mins.y < world_box.mins.y)
	{
		m_camera->m_position = Vector3(world_box.mins.x + (WORLD_WIDTH * 0.5f), world_box.mins.y + (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region Bottom Left
	if (world_box.maxs.x < cam_bounds_base_on_player.maxs.x && cam_bounds_base_on_player.mins.y < world_box.mins.y)
	{
		m_camera->m_position = Vector3(world_box.maxs.x - (WORLD_WIDTH * 0.5f), world_box.mins.y + (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region Top Left
	if (world_box.maxs.x < cam_bounds_base_on_player.maxs.x && world_box.maxs.y < cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position = Vector3(world_box.maxs.x - (WORLD_WIDTH * 0.5f), world_box.maxs.y - (WORLD_HEIGHT * 0.5f), 0.0f);
	}
}

void Game::UpdateHostCamera(float deltaSeconds)
{
	if (!m_gameSession->AmIHost())
		return;

	if (g_theInputSystem->IsKeyDown('W') && m_hostCameraValue == 0)
	{
		m_camera->m_position.y += 200.0f * deltaSeconds;
	}

	if (g_theInputSystem->IsKeyDown('S') && m_hostCameraValue == 0)
	{
		m_camera->m_position.y -= 200.0f * deltaSeconds;
	}

	if (g_theInputSystem->IsKeyDown('A') && m_hostCameraValue == 0)
	{
		m_camera->m_position.x -= 200.0f * deltaSeconds;
	}

	if (g_theInputSystem->IsKeyDown('D') && m_hostCameraValue == 0)
	{
		m_camera->m_position.x += 200.0f * deltaSeconds;
	}

	Vector2 begin_pos;
	if (m_hostCameraValue != 0)
	{
		if(m_ships[m_hostCameraValue])
			begin_pos = m_ships[m_hostCameraValue]->m_position;
	}
	else
	{
		begin_pos = Vector2(m_camera->m_position.x, m_camera->m_position.y);
	}

	AABB2D world_box(Vector2(0.0f, 0.0f), m_worldDimensions.x * 0.5f, m_worldDimensions.y * 0.5f);
	AABB2D cam_bounds_base_on_player(begin_pos, WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f);

	if (world_box.mins.x < cam_bounds_base_on_player.mins.x && world_box.mins.y < cam_bounds_base_on_player.mins.y
		&& world_box.maxs.x > cam_bounds_base_on_player.maxs.x && world_box.maxs.y > cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position = Vector3(begin_pos.x, begin_pos.y, 0.0f);
	}
	
	//Region I
	if (cam_bounds_base_on_player.mins.x < world_box.mins.x && world_box.maxs.y < cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position = Vector3(world_box.mins.x + (WORLD_WIDTH * 0.5f), world_box.maxs.y - (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region II
	if (cam_bounds_base_on_player.mins.x < world_box.mins.x && cam_bounds_base_on_player.mins.y < world_box.mins.y)
	{
		m_camera->m_position = Vector3(world_box.mins.x + (WORLD_WIDTH * 0.5f), world_box.mins.y + (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region III
	if (world_box.maxs.x < cam_bounds_base_on_player.maxs.x && cam_bounds_base_on_player.mins.y < world_box.mins.y)
	{
		m_camera->m_position = Vector3(world_box.maxs.x - (WORLD_WIDTH * 0.5f), world_box.mins.y + (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region IV
	if (world_box.maxs.x < cam_bounds_base_on_player.maxs.x && world_box.maxs.y < cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position = Vector3(world_box.maxs.x - (WORLD_WIDTH * 0.5f), world_box.maxs.y - (WORLD_HEIGHT * 0.5f), 0.0f);
	}
	//Region V
	if (cam_bounds_base_on_player.mins.x < world_box.mins.x)
	{
		m_camera->m_position.x = world_box.mins.x + (WORLD_WIDTH * 0.5f);
	}
	//Region VI
	if (world_box.maxs.x < cam_bounds_base_on_player.maxs.x)
	{
		m_camera->m_position.x = world_box.maxs.x - (WORLD_WIDTH * 0.5f);
	}
	//Region VII
	if (cam_bounds_base_on_player.mins.y < world_box.mins.y)
	{
		m_camera->m_position.y = world_box.mins.y + (WORLD_HEIGHT * 0.5f);
	}
	//Region VIII
	if (world_box.maxs.y < cam_bounds_base_on_player.maxs.y)
	{
		m_camera->m_position.y = world_box.maxs.y - (WORLD_HEIGHT * 0.5f);
	}
}

void Game::UpdatePlayers(float deltaSeconds)
{
	for (uint index = 0; index < m_playerList.size(); ++index)
	{
		if (!m_playerList[index])
			continue;

		uint8_t conn_index = m_playerList[index]->m_connectionIndex;
		if (m_gameSession->m_connections[conn_index] == nullptr)
		{
			HostDestroyShipThatsReplicated(m_ships[index]);
			m_ships[index] = nullptr;

			delete m_playerList[index];
			m_playerList[index] = nullptr;

			continue;
		}

		m_playerList[index]->Update(deltaSeconds);

		CorrectPlayerOnWorldEdge(index);
	}
}

void Game::CorrectPlayerOnWorldEdge(uint player_index)
{
	if (!m_playerList[player_index]->m_ship)
		return;

	Vector2 player_pos = m_playerList[player_index]->m_ship->m_position;
	float player_radius = m_playerList[player_index]->m_ship->m_radius;

	if (player_pos.x - player_radius <= -(m_worldDimensions.x * 0.5f))
	{
		//Left
		float left_most = player_pos.x - player_radius;
		m_playerList[player_index]->m_ship->m_position.x += -(m_worldDimensions.x * 0.5f) - left_most;
		m_playerList[player_index]->m_ship->m_velocity.x = 0.0f;
	}
	
	if (player_pos.x + player_radius >= (m_worldDimensions.x * 0.5f))
	{
		//Right
		float right_most = player_pos.x + player_radius;
		m_playerList[player_index]->m_ship->m_position.x += (m_worldDimensions.x * 0.5f) - right_most;
		m_playerList[player_index]->m_ship->m_velocity.x = 0.0f;
	}
	
	if (player_pos.y + player_radius >= (m_worldDimensions.y * 0.5f))
	{
		//Top
		float top_most = player_pos.y + player_radius;
		m_playerList[player_index]->m_ship->m_position.y += (m_worldDimensions.y * 0.5f) - top_most;
		m_playerList[player_index]->m_ship->m_velocity.y = 0.0f;
	}
	
	if (player_pos.y - player_radius <= -(m_worldDimensions.y * 0.5f))
	{
		//Bottom
		float bttm_most = player_pos.y - player_radius;
		m_playerList[player_index]->m_ship->m_position.y += -(m_worldDimensions.y * 0.5f) - bttm_most;
		m_playerList[player_index]->m_ship->m_velocity.y = 0.0f;
	}
}

void Game::Render() const
{
	Start2DDraw();

	g_simpleRenderer->SetShaderProgram(m_defaultShader);
	g_simpleRenderer->SetTexture(m_background);
	g_simpleRenderer->DrawMesh(*m_backQuad);


	for (uint index = 0; index < m_ships.size(); ++index)
	{
		if (!m_ships[index])
			continue;

		if (m_gameSession->AmIClient()) 
		{
			if (m_playerList[index] != m_myPlayer && !m_playerList[index]->m_hasLockedInShip)
				continue;
		}

		m_ships[index]->Render();
	}

	for (Bullet* bullet : m_bullets)
	{
		bullet->Render();
	}

	for (Asteroid* asteroid : m_asteroids)
	{
		asteroid->Render();
	}

	for (Landmine* mine : m_landmines)
	{
		mine->Render();
	}

	for (Powerup* pwr : m_powerups)
	{
		pwr->Render();
	}

	RenderExplosions();

	if (m_rcs && g_console->IsActive())
		m_rcs->Render();

	if (m_gameSession && g_theApp->m_currentState == GAME)
		RenderGameSessionInfo();

}

void Game::Start2DDraw() const
{
	g_simpleRenderer->SetRenderTarget(nullptr, nullptr);
	g_simpleRenderer->ClearColor(Rgba(0, 0, 0));
	IntVector2 dimensions = g_simpleRenderer->m_output->m_window->GetClientSize();
	g_simpleRenderer->SetViewport(0, 0, (unsigned int)dimensions.x, (unsigned int)dimensions.y);
	g_simpleRenderer->MakeModelMatrixIdentity();
	g_simpleRenderer->SetViewMatrix(Vector3(0.0f, 0.0f, 0.0f), m_camera->m_position);
	g_simpleRenderer->SetOrthoProjection(Vector2(WORLD_WIDTH * -0.5f, WORLD_HEIGHT * -0.5f) * m_zoom, Vector2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f) * m_zoom);
	g_simpleRenderer->SetConstantBuffer(1, m_constBuffer);
}

void Game::DrawConsole() const
{
	g_console->Render();
}

void Game::Draw2DGridAndAxis() const
{
	g_simpleRenderer->SetShaderProgram(m_defaultShader);
	g_simpleRenderer->SetTexture(m_defaultTex);
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	DrawGrid2D();
	DrawAxis2D();
	g_simpleRenderer->DisableBlend();
}

void Game::DrawGrid2D() const
{
	for (float distance = 0.0f; distance < m_worldDimensions.y * 0.5f; distance += 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(-m_worldDimensions.x * 0.5f, distance, 0.0f), Vector3(m_worldDimensions.x * 0.5f, distance, 0.0f), Rgba(255, 255, 255, 128));
	}
	
	for (float distance = -50.0f; distance > -m_worldDimensions.y * 0.5f; distance -= 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(-m_worldDimensions.x * 0.5f, distance, 0.0f), Vector3(m_worldDimensions.x * 0.5f, distance, 0.0f), Rgba(255, 255, 255, 128));
	}

	for (float distance = 25.0f; distance < m_worldDimensions.y * 0.5f; distance += 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(-m_worldDimensions.x * 0.5f, distance, 0.0f), Vector3(m_worldDimensions.x * 0.5f, distance, 0.0f), Rgba(255, 255, 0, 64));
	}

	for (float distance = -25.0f; distance > -m_worldDimensions.y * 0.5f; distance -= 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(-m_worldDimensions.x * 0.5f, distance, 0.0f), Vector3(m_worldDimensions.x * 0.5f, distance, 0.0f), Rgba(255, 255, 0, 64));
	}
	
	for (float distance = 0.0f; distance < m_worldDimensions.x * 0.5f; distance += 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -m_worldDimensions.y * 0.5f, 0.0f), Vector3(distance, m_worldDimensions.y * 0.5f, 0.0f), Rgba(255, 255, 255, 128));
	}
	
	for (float distance = -50.0f; distance > -m_worldDimensions.x * 0.5f; distance -= 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -m_worldDimensions.y * 0.5f, 0.0f), Vector3(distance, m_worldDimensions.y * 0.5f, 0.0f), Rgba(255, 255, 255, 128));
	}

	for (float distance = 25.0f; distance < m_worldDimensions.x * 0.5f; distance += 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -m_worldDimensions.y * 0.5f, 0.0f), Vector3(distance, m_worldDimensions.y * 0.5f, 0.0f), Rgba(255, 255, 0, 64));
	}

	for (float distance = -25.0f; distance > -m_worldDimensions.x * 0.5f; distance -= 50.0f)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -m_worldDimensions.y * 0.5f, 0.0f), Vector3(distance, m_worldDimensions.y * 0.5f, 0.0f), Rgba(255, 255, 0, 64));
	}
}

void Game::DrawAxis2D() const
{
	g_simpleRenderer->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(100.0f, 0.0f, 0.0f), Rgba(255, 0, 0, 255));
	g_simpleRenderer->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 100.0f, 0.0f), Rgba(0, 255, 0, 255));
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
	g_theInputSystem->OnKeyDown(keyThatWasJustPressed);
}

void JoinRCS(void* data)
{
	arguments args = *(arguments*)data;
	net_address_t address = StringToNetAddress(args.arg_list[0]);

	RemoteCommandService::GetInstance()->Join(address);
}

void RemoteCommand(void* data)
{
	arguments args = *(arguments*)data;

	std::vector<std::string> string_vec = g_console->m_commandLog.back();
	std::string string;
	for (uint index = 1; index < string_vec.size(); ++index)
	{
		string.append(string_vec[index].c_str());

		if (index != string_vec.size() - 1)
			string.append(" ");
	}

	RemoteCommandService* rcs = RemoteCommandService::GetInstance();
	rcs->SendCommandToOthers(string.c_str());
}

void TargetRemoteCommand(void* data)
{
	arguments args = *(arguments*)data;
	uint conn_index = std::stoi(args.arg_list[0]);
	std::vector<std::string> string_vec = g_console->m_commandLog.back();
	std::string string;
	for (uint index = 2; index < string_vec.size(); ++index)
	{
		string.append(string_vec[index].c_str());

		if (index != string_vec.size() - 1)
			string.append(" ");
	}

	NetMessage message(RCS_COMMAND);
	message.WriteString(string.c_str());

	RemoteCommandService* rcs = RemoteCommandService::GetInstance();
	rcs->m_session->m_connections[conn_index]->Send(&message);
}

void SetEcho(void* data)
{
	arguments args = *(arguments*)data;
	RemoteCommandService* rcs = RemoteCommandService::GetInstance();

	std::string input = args.arg_list[0];
	if (input == "true" || input == "True")
		rcs->m_canEcho = true;
	else if (input == "false" || input == "False")
		rcs->m_canEcho = false;
}

void LaunchProgram(void* data)
{
	if (!g_theGame->m_rcs->m_session->AmIHost())
		return;

	arguments args = *(arguments*)data;

	WCHAR buffer[254];
	GetModuleFileName(NULL, buffer, 254);

	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;

	CreateProcess(buffer, NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
}

void Game::Initialize()
{
	m_timeConst.GAME_TIME = 0.0f;
	m_constBuffer = new ConstantBuffer(g_simpleRenderer->m_device, &m_timeConst, sizeof(m_timeConst));

	m_rcs = new RemoteCommandService();

	InitializeConsole();
	
	m_defaultTex = CreateOrGetTexture2D("Default", g_simpleRenderer, "Data/Images/blank.png");
	m_shadowBox = CreateOrGetShaderProgram("Console Back", "Data/HLSL/shadow_box.hlsl", g_simpleRenderer);
	m_fontShader = CreateOrGetShaderProgram("Font", "Data/HLSL/font_shader.hlsl", g_simpleRenderer);
	m_skybox = CreateOrGetShaderProgram("Sky Box", "Data/HLSL/skybox.hlsl", g_simpleRenderer);
	m_defaultShader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);

	m_background = CreateOrGetTexture2D("Background", g_simpleRenderer, "Data/Images/space_background.png");
	m_backQuad = new Mesh();
	m_backQuad->CreateOneSidedQuad(Vector3(0.0f, 0.0f, 0.0), Vector3(m_worldDimensions.x, m_worldDimensions.y, 0.0f));

	m_shipSheet = new SpriteSheet("Data/Images/ship_sprites.png", 4, 2, g_simpleRenderer);
	m_bulletSheet = new SpriteSheet("Data/Images/bullet_sprites.png", 4, 2, g_simpleRenderer);
	m_explosionSheet = new SpriteSheet("Data/Images/Explosion_4x4.png", 4, 4, g_simpleRenderer);
	m_shipOptions.resize(8);

	for (uint index = 0; index < 8; ++index)
	{
		m_shipOptions[index].is_selected = false;
	}
	
	m_defaultSampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);


	g_config->ConfigGetFloat(m_fov,"field_of_view");
}

void GetHost(void* data)
{
	arguments args = *(arguments*)data;

	std::string string = NetAddressToString(RemoteCommandService::GetInstance()->m_session->m_hostConnection->m_address);
	g_console->ConsolePrintf(Rgba(255, 255, 255, 255), "%s", string.c_str());
}

void SetDimensions(void* data)
{
	if (g_theGame->m_gameSession->IsRunning())
	{
		g_console->ConsolePrintf(Rgba(255, 0, 0, 255), "Connection established, cannot set dimensions!");
		return;
	}

	arguments args = *(arguments*)data;

	float width;
	float height;
	if (args.arg_list.empty() || args.arg_list.size() < 2)
	{
		width = g_theGame->m_worldDimensions.x;
		height = g_theGame->m_worldDimensions.y;
	}
	else
	{
		width = std::stof(args.arg_list[0]);
		height = std::stof(args.arg_list[1]);
	}
	
	g_theGame->m_worldDimensions = Vector2(width, height);


	g_theGame->m_backQuad->m_vertices.clear();
	g_theGame->m_backQuad->m_indices.clear();
	g_theGame->m_backQuad->CreateOneSidedQuad(Vector3(0.0f, 0.0f, 0.0), Vector3(g_theGame->m_worldDimensions.x, g_theGame->m_worldDimensions.y, 0.0f));
}

void ResetName(void* data)
{
	if (g_theGame->m_gameSession->AmIHost())
	{
		g_console->ConsolePrintf(Rgba(255, 0, 0, 255), "A host has no name!");
		return;
	}

	arguments args = *(arguments*)data;
	std::string new_name = args.arg_list[0];

	uint8_t index_to_type = g_theGame->m_myPlayer->m_connectionIndex;
	g_theGame->m_playerList[index_to_type]->m_name = new_name;
	g_theGame->m_myPlayer = g_theGame->m_playerList[index_to_type];

	NetMessage sync(SYNC);
	sync.m_sender = g_theGame->m_gameSession->m_myConnection;
	sync.write(index_to_type);
	sync.WriteString(g_theGame->m_myPlayer->m_name.c_str());
	sync.write_bytes(&g_theGame->m_myPlayer->m_input.thrust, sizeof(float));
	sync.write_bytes(&g_theGame->m_myPlayer->m_input.steering_angle, sizeof(float));
	uint16_t frst_invalid_id = NetObjectGetUnusedID();
	sync.write_bytes(&frst_invalid_id, sizeof(uint16_t));
	sync.write_bytes(&g_theGame->m_myPlayer->m_shipSelectionIdx, sizeof(uint8_t));
	sync.write_bytes(&g_theGame->m_myPlayer->m_hasLockedInShip, sizeof(bool));
	g_theGame->m_gameSession->SendMessageToOthers(sync);
}

void FollowShip(void* data)
{
	arguments args = *(arguments*)data;
	uint index = (uint)std::stoul(args.arg_list[0]);

	if (g_theGame->m_playerList.size() < index || !g_theGame->m_playerList[index])
		index = 0;

	g_theGame->m_hostCameraValue = index;
}

void SetNetUpdateRate(void* data)
{
	if (!g_theGame->m_gameSession->AmIHost())
		return;

	arguments args = *(arguments*)data;

	float hertz;
	if (args.arg_list.empty())
		hertz = 60.0f;
	else
		hertz = std::stof(args.arg_list[0]);

	SetNetObjectRefreshRate(hertz);
}

void Game::InitializeConsole()
{
	m_font = CreateOrGetKerningFont("Data/Fonts/trebuchetMS32.fnt");
	g_console = new CommandSystem(m_font);
	g_console->RegisterCommand("clear", ConsoleClear, Rgba(255, 255, 255, 255), "Clears Command History.", "Look its here to clear the window.");
	g_console->RegisterCommand("help", ConsoleHelp, Rgba(255, 255, 255, 255), "Gives descriptions for either all aommands, or specific ones.", "Cheeky little developer aren't ya?");
	g_console->RegisterCommand("quit", ConsoleQuit, Rgba(255, 255, 255, 255), "Will close the window.", "This is what I want to do.");
	g_console->RegisterCommand("rcs_join", JoinRCS, Rgba(255, 255, 255, 255), "Joins Provided address and IP.", " ");
	g_console->RegisterCommand("rca", RemoteCommand, Rgba(255, 255, 255, 255), "Sends Command out through established session.", " ");
	g_console->RegisterCommand("get_host", GetHost, Rgba(255, 255, 255, 255), "Display Host Address for RCS.", " ");
	g_console->RegisterCommand("rc", TargetRemoteCommand, Rgba(255, 255, 255, 255), "Takes an index then command to send.", " ");
	g_console->RegisterCommand("set_echo", SetEcho, Rgba(255, 255, 255, 255), "true or false setting.", " ");
	g_console->RegisterCommand("launch", LaunchProgram, Rgba(255, 255, 255, 255), "Opens another EXE.", " ");
	g_console->RegisterCommand("set_dimensions", SetDimensions, Rgba(255, 255, 255, 255), "Requires a float width then height.", " ");
	g_console->RegisterCommand("reset_name", ResetName, Rgba(255, 255, 255, 255), "Change your name.", " ");
	g_console->RegisterCommand("follow", FollowShip, Rgba(255, 255, 255, 255), "Given an index will follow that player, 0 to reset.", " ");
	g_console->RegisterCommand("net_rate", SetNetUpdateRate, Rgba(255, 255, 255, 255), "Host Will Set Net Refresh Rate to given hertz value.", " ");

	g_console->SetFontShader("Font", "Data/HLSL/font_shader.hlsl");
	g_console->SetBackDropShader("Console Back", "Data/HLSL/shadow_box.hlsl"); 
	g_console->SetTextureForBackDrop("Sif_1", "Data/Images/Sif/sif_back_1.png");
	g_console->SetTextureForBackDrop("Sif_2", "Data/Images/Sif/sif_back_2.png");
	g_console->SetTextureForBackDrop("Sif_3", "Data/Images/Sif/sif_back_3.png");
	g_console->SetTextureForBackDrop("Sif_4", "Data/Images/Sif/sif_back_4.png");
	g_console->SetTextureForBackDrop("Sif_5", "Data/Images/Sif/sif_back_5.png");
	g_console->SetTextureForBackDrop("Sif_6", "Data/Images/Sif/sif_back_6.png");
	g_console->SetTextureForBackDrop("Sif_7", "Data/Images/Sif/sif_back_7.png");
	g_console->SetTextureForBackDrop("Sif_8", "Data/Images/Sif/sif_back_8.png");
	g_console->SetTextureForBackDrop("Sif_9", "Data/Images/Sif/sif_back_9.png");
	g_console->SetTextureForBackDrop("Sif_10", "Data/Images/Sif/sif_back_10.png");
	g_console->SetTextureForBackDrop("Sif_11", "Data/Images/Sif/sif_back_11.png");
}

uint Game::GetFirstAvailableShipSelectIncrement(uint start_idx /*= 0*/)
{
	for (uint index = start_idx; index < m_shipOptions.size(); ++index)
	{
		if (m_shipOptions[index].is_selected)
			continue;

		return index;
	}

	for (uint index = 0; index < start_idx; ++index)
	{
		if (m_shipOptions[index].is_selected)
			continue;

		return index;
	}

	return start_idx;
}

uint Game::GetFirstAvailableShipSelectDecrement(uint start_idx /*= 0*/)
{
	for (int index = start_idx; index >= 0; --index)
	{
		if (m_shipOptions[index].is_selected)
			continue;

		return index;
	}

	for (int index = m_shipOptions.size() - 1; index >= start_idx; --index)
	{
		if (m_shipOptions[index].is_selected)
			continue;

		return index;
	}

	return start_idx;
}

void Game::UpdateGameSession()
{
	if (m_gameSession->IsRunning()) {
		m_gameSession->Update();
	}
	else {
		if (!m_gameSession->Join(GetMyAddress(GAME_PORT))) {
			m_gameSession->Host(GAME_PORT);
			m_gameSession->StartListening();
		}
	}

	if (m_gameSession->AmIHost())
	{
		if (m_prevConnectionCount != m_gameSession->m_connections.size() - 1)
		{
			m_prevConnectionCount = GetNumSyncedPlayers();
			InitialSyncForPlayerInfo();
		}
	}

	if (!m_gameSession->IsRunning())
	{
		g_theApp->m_currentState = MAIN_MENU;
		g_theApp->m_infoMenuSelectIndex = 0;
		g_theApp->m_tryingToConnect = false;
		NetObjectCleanup();
		delete m_gameSession;
		m_gameSession = new TCPSession();
		m_playerList.clear();
		m_ships.clear();
		m_playerList.resize(m_gameSession->m_maxConnectionCount + 1);
		m_ships.resize(m_gameSession->m_maxConnectionCount + 1);
		SetUpGameMessages();
		NetSetup();
	}
}

void Game::RenderGameSessionInfo() const
{
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->SetShaderProgram(m_fontShader);

	uint start_height = 700;
	float start_x = 50.0f;
	if(m_gameSession->AmIHost())
		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), "Host", Rgba(255, 255, 0, 255));
	else if(m_gameSession->AmIClient())
		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), "Client", Rgba(255, 255, 0, 255));

	if (m_gameSession->AmIHost()) 
	{
		net_address_t host_addr = GetMyAddress(GAME_PORT);
		std::string host_addr_str = NetAddressToString(host_addr);
		std::string print_host = "Internal Address: " + host_addr_str;
		start_height -= (uint)std::floor(m_font->GetTextHeight("T", 1.0f) + 5.0f);
		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), print_host, Rgba(255, 255, 255, 255), 0.75);

		std::string ext_addr_str = GetExternalIPAddress() + ":" + std::to_string(GAME_PORT);
		std::string print_ext = "External Address: " + ext_addr_str;
		start_height -= (uint)std::floor(m_font->GetTextHeight("T", 1.0f) + 5.0f);
		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), print_ext, Rgba(255, 255, 255, 255), 0.75);

		std::string mode_string;

		if (m_hostState == ASTEROIDS)
		{
			mode_string = "Asteroids";
		}
		else if (m_hostState == LANDMINES)
		{
			mode_string = "Space mines";
		}
		else if (m_hostState == SPREAD_PICKUP)
		{
			mode_string = "Spread Shot";
		}
		else if (m_hostState == EXPLODE_PICKUP)
		{
			mode_string = "Explosive Shot";
		}
		else if (m_hostState == HEALTH_PICKUP)
		{
			mode_string = "Health";
		}

		std::string dm_string = "DM Mode: " + mode_string;
		start_height -= (uint)std::floor(m_font->GetTextHeight("T", 1.0f) + 5.0f);
		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), dm_string, Rgba(255, 255, 255, 255), 0.75);
	}

	uint num_clients = GetNumSyncedPlayers();

	uint total_num_clients = m_gameSession->m_maxConnectionCount;

	std::string num_clients_str = std::to_string(num_clients);
	std::string total_client_str = std::to_string(total_num_clients);
	std::string client_msg = "Players (" + num_clients_str + "/" + total_client_str + ")";
	start_height -= (uint)std::floor(m_font->GetTextHeight("T", 1.0f) + 5.0f);
	g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), client_msg, Rgba(200, 200, 200, 255), 0.75);
	start_height -= (uint)std::floor(m_font->GetTextHeight(client_msg, 0.75f) + 5.0f);

	for (uint index = 0; index < m_playerList.size(); ++index)
	{
		if (!m_playerList[index])
			continue;

		std::string print_client = "- ";
		Rgba color;
		if (index == m_gameSession->m_myConnection->m_connectionIndex)
		{
			color = Rgba(0, 255, 0, 255);
		}
		else
		{
			color = Rgba(200, 200, 200, 255);
		}

		if (!m_playerList[index]->m_ship)
			color = Rgba(255, 0, 0, 255);

		std::string index_str = std::to_string(index);

		if (m_gameSession->AmIHost()) 
		{
			if(!m_gameSession->m_connections[index])
				continue;

			std::string conn_addr = NetAddressToString(m_gameSession->m_connections[index]->m_address);
			print_client += " [" + index_str + "] " + m_playerList[index]->m_name + ". " + conn_addr;
		}
		else if (m_gameSession->AmIClient())
			print_client += " [" + index_str + "] " + m_playerList[index]->m_name;

		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), print_client, color, 0.6f);
		start_height -= (uint)std::floor(m_font->GetTextHeight(print_client, 0.6f) + 5.0f);

		std::stringstream thrust_stream;
		thrust_stream << std::fixed << std::setprecision(3) << m_playerList[index]->m_input.thrust;
		std::string thrust_str = thrust_stream.str();

		std::stringstream angle_stream;
		angle_stream << std::fixed << std::setprecision(3) << m_playerList[index]->m_input.steering_angle;
		std::string angle_str = angle_stream.str();

		std::string input_string = "  - input:   STEER: " + angle_str + "    THRUST: " + thrust_str;

		g_simpleRenderer->DrawTextWithFont(m_font, start_x, (float)(start_height - m_font->m_size), input_string, color, 0.6f);
		start_height -= (uint)std::floor(m_font->GetTextHeight(input_string, 0.6f) + 5.0f);
	}

	g_simpleRenderer->DisableBlend();
}

void Game::JoinGame(const net_address_t& addr_to_join)
{
	m_gameSession->Leave();
	m_gameSession->Join(addr_to_join);
}

void OnNameReceive(NetMessage* msg)
{
	uint8_t rec_index = msg->m_sender->m_connectionIndex;
	std::string name = msg->ReadString();
	if(!g_theGame->m_playerList[rec_index])
		g_theGame->m_playerList[rec_index] = new Player(name, rec_index);

	uint16_t frst_invalid_id;
	msg->read_bytes(&frst_invalid_id, sizeof(uint16_t));

	if (frst_invalid_id == 0 && g_theGame->m_gameSession->AmIHost())
	{
		NetMessage join(INIT_JOIN);
		join.m_sender = g_theGame->m_gameSession->m_myConnection;
		float conn_time = (float)GetCurrentTimeSeconds();
		join.write_bytes(&conn_time, sizeof(float));
		msg->m_sender->Send(&join);


		SyncNetObjects(msg->m_sender);
	}
}

void OnSync(NetMessage* msg)
{
	uint8_t player_index;
	msg->read(&player_index);
	std::string player_name = msg->ReadString();
	float thrust = 0.0f;
	float angle = 0.0f;
	msg->read_bytes(&thrust, sizeof(float));
	msg->read_bytes(&angle, sizeof(float));
	uint16_t frst_invalid_id;
	msg->read_bytes(&frst_invalid_id, sizeof(uint16_t));

	if (!g_theGame->m_playerList[player_index]) 
	{
		g_theGame->m_playerList[player_index] = new Player(player_name, player_index);
		g_theGame->m_playerList[player_index]->m_input.thrust = thrust;
		g_theGame->m_playerList[player_index]->m_input.steering_angle = angle;
		msg->read_bytes(&g_theGame->m_playerList[player_index]->m_shipSelectionIdx, sizeof(uint8_t));
		msg->read_bytes(&g_theGame->m_playerList[player_index]->m_hasLockedInShip, sizeof(bool));
	}
	else
	{
		g_theGame->m_playerList[player_index]->m_name = player_name;
		g_theGame->m_playerList[player_index]->m_input.thrust = thrust;
		g_theGame->m_playerList[player_index]->m_input.steering_angle = angle;
		g_theGame->m_shipOptions[g_theGame->m_playerList[player_index]->m_shipSelectionIdx].is_selected = false;
		msg->read_bytes(&g_theGame->m_playerList[player_index]->m_shipSelectionIdx, sizeof(uint8_t));
		msg->read_bytes(&g_theGame->m_playerList[player_index]->m_hasLockedInShip, sizeof(bool));
	}

	g_theGame->m_shipOptions[g_theGame->m_playerList[player_index]->m_shipSelectionIdx].is_selected = true;

	if (g_theGame->m_gameSession->AmIHost() && g_theGame->m_gameSession->IsRunning())
	{
		//uint16_t host_frst_invalid_id = NetObjectGetUnusedID();
		//if (frst_invalid_id == 0)
		//{
		//	SyncNetObjects(msg->m_sender);
		//}

		NetMessage sync(*msg);
		sync.m_sender = g_theGame->m_gameSession->m_myConnection;
		g_theGame->m_gameSession->SendMessageToOthers(sync);
	}
}

void OnLeave(NetMessage* msg)
{
	uint8_t conn_idx; 
	msg->read_bytes(&conn_idx, sizeof(uint8_t));

	//UnregisterNetObject(g_theGame->m_playerList[conn_idx].m_ship->m_netID)

	if(g_theGame->m_playerList[conn_idx])
	{
		delete g_theGame->m_playerList[conn_idx];
		g_theGame->m_playerList[conn_idx] = nullptr;

		//if(g_theGame->m_ships[conn_idx])
		//{
		//	delete g_theGame->m_ships[conn_idx];
		//	g_theGame->m_ships[conn_idx] = nullptr;
		//}
	}

	if (g_theGame->m_gameSession->AmIHost())
	{
		g_theGame->m_gameSession->DestroyConnection(g_theGame->m_gameSession->m_connections[conn_idx]);
		g_theGame->HostDestroyShipThatsReplicated(g_theGame->m_ships[conn_idx]);
		g_theGame->m_ships[conn_idx] = nullptr;

		NetMessage leaving(LEAVE);
		leaving.write_bytes(&conn_idx, sizeof(uint8_t));
		g_theGame->m_gameSession->SendMessageToOthers(leaving);
	}
}

void OnWorldSet(NetMessage* msg)
{
	float width;
	float height;

	msg->read_bytes(&width, sizeof(float));
	msg->read_bytes(&height, sizeof(float));

	g_theGame->m_worldDimensions = Vector2(width, height);

	g_theGame->m_backQuad->m_vertices.clear();
	g_theGame->m_backQuad->m_indices.clear();
	g_theGame->m_backQuad->CreateOneSidedQuad(Vector3(0.0f, 0.0f, 0.0), Vector3(g_theGame->m_worldDimensions.x, g_theGame->m_worldDimensions.y, 0.0f));
}

void SetBulletDirectionForSpread(Bullet* bullet, uint loop_idx)
{
	float spread = 120.0f;

	switch (loop_idx)
	{
	case 0:
	{
		bullet->m_orientationInDegrees -= (spread / 2.0f);
		break;
	}
	case 1:
	{
		bullet->m_orientationInDegrees -= (spread / 4.0f);
		break;
	}
	case 2:
	{
		return;
	}
	case 3:
	{
		bullet->m_orientationInDegrees += (spread / 4.0f);
		break;
	}
	case 4:
	{
		bullet->m_orientationInDegrees += (spread / 2.0f);
		break;
	}
	default:
	{
		break;
	}
	}

	float magnitude = bullet->m_velocity.CalcLength();
	bullet->m_velocity.x = magnitude * CosInDegrees(bullet->m_orientationInDegrees);
	bullet->m_velocity.y = magnitude * SinInDegrees(bullet->m_orientationInDegrees);
}

void OnFireRequest(NetMessage* msg)
{
	uint8_t index;
	msg->read_bytes(&index, sizeof(uint8_t));

	uint fire_state = g_theGame->m_ships[index]->m_fireState;
	//msg->read_bytes(&fire_state, sizeof(uint));

	if(fire_state == SPREAD_SHOT)
	{
		for (uint loop_index = 0; loop_index < 5; ++loop_index)
		{
			Bullet* bullet = g_theGame->HostCreateBullets(index);
			SetBulletDirectionForSpread(bullet, loop_index);
			g_theGame->m_bullets.push_back(bullet);
		}
	}
	else if (fire_state == NORMAL_SHOT)
	{
		Bullet* bullet = g_theGame->HostCreateBullets(index);
		g_theGame->m_bullets.push_back(bullet);
	}
	else if (fire_state == EXPLOSIVE)
	{
		Bullet* bullet = g_theGame->HostCreateBullets(index, true);
		g_theGame->m_bullets.push_back(bullet);
	}
}

void OnInitJoin(NetMessage* msg)
{
	if (!g_theGame->m_gameSession->AmIClient())
		return;

	float host_time;
	msg->read_bytes(&host_time, sizeof(float));
	SetHostTime(host_time);

	float client_time = (float)GetCurrentTimeSeconds();
	SetClientTime(client_time);
}

void Game::SetUpGameMessages()
{
	m_gameSession->RegisterMessageDefinition(SEND_NAME, OnNameReceive);
	m_gameSession->RegisterMessageDefinition(SYNC, OnSync);
	m_gameSession->RegisterMessageDefinition(LEAVE, OnLeave);
	m_gameSession->RegisterMessageDefinition(SHOOT, OnFireRequest);
	m_gameSession->RegisterMessageDefinition(INIT_JOIN, OnInitJoin);
}

void Game::SendName(const std::string& name)
{
	NetMessage msg(SEND_NAME);
	msg.m_sender = m_gameSession->m_myConnection;
	msg.WriteString(name.c_str());

	uint16_t frst_invalid_id = NetObjectGetUnusedID();
	msg.write_bytes(&frst_invalid_id, sizeof(uint16_t));

	m_gameSession->SendMessageToOthers(msg);

}

void Game::InitialSyncForPlayerInfo()
{
	for (uint index = 0; index < m_gameSession->m_connections.size(); ++index)
	{
		NetConnection* current_conn = m_gameSession->m_connections[index];
		if (current_conn == m_gameSession->m_myConnection)
			continue;

		Player* current_player = m_playerList[index];

		if (!current_player)
			continue;

		NetMessage sync(SYNC);
		sync.m_sender = m_gameSession->m_myConnection;
		uint8_t index_to_type = (uint8_t)index;
		sync.write(index_to_type);
		sync.WriteString(current_player->m_name.c_str());
		sync.write_bytes(&current_player->m_input.thrust, sizeof(float));
		sync.write_bytes(&current_player->m_input.steering_angle, sizeof(float));
		uint16_t frst_invalid_id = NetObjectGetUnusedID();
		sync.write_bytes(&frst_invalid_id, sizeof(uint16_t));
		sync.write_bytes(&current_player->m_shipSelectionIdx, sizeof(uint8_t));
		sync.write_bytes(&current_player->m_hasLockedInShip, sizeof(bool));
		m_gameSession->SendMessageToOthers(sync);

		NetMessage world(SET_WORLD);
		world.m_sender = m_gameSession->m_myConnection;
		world.write_bytes(&m_worldDimensions.x, sizeof(float));
		world.write_bytes(&m_worldDimensions.y, sizeof(float));
		m_gameSession->SendMessageToOthers(world);
	}

}

uint Game::GetNumSyncedPlayers() const
{
	uint num_synced = 0;
	for (uint index = 0; index < m_playerList.size(); ++index)
	{
		if (m_playerList[index])
			++num_synced;
	}
	return num_synced;
}

void Game::IncrementHostState()
{
	uint value = static_cast<uint>(m_hostState);

	switch (value)
	{
	case ASTEROIDS:
	{
		m_hostState = HEALTH_PICKUP;
		break;
	}
	case LANDMINES:
	{
		m_hostState = ASTEROIDS;
		break;
	}
	case EXPLODE_PICKUP:
	{
		m_hostState = LANDMINES;
		break;
	}
	case SPREAD_PICKUP:
	{
		m_hostState = EXPLODE_PICKUP;
		break;
	}
	case HEALTH_PICKUP:
	{
		m_hostState = SPREAD_PICKUP;
		break;
	}
	default: ASSERT_OR_DIE(false, "Host has entered invalid State!");
	}
}

void Game::DecrementHostState()
{
	uint value = static_cast<uint>(m_hostState);

	switch (value)
	{
	case ASTEROIDS:
	{
		m_hostState = LANDMINES;
		break;
	}
	case LANDMINES:
	{
		m_hostState = EXPLODE_PICKUP;
		break;
	}
	case EXPLODE_PICKUP:
	{
		m_hostState = SPREAD_PICKUP;
		break;
	}
	case SPREAD_PICKUP:
	{
		m_hostState = HEALTH_PICKUP;
		break;
	}
	case HEALTH_PICKUP:
	{
		m_hostState = ASTEROIDS;
		break;
	}
	default: ASSERT_OR_DIE(false, "Host has entered invalid State!");
	}
}

Ship* Game::HostCreateShipToReplicate(uint8_t player_idx)
{
	Ship *ship = new Ship();
	// setup ship
	ship->m_playerID = player_idx;
	ship->m_orientationInDegrees = GetRandomFloatInRange(0.0f, 360.0f);
	ship->m_position.x = GetRandomFloatInRange(-(g_theGame->m_worldDimensions.x * 0.5f), (g_theGame->m_worldDimensions.x * 0.5f));
	ship->m_position.y = GetRandomFloatInRange(-(g_theGame->m_worldDimensions.y * 0.5f), (g_theGame->m_worldDimensions.y * 0.5f));

	NetObject *nop = NetObjectReplicate(ship, NETOBJECT_SHIP);
	if(nop)
		ship->m_netID = nop->m_netID;

	return ship;
}

void Game::HostDestroyShipThatsReplicated(Ship *ship)
{
	NetObjectStopRelication(ship->m_netID);
	delete ship;
}

Bullet* Game::HostCreateBullets(uint8_t index, bool is_explosive /*= false*/)
{
	Bullet* new_bullet = new Bullet(index);
	new_bullet->m_spriteIdx = m_playerList[index]->m_shipSelectionIdx;
	Ship* ship = g_theGame->m_ships[index];

	if (is_explosive)
	{
		new_bullet->m_damage = 3;
		new_bullet->m_radius *= (float)new_bullet->m_damage;
	}

	new_bullet->m_orientationInDegrees = ship->m_orientationInDegrees;
	new_bullet->m_position.x = ship->m_position.x + ((ship->m_radius + new_bullet->m_radius) * CosInDegrees(ship->m_orientationInDegrees));
	new_bullet->m_position.y = ship->m_position.y + ((ship->m_radius + new_bullet->m_radius) * SinInDegrees(ship->m_orientationInDegrees));
	new_bullet->m_velocity = ship->m_velocity;

	NetObject *nop = NetObjectReplicate(new_bullet, NETOBJECT_BULLET);
	if (nop)
		new_bullet->m_netID = nop->m_netID;

	return new_bullet;
}

void Game::HostDestroyBullet(uint index)
{
	NetObjectStopRelication(m_bullets[index]->m_netID);

	std::swap(m_bullets.back(), m_bullets[index]);
	delete m_bullets.back();
	m_bullets.pop_back();
}

Landmine* Game::HostCreateLandmine()
{
	Landmine* mine = new Landmine();
	mine->m_position = Vector2(m_camera->m_position.x, m_camera->m_position.y);
	mine->m_spin = GetRandomFloatInRange(-25.0f, 25.0f);
	mine->m_orientationInDegrees = GetRandomFloatInRange(0.0f, 360.0f);

	NetObject *nop = NetObjectReplicate(mine, NETOBJECT_MINE);
	if (nop)
		mine->m_netID = nop->m_netID;

	return mine;
}

Powerup* Game::HostCreatePowerup(uint type)
{
	Powerup* pwr = new Powerup((ePowerUpAbility)type);
	pwr->m_position = Vector2(m_camera->m_position.x, m_camera->m_position.y);
	pwr->m_spin = GetRandomFloatInRange(-25.0f, 25.0f);
	pwr->m_orientationInDegrees = GetRandomFloatInRange(0.0f, 360.0f);


	NetObject *nop = NetObjectReplicate(pwr, NETOBJECT_PICKUP);
	if (nop)
		pwr->m_netID = nop->m_netID;

	return pwr;
}

Asteroid* Game::HostCreateAsteroid()
{
	Asteroid* asteroid = new Asteroid();
	asteroid->m_position = Vector2(m_camera->m_position.x,m_camera->m_position.y);

	float rndm_theta = GetRandomFloatInRange(0.0f, 360.0f);
	asteroid->m_velocity.x = asteroid->m_speed * CosInDegrees(rndm_theta);
	asteroid->m_velocity.y = asteroid->m_speed * SinInDegrees(rndm_theta);

	asteroid->m_spin = GetRandomFloatInRange(-50.0f, 50.0f);
	asteroid->m_orientationInDegrees = GetRandomFloatInRange(0.0f, 360.0f);

	NetObject *nop = NetObjectReplicate(asteroid, NETOBJECT_ASTEROID);
	if (nop)
		asteroid->m_netID = nop->m_netID;

	return asteroid;
}

void Game::HostDestroyAsteroid(uint index)
{
	NetObjectStopRelication(m_asteroids[index]->m_netID);

	std::swap(m_asteroids.back(), m_asteroids[index]);
	delete m_asteroids.back();
	m_asteroids.pop_back();
}

void Game::HostDestroyLandmine(uint index)
{
	NetObjectStopRelication(m_landmines[index]->m_netID);

	std::swap(m_landmines.back(), m_landmines[index]);
	delete m_landmines.back();
	m_landmines.pop_back();
}

void Game::HostDestroyPowerup(uint index)
{
	NetObjectStopRelication(m_powerups[index]->m_netID);

	std::swap(m_powerups.back(), m_powerups[index]);
	delete m_powerups.back();
	m_powerups.pop_back();
}

//////////////////////////////////////////////////////////////////////////////
// Appends are called by host to write, Process is called by client to read //
//////////////////////////////////////////////////////////////////////////////

// Ship
void ShipAppendCreateInfo(NetMessage *msg, void *ship_ptr)
{
	Ship *ship = (Ship*)ship_ptr;

	// constant
	msg->write(ship->m_playerID);

	// initial state
	msg->write_bytes(&ship->m_position.x, sizeof(float));
	msg->write_bytes(&ship->m_position.y, sizeof(float));
	msg->write_bytes(&ship->m_orientationInDegrees, sizeof(float));
}

void* ShipProcessCreateInfo(NetMessage *msg, NetObject *nop)
{
	uint8_t player_idx;
	msg->read_bytes(&player_idx, sizeof(uint8_t));
	Vector2 pos;
	msg->read_bytes(&pos.x, sizeof(float));
	msg->read_bytes(&pos.y, sizeof(float));
	float angle;
	msg->read_bytes(&angle, sizeof(float));

	Ship *ship = new Ship();
	ship->m_playerID = player_idx;
	ship->m_position = pos;
	ship->m_orientationInDegrees = angle;
	ship->m_netID = nop->m_netID;

	g_theGame->m_ships[player_idx] = ship;
	g_theGame->m_playerList[player_idx]->m_ship = ship;

	if (g_theGame->m_myPlayer->m_connectionIndex == player_idx)
		g_theGame->m_camera->m_position = Vector3(pos.x, pos.y, 0.0f);

	return ship;
}

void ShipProcessDestroyInfo(NetMessage *msg, void *ship_ptr)
{
	msg;
	Ship *ship = (Ship*)ship_ptr;

	// Do other information with ship - unregister, notify, whatever.
	g_theGame->m_ships[ship->m_playerID] = nullptr;
	g_theGame->m_playerList[ship->m_playerID]->m_ship = nullptr;

	delete ship;
}

// Bullets
void BulletAppendCreateInfo(NetMessage *msg, void *bullet_ptr)
{
	Bullet *bullet = (Bullet*)bullet_ptr;

	// constant
	msg->write_bytes(&bullet->m_ownerID, sizeof(uint8_t));

	// initial state
	msg->write_bytes(&bullet->m_position.x, sizeof(float));
	msg->write_bytes(&bullet->m_position.y, sizeof(float));
	msg->write_bytes(&bullet->m_orientationInDegrees, sizeof(float));
	msg->write_bytes(&bullet->m_spriteIdx, sizeof(uint8_t));
	msg->write_bytes(&bullet->m_radius, sizeof(float));
	msg->write_bytes(&bullet->m_damage, sizeof(int));
}

void* BulletProcessCreateInfo(NetMessage *msg, NetObject *nop)
{
	uint8_t owner_id;
	msg->read_bytes(&owner_id, sizeof(uint8_t));
	Vector2 pos;
	msg->read_bytes(&pos.x, sizeof(float));
	msg->read_bytes(&pos.y, sizeof(float));
	float angle;
	msg->read_bytes(&angle, sizeof(float));

	Ship* ship = g_theGame->m_ships[owner_id];

	Bullet *bullet = new Bullet(owner_id);
	msg->read_bytes(&bullet->m_spriteIdx, sizeof(uint8_t));
	msg->read_bytes(&bullet->m_radius, sizeof(float));
	msg->read_bytes(&bullet->m_damage, sizeof(int));

	bullet->m_position.x = ship->m_position.x + ((ship->m_radius + bullet->m_radius) * CosInDegrees(ship->m_orientationInDegrees));
	bullet->m_position.y = ship->m_position.y + ((ship->m_radius + bullet->m_radius) * SinInDegrees(ship->m_orientationInDegrees));

	bullet->m_orientationInDegrees = ship->m_orientationInDegrees;
	bullet->m_netID = nop->m_netID;
	bullet->m_velocity = g_theGame->m_ships[owner_id]->m_velocity;

 	g_theGame->m_bullets.push_back(bullet);

	return bullet;
}

void BulletProcessDestroyInfo(NetMessage *msg, void *bullet_ptr)
{
	msg;
	Bullet* bullet = (Bullet*)bullet_ptr;

	for (uint index = 0; index < g_theGame->m_bullets.size(); ++index)
	{
		if (g_theGame->m_bullets[index]->m_netID == bullet->m_netID)
		{
			g_theGame->m_bullets[index] = nullptr;
			std::swap(g_theGame->m_bullets.back(), g_theGame->m_bullets[index]);
			g_theGame->m_bullets.pop_back();
			break;
		}
	}

	delete bullet;
}

// Asteroids
void AsteroidAppendCreateInfo(NetMessage* msg, void* asteroid_ptr)
{
	Asteroid *asteroid = (Asteroid*)asteroid_ptr;

	// initial state
	msg->write_bytes(&asteroid->m_position.x, sizeof(float));
	msg->write_bytes(&asteroid->m_position.y, sizeof(float));
	msg->write_bytes(&asteroid->m_orientationInDegrees, sizeof(float));
	msg->write_bytes(&asteroid->m_spin, sizeof(float));
	msg->write_bytes(&asteroid->m_velocity.x, sizeof(float));
	msg->write_bytes(&asteroid->m_velocity.y, sizeof(float));
}

void* AsteroidProcessCreateInfo(NetMessage* msg, NetObject* obj)
{
	Vector2 pos;
	msg->read_bytes(&pos.x, sizeof(float));
	msg->read_bytes(&pos.y, sizeof(float));
	float angle;
	msg->read_bytes(&angle, sizeof(float));
	float spin;
	msg->read_bytes(&spin, sizeof(float));
	Vector2 vel;
	msg->read_bytes(&vel.x, sizeof(float));
	msg->read_bytes(&vel.y, sizeof(float));

	Asteroid* asteroid = new Asteroid();

	asteroid->m_netID = obj->m_netID;
	asteroid->m_position = pos;
	asteroid->m_velocity = vel;
	asteroid->m_spin = spin;
	asteroid->m_orientationInDegrees = angle;

	g_theGame->m_asteroids.push_back(asteroid);

	return asteroid;
}

void AsteroidProcessDestroyInfo(NetMessage* msg, void* asteroid_ptr)
{
	msg;
	Asteroid* asteroid = (Asteroid*)asteroid_ptr;

	for (uint index = 0; index < g_theGame->m_asteroids.size(); ++index)
	{
		if (g_theGame->m_asteroids[index]->m_netID == asteroid->m_netID)
		{
			g_theGame->m_asteroids[index] = nullptr;
			std::swap(g_theGame->m_asteroids.back(), g_theGame->m_asteroids[index]);
			g_theGame->m_asteroids.pop_back();
			break;
		}
	}

	delete asteroid;
}

// Landmines
void LandmineAppendCreateInfo(NetMessage* msg, void* mine_ptr)
{
	Landmine *mine = (Landmine*)mine_ptr;

	// initial state
	msg->write_bytes(&mine->m_position.x, sizeof(float));
	msg->write_bytes(&mine->m_position.y, sizeof(float));
	msg->write_bytes(&mine->m_orientationInDegrees, sizeof(float));
	msg->write_bytes(&mine->m_spin, sizeof(float));
}

void* LandmineProcessCreateInfo(NetMessage* msg, NetObject* obj)
{
	Vector2 pos;
	msg->read_bytes(&pos.x, sizeof(float));
	msg->read_bytes(&pos.y, sizeof(float));
	float angle;
	msg->read_bytes(&angle, sizeof(float));
	float spin;
	msg->read_bytes(&spin, sizeof(float));

	Landmine* mine = new Landmine();

	mine->m_netID = obj->m_netID;
	mine->m_position = pos;
	mine->m_spin = spin;
	mine->m_orientationInDegrees = angle;

	g_theGame->m_landmines.push_back(mine);

	return mine;
}

void LandmineProcessDestroyInfo(NetMessage* msg, void* mine_ptr)
{
	msg;
	Landmine* mine = (Landmine*)mine_ptr;

	for (uint index = 0; index < g_theGame->m_landmines.size(); ++index)
	{
		if (g_theGame->m_landmines[index]->m_netID == mine->m_netID)
		{
			g_theGame->m_landmines[index] = nullptr;
			std::swap(g_theGame->m_landmines.back(), g_theGame->m_landmines[index]);
			g_theGame->m_landmines.pop_back();
			break;
		}
	}

	delete mine;
}

// Powerups
void PickupAppendCreateInfo(NetMessage* msg, void* pwr_ptr)
{
	Powerup *pwr = (Powerup*)pwr_ptr;

	// initial state
	msg->write_bytes(&pwr->m_position.x, sizeof(float));
	msg->write_bytes(&pwr->m_position.y, sizeof(float));
	msg->write_bytes(&pwr->m_orientationInDegrees, sizeof(float));
	msg->write_bytes(&pwr->m_spin, sizeof(float));
	msg->write_bytes(&pwr->m_type, sizeof(uint));
}

void* PickupProcessCreateInfo(NetMessage* msg, NetObject* obj)
{
	Vector2 pos;
	msg->read_bytes(&pos.x, sizeof(float));
	msg->read_bytes(&pos.y, sizeof(float));
	float angle;
	msg->read_bytes(&angle, sizeof(float));
	float spin;
	msg->read_bytes(&spin, sizeof(float));
	uint type;
	msg->read_bytes(&type, sizeof(uint));

	Powerup* pwr = new Powerup((ePowerUpAbility)type);

	pwr->m_netID = obj->m_netID;
	pwr->m_position = pos;
	pwr->m_spin = spin;
	pwr->m_orientationInDegrees = angle;

	g_theGame->m_powerups.push_back(pwr);

	return pwr;
}

void PickupProcessDestroyInfo(NetMessage* msg, void* pwr_ptr)
{
	msg;
	Powerup* pwr = (Powerup*)pwr_ptr;

	for (uint index = 0; index < g_theGame->m_powerups.size(); ++index)
	{
		if (g_theGame->m_powerups[index]->m_netID == pwr->m_netID)
		{
			g_theGame->m_powerups[index] = nullptr;
			std::swap(g_theGame->m_powerups.back(), g_theGame->m_powerups[index]);
			g_theGame->m_powerups.pop_back();
			break;
		}
	}

	delete pwr;
}


//////////////////////////////////////////////////////////////////////////
// Network Update
//////////////////////////////////////////////////////////////////////////

// Ships
struct net_ship_snapshot_t
{
	Vector2 position;
	Vector2 velocity;
	float angle;
	int health;
};

void ShipApplySnapShot(void* local_obj, void* last_snap, float delta_time)
{
	Ship* ship = (Ship*)local_obj;
	net_ship_snapshot_t* snap_shot = (net_ship_snapshot_t*)last_snap;

	ship->m_deadReckon.position = snap_shot->position + (snap_shot->velocity * delta_time);
	float turn_rate = g_theGame->m_playerList[ship->m_playerID]->m_input.steering_angle;
	ship->m_deadReckon.orientation_in_degrees = snap_shot->angle + (turn_rate * delta_time);
	ship->m_velocity = snap_shot->velocity;
	ship->m_health = snap_shot->health;
}

void ShipGetCurrentSnapShot(void* curr_snap, void* local_obj)
{
	Ship* ship = (Ship*)local_obj;

	((net_ship_snapshot_t*)curr_snap)->position = ship->m_position;
	((net_ship_snapshot_t*)curr_snap)->velocity = ship->m_velocity;
	((net_ship_snapshot_t*)curr_snap)->angle = ship->m_orientationInDegrees;
	((net_ship_snapshot_t*)curr_snap)->health = ship->m_health;
}

void ShipAppendSnapShot(NetMessage* msg, void* curr_snap)
{
	net_ship_snapshot_t* snap_shot = (net_ship_snapshot_t*)curr_snap;

	msg->write_bytes(&snap_shot->position.x, sizeof(float));
	msg->write_bytes(&snap_shot->position.y, sizeof(float));

	msg->write_bytes(&snap_shot->velocity.x, sizeof(float));
	msg->write_bytes(&snap_shot->velocity.y, sizeof(float));

	msg->write_bytes(&snap_shot->angle, sizeof(float)); 
	msg->write_bytes(&snap_shot->health, sizeof(int));
}

void ShipProcessSnapShot(void* last_snap, NetMessage* msg)
{
	net_ship_snapshot_t* snap_shot = (net_ship_snapshot_t*)last_snap;

	msg->read_bytes(&snap_shot->position.x, sizeof(float));
	msg->read_bytes(&snap_shot->position.y, sizeof(float));

	msg->read_bytes(&snap_shot->velocity.x, sizeof(float));
	msg->read_bytes(&snap_shot->velocity.y, sizeof(float));

	msg->read_bytes(&snap_shot->angle, sizeof(float));
	msg->read_bytes(&snap_shot->health, sizeof(int));
}

size_t ShipSnapShotSize()
{
	return sizeof(net_ship_snapshot_t);
}

// Bullets
struct net_bullet_snapshot_t
{
	Vector2 position;
	Vector2 velocity;
	float angle;
	uint8_t ownerID;
};

void BulletApplySnapShot(void* local_obj, void* last_snap, float delta_time)
{
	Bullet* bullet = (Bullet*)local_obj;
	net_bullet_snapshot_t* snap_shot = (net_bullet_snapshot_t*)last_snap;

	bullet->m_deadReckon.position = snap_shot->position + (snap_shot->velocity * delta_time);
	bullet->m_velocity = snap_shot->velocity;
	bullet->m_orientationInDegrees = snap_shot->angle;
	bullet->m_ownerID = snap_shot->ownerID;
}

void BulletGetCurrentSnapShot(void* curr_snap, void* local_obj)
{
	Bullet* bullet = (Bullet*)local_obj;

	((net_bullet_snapshot_t*)curr_snap)->position = bullet->m_position;
	((net_bullet_snapshot_t*)curr_snap)->velocity = bullet->m_velocity;
	((net_bullet_snapshot_t*)curr_snap)->angle = bullet->m_orientationInDegrees;
	((net_bullet_snapshot_t*)curr_snap)->ownerID = bullet->m_ownerID;
}

void BulletAppendSnapShot(NetMessage* msg, void* curr_snap)
{
	net_bullet_snapshot_t* snap_shot = (net_bullet_snapshot_t*)curr_snap;

	msg->write_bytes(&snap_shot->position.x, sizeof(float));
	msg->write_bytes(&snap_shot->position.y, sizeof(float));

	msg->write_bytes(&snap_shot->velocity.x, sizeof(float));
	msg->write_bytes(&snap_shot->velocity.y, sizeof(float));

	msg->write_bytes(&snap_shot->angle, sizeof(float));
	msg->write_bytes(&snap_shot->ownerID, sizeof(uint8_t));
}

void BulletProcessSnapShot(void* last_snap, NetMessage* msg)
{
	net_bullet_snapshot_t* snap_shot = (net_bullet_snapshot_t*)last_snap;

	msg->read_bytes(&snap_shot->position.x, sizeof(float));
	msg->read_bytes(&snap_shot->position.y, sizeof(float));

	msg->read_bytes(&snap_shot->velocity.x, sizeof(float));
	msg->read_bytes(&snap_shot->velocity.y, sizeof(float));

	msg->read_bytes(&snap_shot->angle, sizeof(float));
	msg->read_bytes(&snap_shot->ownerID, sizeof(uint8_t));
}

size_t BulletSnapShotSize()
{
	return sizeof(net_bullet_snapshot_t);
}

// Asteroids
struct net_asteroid_snapshot_t
{
	Vector2 position;
	Vector2 velocity;
	float angle;
	int health;
};

void AsteroidApplySnapshot(void* local_obj, void* last_snap, float delta_time)
{
	Asteroid* asteroid = (Asteroid*)local_obj;
	net_asteroid_snapshot_t* snap_shot = (net_asteroid_snapshot_t*)last_snap;

	asteroid->m_deadReckon.position = snap_shot->position + (snap_shot->velocity * delta_time);
	asteroid->m_velocity = snap_shot->velocity;
	asteroid->m_deadReckon.orientation_in_degrees = snap_shot->angle + (asteroid->m_spin * delta_time);
	asteroid->m_health = snap_shot->health;
}

void AsteroidGetCurrentSnapShot(void* curr_snap, void* local_obj)
{
	Asteroid* asteroid = (Asteroid*)local_obj;

	((net_asteroid_snapshot_t*)curr_snap)->position = asteroid->m_position;
	((net_asteroid_snapshot_t*)curr_snap)->velocity = asteroid->m_velocity;
	((net_asteroid_snapshot_t*)curr_snap)->angle = asteroid->m_orientationInDegrees;
	((net_asteroid_snapshot_t*)curr_snap)->health = asteroid->m_health;
}

void AsteroidAppendSnapShot(NetMessage* msg, void* curr_snap)
{
	net_asteroid_snapshot_t* snap_shot = (net_asteroid_snapshot_t*)curr_snap;

	msg->write_bytes(&snap_shot->position.x, sizeof(float));
	msg->write_bytes(&snap_shot->position.y, sizeof(float));

	msg->write_bytes(&snap_shot->velocity.x, sizeof(float));
	msg->write_bytes(&snap_shot->velocity.y, sizeof(float));

	msg->write_bytes(&snap_shot->angle, sizeof(float));
	msg->write_bytes(&snap_shot->health, sizeof(uint8_t));
}

void AsteroidProcessSnapShot(void* last_snap, NetMessage* msg)
{
	net_asteroid_snapshot_t* snap_shot = (net_asteroid_snapshot_t*)last_snap;

	msg->read_bytes(&snap_shot->position.x, sizeof(float));
	msg->read_bytes(&snap_shot->position.y, sizeof(float));

	msg->read_bytes(&snap_shot->velocity.x, sizeof(float));
	msg->read_bytes(&snap_shot->velocity.y, sizeof(float));

	msg->read_bytes(&snap_shot->angle, sizeof(float));
	msg->read_bytes(&snap_shot->health, sizeof(int));
}

size_t AsteroidSnapShotSize()
{
	return sizeof(net_asteroid_snapshot_t);
}

void Game::NetSetup()
{
	NetObjectStartup();
	RegisterNetObjectSession(m_gameSession);

	NetObjectTypeDefinition ship_defn;
	ship_defn.m_appendCreateInfo = ShipAppendCreateInfo;
	ship_defn.m_processCreateInfo = ShipProcessCreateInfo;
	ship_defn.m_appendDestroyInfo = nullptr;
	ship_defn.m_processDestroyInfo = ShipProcessDestroyInfo;
	ship_defn.m_applySnapshot = ShipApplySnapShot;
	ship_defn.m_getCurrentSnapShot = ShipGetCurrentSnapShot;
	ship_defn.m_appendSnapshot = ShipAppendSnapShot;
	ship_defn.m_processSnapshot = ShipProcessSnapShot;
	ship_defn.m_getSnapShotSize = ShipSnapShotSize;
	NetObjectSystemRegisterType(NETOBJECT_SHIP, ship_defn);

	NetObjectTypeDefinition bullet_defn;
	bullet_defn.m_appendCreateInfo = BulletAppendCreateInfo;
	bullet_defn.m_processCreateInfo = BulletProcessCreateInfo;
	bullet_defn.m_appendDestroyInfo = nullptr;
	bullet_defn.m_processDestroyInfo = BulletProcessDestroyInfo;
	bullet_defn.m_applySnapshot = BulletApplySnapShot;
	bullet_defn.m_getCurrentSnapShot = BulletGetCurrentSnapShot;
	bullet_defn.m_appendSnapshot = BulletAppendSnapShot;
	bullet_defn.m_processSnapshot = BulletProcessSnapShot;
	bullet_defn.m_getSnapShotSize = BulletSnapShotSize; 
	NetObjectSystemRegisterType(NETOBJECT_BULLET, bullet_defn);

	NetObjectTypeDefinition asteroid_defn;
	asteroid_defn.m_appendCreateInfo = AsteroidAppendCreateInfo;
	asteroid_defn.m_processCreateInfo = AsteroidProcessCreateInfo;
	asteroid_defn.m_appendDestroyInfo = nullptr;
	asteroid_defn.m_processDestroyInfo = AsteroidProcessDestroyInfo;
	asteroid_defn.m_applySnapshot = AsteroidApplySnapshot;
	asteroid_defn.m_getCurrentSnapShot = AsteroidGetCurrentSnapShot;
	asteroid_defn.m_appendSnapshot = AsteroidAppendSnapShot;
	asteroid_defn.m_processSnapshot = AsteroidProcessSnapShot;
	asteroid_defn.m_getSnapShotSize = AsteroidSnapShotSize;
	NetObjectSystemRegisterType(NETOBJECT_ASTEROID, asteroid_defn);

	NetObjectTypeDefinition mine_defn;
	mine_defn.m_appendCreateInfo = LandmineAppendCreateInfo;
	mine_defn.m_processCreateInfo = LandmineProcessCreateInfo;
	mine_defn.m_appendDestroyInfo = nullptr;
	mine_defn.m_processDestroyInfo = LandmineProcessDestroyInfo;
	mine_defn.m_applySnapshot = nullptr;
	mine_defn.m_getCurrentSnapShot = nullptr;
	mine_defn.m_appendSnapshot = nullptr;
	mine_defn.m_processSnapshot = nullptr;
	mine_defn.m_getSnapShotSize = nullptr;
	NetObjectSystemRegisterType(NETOBJECT_MINE, mine_defn);

	NetObjectTypeDefinition pwr_defn;
	pwr_defn.m_appendCreateInfo = PickupAppendCreateInfo;
	pwr_defn.m_processCreateInfo = PickupProcessCreateInfo;
	pwr_defn.m_appendDestroyInfo = nullptr;
	pwr_defn.m_processDestroyInfo = PickupProcessDestroyInfo;
	pwr_defn.m_applySnapshot = nullptr;
	pwr_defn.m_getCurrentSnapShot = nullptr;
	pwr_defn.m_appendSnapshot = nullptr;
	pwr_defn.m_processSnapshot = nullptr;
	pwr_defn.m_getSnapShotSize = nullptr;
	NetObjectSystemRegisterType(NETOBJECT_PICKUP, pwr_defn);

	EstablishNetObjectMessages();
	SetNetObjectRefreshRate(20.0f);
}