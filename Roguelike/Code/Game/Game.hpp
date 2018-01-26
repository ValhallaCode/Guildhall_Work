#pragma once
#include "Game/GameCommons.hpp"
#include "Game/Camera3D.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/RHI/Material.hpp"
#include "Engine/Core/CommandSystem.hpp"
#include "Game/World.hpp"
#include <map>

class Texture2D;
class Sampler;
class KerningFont;
class Adventure;

enum GameState : unsigned int
{
	MAIN_MENU,
	PLAY,
	STATS_SCREEN,
	ADVENTURE_SELECT,
	NUMSTATES
};

class Game
{
public:
	Game();
	~Game();
	void Update(float deltaSeconds);
	void UpdatePlayState(float deltaSeconds);
	void UpdateConsole();
	void UpdateMainMenuState();
	void AdventureSelectUpdate();
	void AdventureSelectRender() const;
	void UpdateStatState();
	void Render() const;
	void RenderPlayState() const;
	void RenderMainMenu() const;
	void RenderMainMenuOptions() const;
	void RenderStatsScreen() const;
	void RenderCurrentStats() const;
	void RenderBaseStats() const;
	void RenderEquippedList() const;
	void RenderInventoryList() const;
	void RenderInstructionsForStatState() const;
	void Start2DDraw() const;
	void DrawConsole() const;
	void Draw2DGridAndAxis(float factorToDraw = 1.0f) const;
	void DrawGrid2D(float factorToDraw = 1.0f) const;
	void DrawAxis2D(float factorToDraw = 1.0f) const;
	void DrawBlinkyCursor() const;
	void UpdateCursorBlink();
	void KeyUp(unsigned char asKey);
	void KeyDown(unsigned char asKey);
	void Initialize();
	void InitializeMapList();
	void InitializeConsole();
	void InitializeTileRegistry();
	void InitializeCharacterRegistry();
	void InitializeInteractableRegistry();
	void InitializeItemRegistry();
	void InitializeLootList();
	void CheckForVictoryCondition();
	void CheckForFailure();
	void InitializeAdventureList();
public:
	bool m_victory;
	bool m_failure;
	bool m_isQuitting;
	int m_cursorCounter;
	bool m_shouldBlink;
	bool m_isConsoleActive;
	std::string m_inputText;
	timeConstants m_timeConst;
	ConstantBuffer* m_constBuffer;
	ShaderProgram* m_shadowBox;
	ShaderProgram* m_fontShader;
	ShaderProgram* m_defaultShader;
	Sampler* m_sampler;
	Texture2D* m_defaultTex;
	Camera3D* m_camera;
	float m_totalTimePassed;
	KerningFont* m_font;
	Mesh* m_fontBackDrop;
	World* m_world;
	GameState m_currentState;
	Adventure* m_currentAdventure;
};
