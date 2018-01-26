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
#include "Engine/EngineConfig.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Game/TileDescription.hpp"
#include "Game/CharacterDescription.hpp"
#include "Game/InteractableDescription.hpp"
#include "Game/ItemDescription.hpp"
#include "Game/MapDescription.hpp"
#include "Game/Behavior.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Game/Player.hpp"
#include "Game/NPC.hpp"
#include "Game/Adventure.hpp"
#include <sstream>


Config* g_config = nullptr;

//*******************************************************************
void ConsoleClear(const std::string& args)
{
UNUSED(args);
g_console->m_commandLog.clear();
g_console->m_cursorY = WORLD_HEIGHT;
}

//*******************************************************************
void ConsoleHelp(const std::string& args)
{
UNUSED(args);

if (args == "default")
{
for (auto iterate : g_console->m_commands)
{
std::string command = iterate.first;
std::string description;
command_cb commandInfo;
if (command == "clear")
{
description = "Will clear the console log history.";
}
else if (command == "help")
{
description = "Will print a list of all commands if given 'default' or a specific command description.";
}
else if (command == "quit")
{
description = "Will exit out of the game.";
}

std::string print = command + ": " + description;
commandInfo.color = Rgba(0, 255, 0, 255);
commandInfo.message = print;
g_console->m_commandLog.push_back(commandInfo);
g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight; //change to renderer
}
}
else
{
std::string description;
command_cb commandInfo;
if (args == "clear")
{
description = "Will clear the console log history.";
}
else if (args == "help")
{
description = "Will print a list of all commands if given 'default' or a specific command description.";
}
else if (args == "quit")
{
description = "Will exit out of the game.";
}

std::string print = args + ": " + description;
commandInfo.color = Rgba(0, 255, 0, 255);
commandInfo.message = print;
g_console->m_commandLog.push_back(commandInfo);
g_console->m_cursorY -= g_console->m_consoleFont->m_lineHeight; //change to renderer
}
}

//*******************************************************************
void ConsoleQuit(const std::string& args)
{
UNUSED(args);
g_theGame->m_isQuitting = true;
g_theApp->OnExitRequested();
}

Game::Game()
	:m_isQuitting(false)
	, m_inputText("")
	, m_cursorCounter(10)
	, m_shouldBlink(false)
	, m_isConsoleActive(false)
	, m_camera(nullptr)
	, m_defaultTex(nullptr)
	, m_totalTimePassed(0.0f)
	, m_fontBackDrop(nullptr)
	, m_defaultShader(nullptr)
	, m_world(nullptr)
	, m_currentState(MAIN_MENU)
	, m_currentAdventure(nullptr)
	, m_victory(false)
	, m_failure(false)
{	
	
}

Game::~Game()
{
	
}

void Game::Update(float deltaSeconds)
{
	m_timeConst.GAME_TIME += deltaSeconds;
	m_timeConst.GAME_FRAME_TIME = deltaSeconds;
	m_constBuffer->Update(g_simpleRenderer->m_context, &m_timeConst);
	UpdateCursorBlink();

	if (m_currentState == PLAY) {
		UpdatePlayState(deltaSeconds);
		return;
	}

	if (m_currentState == MAIN_MENU) {
		UpdateMainMenuState();
		return;
	}

	if (m_currentState == STATS_SCREEN) {
		UpdateStatState();
		return;
	}

	if (m_currentState == ADVENTURE_SELECT) {
		AdventureSelectUpdate();
		return;
	}
}

void Game::UpdatePlayState(float deltaSeconds)
{
	if (g_IsTheGamePaused == true)
	{
		deltaSeconds *= 0.0f;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ESCAPE) && !m_isConsoleActive)
	{
		m_currentState = MAIN_MENU;
		if (m_failure)
		{
			delete m_world;
			m_world = nullptr;
			return;
		}
	}

	if (g_theInputSystem->WasKeyJustPressed('P'))
	{
		m_currentState = STATS_SCREEN;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_F3))
		g_debug = !g_debug;

	UpdateConsole();

	if (!m_isConsoleActive)
	{
		m_world->Update(deltaSeconds);
		CheckForVictoryCondition();
		CheckForFailure();
	}
}

void Game::UpdateConsole()
{
	if (g_theInputSystem->WasKeyJustPressed(KEY_TILDE) || (g_theInputSystem->WasKeyJustPressed(KEY_ESCAPE) && g_theGame->m_inputText.empty()))
	{
		g_theGame->m_isConsoleActive = !g_theGame->m_isConsoleActive;
		g_console->m_commandLog.clear();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ESCAPE))
	{
		g_theGame->m_inputText.clear();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_BACK) && g_theGame->m_inputText.size() > 0)
	{
		g_theGame->m_inputText.pop_back();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ENTER) && g_theGame->m_isConsoleActive)
	{
		g_console->RunCommand(g_theGame->m_inputText);
		g_theGame->m_inputText.clear();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_TILDE) && g_theGame->m_isConsoleActive)
	{
		if (!m_isConsoleActive)
			m_inputText.clear();
	}
}

void Game::UpdateMainMenuState()
{
	if (g_theInputSystem->WasKeyJustPressed('1') || g_theInputSystem->WasKeyJustPressed(NUMPAD_1))
	{
		if (m_world != nullptr)
		{
			delete m_world;
			m_world = nullptr;
		}

		//m_world = new World();
		m_currentState = ADVENTURE_SELECT;
	}

	if ((g_theInputSystem->WasKeyJustPressed('2') || g_theInputSystem->WasKeyJustPressed(NUMPAD_2)) && m_world != nullptr)
	{
		m_currentState = PLAY;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ESCAPE) && !m_isConsoleActive)
	{
		g_theApp->m_isQuitting = true;
		return;
	}

	m_isConsoleActive = false;
}

void Game::AdventureSelectUpdate()
{
	if (g_theInputSystem->WasKeyJustPressed(KEY_ESCAPE) && !m_isConsoleActive)
	{
		m_currentState = MAIN_MENU;
	}

	for (auto iterate = Adventure::s_adventureList.begin(); iterate != Adventure::s_adventureList.end(); iterate++)
	{
		unsigned int index = iterate->second->m_adventureID;

		if (index > 10)
			break;

		int indexToCheck = index;
		if (indexToCheck == 10)
			indexToCheck = 0;

		int idToCheck = indexToCheck + 48;
		if (g_theInputSystem->WasKeyJustPressed(idToCheck))
		{
			m_currentAdventure = iterate->second;
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ENTER) && !m_isConsoleActive && m_currentAdventure != nullptr)
	{
		if (m_world != nullptr)
		{
			delete m_world;
			m_world = nullptr;
		}

		m_currentState = PLAY;
		m_world = new World(*m_currentAdventure);
	}
}

void Game::AdventureSelectRender() const
{
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->SetShaderProgram(m_defaultShader);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);
	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->DrawOneSidedQuad(Vector3(0.0f, 0.0f, 0.0f), Vector3((float)WORLD_WIDTH * 0.5f, (float)WORLD_HEIGHT * 0.5f, 0.0f), Rgba(0, 0, 0, 170));

	std::vector<ToolTip> currentStatColection;
	ToolTip currentStatTitle;
	currentStatTitle.m_text = "Adventure List: ";
	currentStatTitle.m_color = Rgba(255, 255, 255, 255);
	currentStatTitle.m_size = 1.3f;
	currentStatColection.push_back(currentStatTitle);

	for (auto iterate = Adventure::s_adventureList.begin(); iterate != Adventure::s_adventureList.end(); iterate++)
	{
		ToolTip currentStatsText;
		if(iterate->second->m_adventureID == 10)
			currentStatsText.m_text = "0) " + iterate->second->m_title;
		else
			currentStatsText.m_text = std::to_string(iterate->second->m_adventureID) + ") " + iterate->second->m_title;
		if(iterate->second == m_currentAdventure)
			currentStatsText.m_color = Rgba(255, 199, 0, 255);
		else
			currentStatsText.m_color = Rgba(255, 255, 255, 255);
		currentStatsText.m_size = 1.0f;
		currentStatColection.push_back(currentStatsText);
	}

	int currentDrawHeight = m_font->m_lineHeight;
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	for (auto iterate = currentStatColection.begin(); iterate != currentStatColection.end(); ++iterate)
	{
		g_simpleRenderer->DrawTextWithFont(m_font, (float)DEFAULT_WINDOW_WIDTH * 0.4f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.05f) + m_font->m_size + currentDrawHeight), iterate->m_text, iterate->m_color, iterate->m_size);
		currentDrawHeight -= m_font->m_lineHeight;
	}

	if (m_currentAdventure != nullptr)
	{
		g_simpleRenderer->DrawTextCenteredOnPosition2D(m_font, Vector2((float)DEFAULT_WINDOW_WIDTH * 0.85f, (float)DEFAULT_WINDOW_HEIGHT * 0.25f), "-Press Enter To Start-", Rgba(255, 255, 255, 255), 2.0f);
	}
}

void Game::UpdateStatState()
{
	if (g_theInputSystem->WasKeyJustPressed(KEY_ESCAPE))
	{
		m_currentState = PLAY;
		return;
	}

	Player* player = dynamic_cast<Player*>(m_world->m_currentPlayer);
	Inventory* inventory = player->m_inventory;
	for (unsigned int index = 0; index < inventory->m_itemList.size(); ++index)
	{
		Item* item = inventory->m_itemList[index];
		if (item == nullptr)
			continue;

		char c = (char)(index + 65);

		if (g_theInputSystem->WasKeyJustPressed(c) && g_theInputSystem->IsKeyDown(KEY_SHIFT))
		{
			//Drop Consume Action
			IntVector2 playerPos = player->GetPositionInMap();
			Tile* tile = m_world->m_currentMap->GetTileAtCoords(playerPos);
			player->DropItem(*item, tile->m_inventory);
			break;
		}

		if (g_theInputSystem->WasKeyJustPressed(c))
		{
			//Equip/Unequip action
			if (player->IsItemEquiped(*item))
			{
				player->RemoveEquippedItem(*item);
				break;
			}

			EquipType newType = item->m_type;
			if(newType != EquipType::NONE)
			{
				Item* previousEquip = player->GetEquippedItemByType(newType);
				if(previousEquip != nullptr)
					player->RemoveEquippedItem(*previousEquip);
				
				player->EquipItemIfAvailableSlot(item);
				//player->m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item->m_type)] = item;
				//player->ApplyEquippedStats(item->m_stats);
				break;
			}
			break;
		}
	}
}

void Game::Render() const
{
	Start2DDraw();

	if (m_currentState == MAIN_MENU)
		RenderMainMenu();

	if (m_currentState == PLAY)
		RenderPlayState();

	if (m_currentState == STATS_SCREEN)
		RenderStatsScreen();

	if (m_currentState == ADVENTURE_SELECT)
		AdventureSelectRender();

	DrawConsole();
}

void Game::RenderPlayState() const
{
	if (m_world != nullptr) {
		m_world->Render();
	}
}

void Game::RenderMainMenu() const 
{
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->SetShaderProgram(m_defaultShader);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);
	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->DrawOneSidedQuad(Vector3(0.0f, 0.0f, 0.0f), Vector3((float)WORLD_WIDTH * 0.5f, (float)WORLD_HEIGHT * 0.5f, 0.0f), Rgba(255, 255, 255, 255));

	RenderMainMenuOptions();

	g_simpleRenderer->DisableBlend();
}

void Game::RenderMainMenuOptions() const
{
	unsigned int drawHeight = m_font->m_lineHeight;

	g_simpleRenderer->SetShaderProgram(m_fontShader);

	g_simpleRenderer->DrawTextCenteredOnPosition2D(m_font, Vector2((float)DEFAULT_WINDOW_WIDTH * 0.75f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 0.15f) + m_font->m_size + drawHeight)), "Esc : Exit Main Menu", Rgba(0, 0, 0, 255));
	drawHeight += m_font->m_lineHeight;

	Rgba resumeColor;
	if (m_world == nullptr)
		resumeColor = Rgba(100, 100, 100, 255);
	else
		resumeColor = Rgba(0, 0, 0, 255);

	g_simpleRenderer->DrawTextCenteredOnPosition2D(m_font, Vector2((float)DEFAULT_WINDOW_WIDTH * 0.75f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 0.15f) + m_font->m_size + drawHeight)), "2 : Resume Game", resumeColor);
	drawHeight += m_font->m_lineHeight;

	g_simpleRenderer->DrawTextCenteredOnPosition2D(m_font, Vector2((float)DEFAULT_WINDOW_WIDTH * 0.75f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 0.15f) + m_font->m_size + drawHeight)), "1 : Play Game", Rgba(0, 0, 0, 255));

	g_simpleRenderer->DrawTextCenteredOnPosition2D(m_font, Vector2((float)DEFAULT_WINDOW_WIDTH * 0.75f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 0.95f) + m_font->m_size)), "Rogue-Like", Rgba(0, 0, 0, 255), 3.0f);
}

void Game::RenderStatsScreen() const
{
	if (m_world != nullptr)
		m_world->Render();

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->SetShaderProgram(m_defaultShader);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);
	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->DrawOneSidedQuad(Vector3((float)m_world->m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_world->m_currentPlayer->GetPositionInMap().y + 0.5f, 0.0f), Vector3((float)WORLD_WIDTH * 0.5f, (float)WORLD_HEIGHT * 0.5f, 0.0f), Rgba(0, 0, 0, 170));

	RenderBaseStats();
	RenderCurrentStats();
	RenderEquippedList();
	RenderInventoryList();
	RenderInstructionsForStatState();

	g_simpleRenderer->DisableBlend();
}

void Game::RenderCurrentStats() const
{
	std::vector<ToolTip> currentStatColection;
	ToolTip currentStatTitle;
	currentStatTitle.m_text = "Current Stats: ";
	currentStatTitle.m_color = Rgba(255, 255, 255, 255);
	currentStatTitle.m_size = 1.3f;
	currentStatColection.push_back(currentStatTitle);

	Stat* currentStats = m_world->m_currentPlayer->m_stats;
	for (unsigned int index = 0; index < (unsigned int)StatType::NUM_STAT_TYPES; ++index)
	{
		ToolTip currentStatsText;
		currentStatsText.m_text = currentStats->GetStatNameByIndex(index) + " : " + std::to_string(currentStats->m_statCollection[index]);
		currentStatsText.m_color = Rgba(255, 255, 255, 255);
		currentStatsText.m_size = 1.0f;
		currentStatColection.push_back(currentStatsText);
	}

	int currentDrawHeight = m_font->m_lineHeight;
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	for (auto iterate = currentStatColection.begin(); iterate != currentStatColection.end(); ++iterate)
	{
		g_simpleRenderer->DrawTextWithFont(m_font, (float)DEFAULT_WINDOW_WIDTH * 0.4f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.05f) + m_font->m_size + currentDrawHeight), iterate->m_text, iterate->m_color, iterate->m_size);
		currentDrawHeight -= m_font->m_lineHeight;
	}
}

void Game::RenderBaseStats() const
{
	std::vector<ToolTip> baseStatColection;
	ToolTip baseStatTitle;
	baseStatTitle.m_text = "Base Stats: ";
	baseStatTitle.m_color = Rgba(255, 255, 255, 255);
	baseStatTitle.m_size = 1.3f;
	baseStatColection.push_back(baseStatTitle);

	Stat baseStats = m_world->m_currentPlayer->GetBaseWithoutEquipmentStats();
	for (unsigned int index = 0; index < (unsigned int)StatType::NUM_STAT_TYPES; ++index)
	{
		ToolTip baseStatsText;
		baseStatsText.m_text = baseStats.GetStatNameByIndex(index) + " : " + std::to_string(baseStats.m_statCollection[index]);
		baseStatsText.m_color = Rgba(255, 255, 255, 255);
		baseStatsText.m_size = 1.0f;
		baseStatColection.push_back(baseStatsText);
	}

	int basedDrawHeight = m_font->m_lineHeight;
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	for (auto iterate = baseStatColection.begin(); iterate != baseStatColection.end(); ++iterate)
	{
		g_simpleRenderer->DrawTextWithFont(m_font, (float)DEFAULT_WINDOW_WIDTH * 0.05f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.05f) + m_font->m_size + basedDrawHeight), iterate->m_text, iterate->m_color, iterate->m_size);
		basedDrawHeight -= m_font->m_lineHeight;
	}

}

void Game::RenderEquippedList() const
{
	std::vector<ToolTip> euqippedTextList;
	ToolTip equipListTitle;
	equipListTitle.m_text = "Equipped Items: ";
	equipListTitle.m_color = Rgba(255, 255, 255, 255);
	equipListTitle.m_size = 1.3f;
	euqippedTextList.push_back(equipListTitle);

	Player* player = dynamic_cast<Player*>(m_world->m_currentPlayer);
	for (unsigned int index = 0; index < 6; ++index)
	{
		Item* item = player->m_equippedItems[index];
		if (item == nullptr)
			continue;

		ToolTip currentEquippedItemText;
		currentEquippedItemText.m_text = item->GetNameOfEquipType(item->m_type) + " : " + item->m_name;
		currentEquippedItemText.m_color = Rgba(255, 255, 255, 255);
		currentEquippedItemText.m_size = 1.0f;
		euqippedTextList.push_back(currentEquippedItemText);
	}

	int currentDrawHeight = m_font->m_lineHeight;
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	for (auto iterate = euqippedTextList.begin(); iterate != euqippedTextList.end(); ++iterate)
	{
		g_simpleRenderer->DrawTextWithFont(m_font, (float)DEFAULT_WINDOW_WIDTH * 0.75f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.05f) + m_font->m_size + currentDrawHeight), iterate->m_text, iterate->m_color, iterate->m_size);
		currentDrawHeight -= m_font->m_lineHeight;
	}
}

void Game::RenderInventoryList() const
{
	std::vector<ToolTip> itemTextList;
	ToolTip itemListTitle;
	itemListTitle.m_text = "Inventory: ";
	itemListTitle.m_color = Rgba(255, 255, 255, 255);
	itemListTitle.m_size = 1.0f;
	itemTextList.push_back(itemListTitle);

	Player* player = dynamic_cast<Player*>(m_world->m_currentPlayer);
	Inventory* inventory = player->m_inventory;
	for (unsigned int index = 0; index < inventory->m_itemList.size(); ++index)
	{
		if (itemTextList.size() + 1 >= inventory->m_maxItemCount)
			break;

		Item* item = inventory->m_itemList[index];
		if (item == nullptr)
			continue;

		char c = (char)(index + 65);
		std::string itemChoice;
		itemChoice.push_back(c);

		ToolTip currentItemText;
		currentItemText.m_text = itemChoice + " : " + item->m_name;
		if (player->IsItemEquiped(*item))
			currentItemText.m_color = Rgba(255, 205, 0, 255);
		else
			currentItemText.m_color = Rgba(255, 255, 255, 255);
		currentItemText.m_size = 0.7f;
		itemTextList.push_back(currentItemText);
	}

	int currentDrawHeight = m_font->m_lineHeight;
	g_simpleRenderer->SetShaderProgram(m_fontShader);
	for (auto iterate = itemTextList.begin(); iterate != itemTextList.end(); ++iterate)
	{
		g_simpleRenderer->DrawTextWithFont(m_font, (float)DEFAULT_WINDOW_WIDTH * 1.2f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.4f) + m_font->m_size + currentDrawHeight), iterate->m_text, iterate->m_color, iterate->m_size);
		currentDrawHeight -= m_font->m_lineHeight;
	}
}

void Game::RenderInstructionsForStatState() const
{
	int drawHeight = m_font->m_lineHeight;
	std::string string = "Instructions:";
	g_simpleRenderer->DrawTextWithFont(m_font, 35.0f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.41f) + m_font->m_size + drawHeight), string, Rgba(255, 255, 255, 255));
	drawHeight -= m_font->m_lineHeight;

	string = "Press the associated letter key to equip/unequip and item, and shift + ";
	g_simpleRenderer->DrawTextWithFont(m_font, 35.0f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.41f) + m_font->m_size + drawHeight), string, Rgba(255, 255, 255, 255));
	drawHeight -= m_font->m_lineHeight;

	string = "' That key ' to drop or use that item. Note that Equipped Items are yellow!";
	g_simpleRenderer->DrawTextWithFont(m_font, 35.0f, (float)(((float)DEFAULT_WINDOW_HEIGHT * 1.41f) + m_font->m_size + drawHeight), string, Rgba(255, 255, 255, 255));
}

void Game::Start2DDraw() const
{
	//g_theInputSystem->HideMouseCursor();
	g_simpleRenderer->SetRenderTarget(nullptr, nullptr);
	g_simpleRenderer->ClearColor(Rgba(0, 0, 0));
	IntVector2 dimensions = g_simpleRenderer->m_output->m_window->GetClientSize();
	g_simpleRenderer->SetViewport(0, 0, (unsigned int)dimensions.x, (unsigned int)dimensions.y);
	g_simpleRenderer->MakeModelMatrixIdentity();

	if (m_world != nullptr && m_currentState != MAIN_MENU)
	{
		g_simpleRenderer->MakeViewMatrixIdentity();
		if (m_world->m_currentPlayer != nullptr)
			g_simpleRenderer->SetViewMatrix(Vector3(0.0f, 0.0f, 0.0f), Vector3((float)m_world->m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_world->m_currentPlayer->GetPositionInMap().y + 0.5f, 0.0f));
	}
	else
		g_simpleRenderer->MakeViewMatrixIdentity();

	g_simpleRenderer->SetOrthoProjection(Vector2(WORLD_WIDTH * -0.5f, WORLD_HEIGHT * -0.5f), Vector2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f));
	g_simpleRenderer->SetConstantBuffer(1, m_constBuffer);
}

void Game::DrawConsole() const
{
	if (m_isConsoleActive)
	{
		g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
		g_simpleRenderer->EnableDepthTest(false);
		g_simpleRenderer->MakeViewMatrixIdentity();
		g_simpleRenderer->MakeModelMatrixIdentity();
		g_simpleRenderer->SetOrthoProjection(Vector2(0.0f, 0.0f), Vector2((float)WORLD_WIDTH, (float)WORLD_HEIGHT));
		g_simpleRenderer->SetShaderProgram(m_shadowBox);
		g_simpleRenderer->DrawMesh(*m_fontBackDrop);
		g_simpleRenderer->SetShaderProgram(m_fontShader);
		g_simpleRenderer->SetSampler(new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR));
		g_simpleRenderer->DrawTextWithFont(m_font, 20.0f, (float)(38 + m_font->m_size), m_inputText, Rgba(255, 255, 255, 255));
		DrawBlinkyCursor();

		unsigned int drawHeight = m_font->m_lineHeight;
		for (auto iterate = g_console->m_commandLog.rbegin(); iterate != g_console->m_commandLog.rend(); ++iterate)
		{
			g_simpleRenderer->DrawTextWithFont(m_font, 20.0f, (float)(38 + m_font->m_size + drawHeight), iterate->message, iterate->color);
			drawHeight += m_font->m_lineHeight;
		}
		g_simpleRenderer->DisableBlend();
	}
}

void Game::Draw2DGridAndAxis(float factorToDraw /*= 1.0f*/) const
{
	g_simpleRenderer->SetShaderProgram(m_defaultShader);
	g_simpleRenderer->SetTexture(m_defaultTex);
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	DrawGrid2D(factorToDraw);
	DrawAxis2D(factorToDraw);
	g_simpleRenderer->DisableBlend();
}

void Game::DrawGrid2D(float factorToDraw /*= 1.0f*/) const
{
	for (float distance = 0.0f; distance < WORLD_HEIGHT * 0.5f; distance += factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(-WORLD_WIDTH * 0.5f, distance, 0.0f), Vector3(WORLD_WIDTH * 0.5f, distance, 0.0f), Rgba(255, 255, 255, 128));
	}
	
	for (float distance = -1.0f * factorToDraw; distance > -WORLD_HEIGHT * 0.5f; distance -= factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(-WORLD_WIDTH * 0.5f, distance, 0.0f), Vector3(WORLD_WIDTH * 0.5f, distance, 0.0f), Rgba(255, 255, 255, 128));
	}

	for (float distance = 0.5f * factorToDraw; distance < WORLD_HEIGHT * 0.5f; distance += factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(-WORLD_WIDTH * 0.5f, distance, 0.0f), Vector3(WORLD_WIDTH * 0.5f, distance, 0.0f), Rgba(255, 255, 0, 64));
	}

	for (float distance = -0.5f * factorToDraw; distance > -WORLD_HEIGHT * 0.5f; distance -= factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(-WORLD_WIDTH * 0.5f, distance, 0.0f), Vector3(WORLD_WIDTH * 0.5f, distance, 0.0f), Rgba(255, 255, 0, 64));
	}
	
	for (float distance = 0.0f; distance < WORLD_WIDTH * 0.5f; distance += factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -WORLD_HEIGHT * 0.5f, 0.0f), Vector3(distance, WORLD_HEIGHT * 0.5f, 0.0f), Rgba(255, 255, 255, 128));
	}
	
	for (float distance = -1.0f * factorToDraw; distance > -WORLD_WIDTH * 0.5f; distance -= factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -WORLD_HEIGHT * 0.5f, 0.0f), Vector3(distance, WORLD_HEIGHT * 0.5f, 0.0f), Rgba(255, 255, 255, 128));
	}

	for (float distance = 0.5f * factorToDraw; distance < WORLD_WIDTH * 0.5f; distance += factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -WORLD_HEIGHT * 0.5f, 0.0f), Vector3(distance, WORLD_HEIGHT * 0.5f, 0.0f), Rgba(255, 255, 0, 64));
	}

	for (float distance = -0.5f * factorToDraw; distance > -WORLD_WIDTH * 0.5f; distance -= factorToDraw)
	{
		g_simpleRenderer->DrawLine(Vector3(distance, -WORLD_HEIGHT * 0.5f, 0.0f), Vector3(distance, WORLD_HEIGHT * 0.5f, 0.0f), Rgba(255, 255, 0, 64));
	}
}

void Game::DrawAxis2D(float factorToDraw /*= 1.0f*/) const
{
	g_simpleRenderer->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.25f * factorToDraw, 0.0f, 0.0f), Rgba(255, 0, 0, 255));
	g_simpleRenderer->DrawLine(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.25f * factorToDraw, 0.0f), Rgba(0, 255, 0, 255));
}

void Game::DrawBlinkyCursor() const 
{
	if (m_shouldBlink)
	{
		g_simpleRenderer->DrawTextWithFont(m_font, (float)(15 + m_font->GetTextWidth(m_inputText)), (float)(38 + m_font->m_size), "|", Rgba(255, 255, 255, 255));
	}
}

void Game::UpdateCursorBlink()
{
	if (m_cursorCounter < 4)
	{
		m_shouldBlink = true;
		if (m_cursorCounter == 0)
			m_cursorCounter = 10;
		else
			--m_cursorCounter;
	}
	else
	{
		m_shouldBlink = false;
		--m_cursorCounter;
	}
}

void Game::KeyUp(unsigned char keyThatWasJustReleased)
{
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

void Game::Initialize()
{
	m_timeConst.GAME_TIME = 0.0f;
	m_constBuffer = new ConstantBuffer(g_simpleRenderer->m_device, &m_timeConst, sizeof(m_timeConst));

	InitializeConsole();

	m_defaultTex = g_simpleRenderer->CreateOrGetTexture("Data/Images/blank.png");
	m_shadowBox = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/shadow_box.hlsl");
	m_fontShader = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/font_shader.hlsl");
	m_defaultShader = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/nop_textured.hlsl");
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTERMODE_ANISOTROPIC, FILTERMODE_ANISOTROPIC);
	
	InitializeTileRegistry();
	InitializeItemRegistry();
	InitializeLootList();
	InitializeCharacterRegistry();
	InitializeInteractableRegistry();
	InitializeMapList();
	InitializeAdventureList();
}

void Game::InitializeMapList()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/Maps.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlType = xmlRoot->FirstChildElement("Map"); xmlType != nullptr; xmlType = xmlType->NextSiblingElement("Map"))
	{
		MapDescription* desc = new MapDescription(*xmlType);
		MapDescription::s_mapList.insert_or_assign(desc->m_name, desc);
	}
}

void Game::InitializeConsole()
{
	m_font = g_simpleRenderer->CreateOrGetKerningFont("Data/Fonts/trebuchetMS32.fnt");
	g_console = new CommandSystem(m_font);
	g_console->RegisterCommand("clear", ConsoleClear);
	g_console->RegisterCommand("help", ConsoleHelp);
	g_console->RegisterCommand("quit", ConsoleQuit);

	m_fontBackDrop = new Mesh();
	m_fontBackDrop->CreateOneSidedQuad(Vector3((float)WORLD_WIDTH * 0.5f, (float)WORLD_HEIGHT * 0.01f, 0.0f), Vector3((float)WORLD_WIDTH * 0.5f, (float)WORLD_HEIGHT * 0.98f, 0.0f), Rgba(0, 0, 0, 128));
}

void Game::InitializeTileRegistry()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/TileDescriptions.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlTileDesc = xmlRoot->FirstChildElement("TileDescription"); xmlTileDesc != nullptr; xmlTileDesc = xmlTileDesc->NextSiblingElement())
	{
		TileDescription* tileDesc = new TileDescription(*xmlTileDesc);
		TileDescription::s_tileDefRegistry.insert_or_assign(tileDesc->m_name, tileDesc);
	}
}

void Game::InitializeCharacterRegistry()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/Entities.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlEntity = xmlRoot->FirstChildElement("Character"); xmlEntity != nullptr; xmlEntity = xmlEntity->NextSiblingElement())
	{
		CharacterDescription* charDesc = new CharacterDescription(*xmlEntity);
		CharacterDescription::s_characterDefRegistry.insert_or_assign(charDesc->m_name, charDesc);

		auto xmlBehavior = xmlEntity->FirstChildElement("Behavior");
		if (xmlBehavior == nullptr)
		{
			continue;
		}

		std::map<std::string, Behavior*> tempMap;
		for (auto xmlType = xmlBehavior->FirstChildElement(); xmlType != nullptr; xmlType = xmlType->NextSiblingElement())
		{
			Behavior* behavior = new Behavior();
			behavior = behavior->CreateBehavior(*xmlType);
			tempMap.insert_or_assign(xmlType->Name(), behavior);
		}
		Behavior::s_behaviorList.insert_or_assign(charDesc->m_name, tempMap);
	}
}

void Game::InitializeInteractableRegistry()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/Interactables.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlInteractable = xmlRoot->FirstChildElement("Interactable"); xmlInteractable != nullptr; xmlInteractable = xmlInteractable->NextSiblingElement())
	{
		InteractableDescription* interactDesc = new InteractableDescription(*xmlInteractable);
		InteractableDescription::s_interactableDefRegistry.insert_or_assign(interactDesc->m_name, interactDesc);
	}
}

void Game::InitializeItemRegistry()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/Items.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlItem = xmlRoot->FirstChildElement("Item"); xmlItem != nullptr; xmlItem = xmlItem->NextSiblingElement())
	{
		ItemDescription* itemDesc = new ItemDescription(*xmlItem);
		ItemDescription::s_itemDefRegistry.insert_or_assign(itemDesc->m_name, itemDesc);
	}
}

void Game::InitializeLootList()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/Loot.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlType = xmlRoot->FirstChildElement("Loot_Type"); xmlType != nullptr; xmlType = xmlType->NextSiblingElement("Loot_Type"))
	{
		std::map<std::string, int> tempMap;
		std::string npcName = ParseXmlAttribute(*xmlType, "type", std::string("ERROR_LOOT_TYPE_NOT_FOUND!"));
		for (auto xmlLoot = xmlType->FirstChildElement("Item"); xmlLoot != nullptr; xmlLoot = xmlLoot->NextSiblingElement("Item"))
		{
			std::string itemName = ParseXmlAttribute(*xmlLoot, "name", std::string("ERROR_ITEM_NAME_NOT_FOUND!"));
			IntVector2 weightRange = ParseXmlAttribute(*xmlLoot, "weight", IntVector2(0,0));
			int weightValue = GetRandomIntInRange(weightRange.x, weightRange.y);

			tempMap.insert_or_assign(itemName, weightValue);
		}
		NPC::s_lootList.insert_or_assign(npcName, tempMap);
	}
}

void Game::InitializeAdventureList()
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile("Data/XML/Adventure.xml");
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	for (auto xmlType = xmlRoot->FirstChildElement("Adventure"); xmlType != nullptr; xmlType = xmlType->NextSiblingElement("Adventure"))
	{
		Adventure* adventure = new Adventure(*xmlType);
		Adventure::s_adventureList.insert_or_assign(adventure->m_name, adventure);
	}
}

void Game::CheckForVictoryCondition()
{
	if (m_world->m_currentPlayer == nullptr)
		return;

	if (m_world->m_currentPlayer->m_inventory->m_itemList.empty() || m_victory)
		return;

	if(!m_currentAdventure->m_goalList.empty())
	{
		unsigned int goalSize = m_currentAdventure->m_goalList.size();
		std::vector<std::string> temp;
		for (unsigned int goalIndex = 0; goalIndex < m_currentAdventure->m_goalList.size(); ++goalIndex)
		{
			for (unsigned int itemIndex = 0; itemIndex < m_world->m_currentPlayer->m_inventory->m_itemList.size(); ++itemIndex)
			{
				//Search for goal items in inventory
				Item* item = m_world->m_currentPlayer->m_inventory->m_itemList[itemIndex];
				if (item->m_name == m_currentAdventure->m_goalList[goalIndex]) {
					temp.push_back(m_currentAdventure->m_goalList[goalIndex]);
					m_currentAdventure->m_goalList[goalIndex] = m_currentAdventure->m_goalList.back();
					m_currentAdventure->m_goalList.pop_back();
					--goalIndex;
				}
			}
		}
		temp.shrink_to_fit();

		if (temp.size() == goalSize)
			m_victory = true;
		else
			m_currentAdventure->m_goalList.insert(m_currentAdventure->m_goalList.end(), temp.begin(), temp.end());
	}

	if (!m_currentAdventure->m_killList.empty())
	{
		unsigned int killSize = m_currentAdventure->m_killList.size();
		std::vector<std::string> temp;
		Player* player = dynamic_cast<Player*>(m_world->m_currentPlayer);
		for (unsigned int killIndex = 0; killIndex < m_currentAdventure->m_killList.size(); ++killIndex)
		{
			for (unsigned int playerKills = 0; playerKills < player->m_killList.size(); ++playerKills)
			{
				std::string targetName = m_currentAdventure->m_killList[killIndex];
				std::string playerKillName = player->m_killList[playerKills];
				if (targetName == playerKillName)
				{
					temp.push_back(targetName);
					m_currentAdventure->m_killList[killIndex] = m_currentAdventure->m_killList.back();
					m_currentAdventure->m_killList.pop_back();
					--killIndex;
				}
			}
		}
		temp.shrink_to_fit();

		if (temp.size() == killSize)
			m_victory = true;
		else
			m_currentAdventure->m_killList.insert(m_currentAdventure->m_killList.end(), temp.begin(), temp.end());
	}
}

void Game::CheckForFailure()
{
	if (m_failure)
		return;

	if (m_world->m_currentPlayer == nullptr)
		m_failure = true;
}