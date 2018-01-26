#include "Game/App.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Network/TCPSession.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/NetObject.hpp"
#include "Game/Player.hpp"


const float MIN_FRAMES_PER_SECOND = 10.f; // Game time will slow down beneath this framerate
										  //const float TARGET_FRAMES_PER_SECOND = 60.f;
										  //const float TARGET_SECONDS_PER_FRAME = (1.f / TARGET_FRAMES_PER_SECOND);
const float MAX_FRAMES_PER_SECOND = 60.f;
const float MIN_SECONDS_PER_FRAME = (1.f / MAX_FRAMES_PER_SECOND);
const float MAX_SECONDS_PER_FRAME = (1.f / MIN_FRAMES_PER_SECOND);

App::App()
	:m_output(nullptr)
	, m_isQuitting(false)
	, m_currentState(MAIN_MENU)
	, m_mainMenuSelectIndex(0)
	, m_infoMenuSelectIndex(0)
	, m_tryingToConnect(false)
	, m_username("Generic Player")
{
	net_address_t default_addr = GetMyAddress(GAME_PORT);
	m_addrInput = NetAddressToString(default_addr);

	g_theAudioSystem = new AudioSystem();
	g_simpleRenderer = new SimpleRenderer();
	g_theInputSystem = new Input();
	g_theGame = new Game();

	m_font = CreateOrGetKerningFont("Data/Fonts/trebuchetMS32.fnt");
}

App::~App()
{
	delete g_theGame;
	g_theGame = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_simpleRenderer;
	g_simpleRenderer = nullptr;

	delete g_theAudioSystem;
	g_theAudioSystem = nullptr;

	delete g_console;
	g_console = nullptr;

	m_output = nullptr;
}

void App::Update(float deltaSeconds)
{
	if (g_theAudioSystem != nullptr)
		g_theAudioSystem->Update();

	if (g_console != nullptr)
		g_console->Update(deltaSeconds);

	if (m_currentState == MAIN_MENU && !g_console->IsActive())
		UpdateMainMenu();

	if (m_currentState == INFO_MENU && !g_console->IsActive())
		UpdateInfoMenu(); 

	if (g_theGame->m_gameSession && m_tryingToConnect)
		g_theGame->UpdateGameSession();


	if (g_theGame != nullptr && m_currentState == GAME)
		g_theGame->Update(deltaSeconds);


	if (g_theInputSystem != nullptr)
		g_theInputSystem->Update();
}

void App::Render()
{
	if (g_theGame != nullptr)
		g_theGame->Render();

	if (m_currentState == MAIN_MENU)
		RenderMainMenu();

	if (m_currentState == INFO_MENU)
		RenderInfoMenu();

	g_theGame->DrawConsole();
}

float App::CalculateDeltaSeconds()
{
	double timeNow = GetCurrentTimeSeconds();
	static double lastFrameTime = timeNow;
	double deltaSeconds = timeNow - lastFrameTime;

	// Wait until [nearly] the minimum frame time has elapsed (limit framerate to within the max)
	while (deltaSeconds < MIN_SECONDS_PER_FRAME * .999f)
	{
		timeNow = GetCurrentTimeSeconds();
		deltaSeconds = timeNow - lastFrameTime;
	}
	lastFrameTime = timeNow;

	// Clamp deltaSeconds to be within the max time allowed (e.g. sitting at a debug break point)
	if (deltaSeconds > MAX_SECONDS_PER_FRAME)
	{
		deltaSeconds = MAX_SECONDS_PER_FRAME;
	}

	return (float)deltaSeconds;
}

void App::RunFrame()
{
	g_simpleRenderer->ProcessMessages();
	//---------WinMain32^^^
	//BeginFrame()
	float deltaSeconds = CalculateDeltaSeconds();
	Update(deltaSeconds);
	Render();
	//EndFrame()
	//Subsystem calls for each
}

bool App::IsQuitting() const
{
	return m_isQuitting;
}

void App::OnExitRequested()
{
	m_isQuitting = true;
}

void App::OnKeyDown(unsigned char keyCode)
{
	g_theInputSystem->OnKeyDown(keyCode);
}

void App::CaptureConsole(unsigned char keyCode)
{
	if (keyCode == '`' || keyCode == '~' || keyCode == KEY_ENTER || keyCode == KEY_BACK || keyCode == KEY_ESCAPE)
		return;

	if (g_console->IsActive() && !(keyCode == KEY_TILDE))
		g_console->AddToInput(keyCode);

	if (m_currentState == INFO_MENU && !g_console->IsActive())
	{
		if (m_infoMenuSelectIndex == 0)
		{
			m_username.push_back(keyCode);
		}
		if (m_infoMenuSelectIndex == 1)
		{
			m_addrInput.push_back(keyCode);
		}
	}
}

void App::OnKeyUp(unsigned char keyCode)
{
	if (keyCode == KEY_ESCAPE && !g_console->IsActive())
	{

		if (g_theGame->m_gameSession->AmIClient())
		{
			NetMessage msg(LEAVE);
			msg.m_sender = g_theGame->m_gameSession->m_myConnection;
			msg.write_bytes(&g_theGame->m_gameSession->m_myConnection->m_connectionIndex, sizeof(uint8_t));
			g_theGame->m_gameSession->SendMessageToOthers(msg);
		}

		if (m_currentState != MAIN_MENU) 
		{
			g_theApp->m_currentState = MAIN_MENU;
			g_theGame->m_camera->m_position = Vector3(0.0f, 0.0f, 0.0f);
			g_theApp->m_infoMenuSelectIndex = 0;
			g_theApp->m_tryingToConnect = false;
			delete g_theGame->m_gameSession;
			g_theGame->m_gameSession = new TCPSession();
			RegisterNetObjectSession(g_theGame->m_gameSession);
			ClearNetObjectList();
			g_theGame->m_playerList.clear();
			g_theGame->m_ships.clear();
			g_theGame->m_playerList.resize(g_theGame->m_gameSession->m_maxConnectionCount + 1);
			g_theGame->m_ships.resize(g_theGame->m_gameSession->m_maxConnectionCount + 1);
			g_theGame->SetUpGameMessages();
		}
		else
			OnExitRequested();

		return;
	}

	if (keyCode == KEY_TILDE || (keyCode == KEY_ESCAPE && g_console->IsInputEmpty()))
	{
		g_console->ToggleActivity();
		g_console->ClearHistory();
	}

	if (keyCode == KEY_ESCAPE)
	{
		g_console->ClearInput();
	}

	if (keyCode == KEY_ENTER && g_console->IsActive())
	{
		g_console->Execute();
		g_console->ClearInput();
	}

	g_theInputSystem->OnKeyUp(keyCode);
}

void App::Initialize(RHIOutput* output)
{
	m_output = output;
	g_theGame->Initialize();
}

void App::RenderMainMenu() const
{
	g_simpleRenderer->SetRenderTarget(nullptr, nullptr);
	g_simpleRenderer->ClearColor(Rgba(0, 0, 0, 255));
	IntVector2 dimensions = g_simpleRenderer->m_output->m_window->GetClientSize();
	g_simpleRenderer->SetViewport(0, 0, (unsigned int)dimensions.x, (unsigned int)dimensions.y);
	g_simpleRenderer->MakeModelMatrixIdentity();
	g_simpleRenderer->SetOrthoProjection(Vector2(WORLD_WIDTH * -0.5f, WORLD_HEIGHT * -0.5f), Vector2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f));
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->EnableDepthTest(false);
	g_simpleRenderer->SetSampler(new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR));

	g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("PEW CUBED", 3.0f) * 0.5f), (WORLD_WIDTH * 0.5f) * 0.25f, "PEW CUBED", Rgba(255, 255, 255, 255), 3.0f);

	float height = 0.0f;
	Rgba selected_color = Rgba(255, 255, 0, 255);
	Rgba non_selected_color = Rgba(255, 255, 255, 255);

	if(m_mainMenuSelectIndex == 0)
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("HOST", 2.1f) * 0.5f), height, "HOST", selected_color, 2.1f);
	else
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("HOST", 2.0f) * 0.5f), height, "HOST", non_selected_color, 2.0f);

	height -= m_font->GetTextHeight("HOST", 2.0f) + 15.0f;

	if (m_mainMenuSelectIndex == 1)
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("JOIN", 2.1f) * 0.5f), height, "JOIN", selected_color, 2.1f);
	else
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("JOIN", 2.0f) * 0.5f), height, "JOIN", non_selected_color, 2.0f);

	height -= m_font->GetTextHeight("JOIN", 2.0f) + 15.0f;

	if (m_mainMenuSelectIndex == 2)
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("QUIT", 2.1f) * 0.5f), height, "QUIT", selected_color, 2.1f);
	else
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("QUIT", 2.0f) * 0.5f), height, "QUIT", non_selected_color, 2.0f);

}

void App::UpdateMainMenu()
{
	if (g_theInputSystem->WasKeyJustPressed(KEY_UPARROW))
	{
		--m_mainMenuSelectIndex;
		m_mainMenuSelectIndex = ClampWithin(m_mainMenuSelectIndex, 2, 0);
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_DOWNARROW))
	{
		++m_mainMenuSelectIndex;
		m_mainMenuSelectIndex = ClampWithin(m_mainMenuSelectIndex, 2, 0);
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ENTER))
	{
		switch (m_mainMenuSelectIndex)
		{
			case 0:
			{
				// Host Logic
				EstablishUPNPConnection("Asteroid UPnP", 8912, 8912);
				m_currentState = GAME;
				m_tryingToConnect = true;
				break;
			}
			case 1:
			{
				// Join Logic
				m_currentState = INFO_MENU;
				break;
			}
			case 2:
			{
				m_isQuitting = true;
				break;
			}
			default: break;
		}
	}
}

void App::RenderInfoMenu() const 
{
	g_simpleRenderer->SetRenderTarget(nullptr, nullptr);
	g_simpleRenderer->ClearColor(Rgba(0, 0, 64, 255));
	IntVector2 dimensions = g_simpleRenderer->m_output->m_window->GetClientSize();
	g_simpleRenderer->SetViewport(0, 0, (unsigned int)dimensions.x, (unsigned int)dimensions.y);
	g_simpleRenderer->MakeModelMatrixIdentity();
	g_simpleRenderer->SetOrthoProjection(Vector2(WORLD_WIDTH * -0.5f, WORLD_HEIGHT * -0.5f), Vector2(WORLD_WIDTH * 0.5f, WORLD_HEIGHT * 0.5f));
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->EnableDepthTest(false);
	g_simpleRenderer->SetSampler(new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR));


	if (m_mainMenuSelectIndex == 0)
	{
		// Hosting Render
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("HOST", 3.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * 0.75f, "HOST", Rgba(255, 255, 0, 255), 3.0f);
	}
	else if (m_mainMenuSelectIndex == 1) 
	{
		//Client Render
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("JOIN", 3.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * 0.75f, "JOIN", Rgba(255, 255, 0, 255), 3.0f);
	}

	if(m_infoMenuSelectIndex == 0)
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("USERNAME", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * 0.5f, "USERNAME", Rgba(255, 255, 0, 255), 2.0f);
	else
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("USERNAME", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * 0.5f, "USERNAME", Rgba(255, 255, 255, 255), 2.0f);

	Mesh user_input_box;

	float user_input_box_width = (WORLD_WIDTH * 0.5f) - 50.0f;
	float user_input_box_height = m_font->GetTextHeight("T", 1.75f) + 10.0f;
	float user_input_box_center_y = ((WORLD_HEIGHT * 0.5f) * 0.5f) + (-(float)(2 * m_font->GetTextHeight("USERNAME", 2.0f)) - (user_input_box_height * 0.5f)) - 10.0f;
	user_input_box.CreateTwoSidedQuad(Vector3(0.0f, user_input_box_center_y, 0.0f), Vector3(user_input_box_width, user_input_box_height, 0.0f), Rgba(0, 64, 255, 255));

	g_simpleRenderer->SetDiffuse(g_simpleRenderer->m_whiteTexture);
	g_simpleRenderer->DrawMesh(user_input_box);

	float user_input_height = user_input_box_center_y + ((float)m_font->GetTextHeight("T", 1.75f) * 0.5f) + 0.5f;
	g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth(m_username, 1.75f) * 0.5f), user_input_height, m_username, Rgba(0, 0, 0, 255), 1.75f);

	if(m_infoMenuSelectIndex == 0)
		g_simpleRenderer->DrawTextWithFont(m_font, (m_font->GetTextWidth(m_username, 1.75f) * 0.5f), user_input_height, "|", Rgba(0, 0, 0, 255), 1.75f);

	if(m_mainMenuSelectIndex == 1)
	{
		if (m_infoMenuSelectIndex == 1)
			g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("HOST ADDRESS", 2.0f) * 0.5f), 0.0f, "HOST ADDRESS", Rgba(255, 255, 0, 255), 2.0f);
		else
			g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("HOST ADDRESS", 2.0f) * 0.5f), 0.0f, "HOST ADDRESS", Rgba(255, 255, 255, 255), 2.0f);

		Mesh addr_input_box;

		float addr_input_box_width = (WORLD_WIDTH * 0.5f) - 50.0f;
		float addr_input_box_height = m_font->GetTextHeight("T", 1.75f) + 10.0f;
		float addr_input_box_center_y = (-(float)(2 * m_font->GetTextHeight("HOST ADDRESS", 2.0f)) - (addr_input_box_height * 0.5f)) - 10.0f;
		addr_input_box.CreateTwoSidedQuad(Vector3(0.0f, addr_input_box_center_y, 0.0f), Vector3(addr_input_box_width, addr_input_box_height, 0.0f), Rgba(0, 64, 255, 255));

		g_simpleRenderer->SetDiffuse(g_simpleRenderer->m_whiteTexture);
		g_simpleRenderer->DrawMesh(addr_input_box);

		float addr_input_height = addr_input_box_center_y + ((float)m_font->GetTextHeight("T", 1.75f) * 0.5f) + 0.5f;
		g_simpleRenderer->DrawTextWithFont(m_font, -((float)m_font->GetTextWidth(m_addrInput, 1.75f) * 0.5f), addr_input_height, m_addrInput, Rgba(0, 0, 0, 255), 1.75f);

		if(m_infoMenuSelectIndex == 1)
			g_simpleRenderer->DrawTextWithFont(m_font, ((float)m_font->GetTextWidth(m_addrInput, 1.75f) * 0.5f), addr_input_height, "|", Rgba(0, 0, 0, 255), 1.75f);
	}

	if (m_mainMenuSelectIndex == 0)
	{
		// Hosting Render
		if(m_infoMenuSelectIndex == 2)
			g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("START", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * -0.45f, "START", Rgba(255, 255, 0, 255), 2.0f);
		else
			g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("START", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * -0.45f, "START", Rgba(255, 255, 255, 255), 2.0f);
	}
	else if (m_mainMenuSelectIndex == 1)
	{
		//Client Render
		if (m_infoMenuSelectIndex == 2)
			g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("JOIN", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * -0.45f, "JOIN", Rgba(255, 255, 0, 255), 2.0f);
		else
			g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("JOIN", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * -0.45f, "JOIN", Rgba(255, 255, 255, 255), 2.0f);
	}
	
	if (m_infoMenuSelectIndex == 3)
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("BACK", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * -0.65f, "BACK", Rgba(255, 255, 0, 255), 2.0f);
	else
		g_simpleRenderer->DrawTextWithFont(m_font, -(m_font->GetTextWidth("BACK", 2.0f) * 0.5f), (WORLD_HEIGHT * 0.5f) * -0.65f, "BACK", Rgba(255, 255, 255, 255), 2.0f);
}

void App::UpdateInfoMenu()
{
	if (g_theInputSystem->WasKeyJustPressed(KEY_UPARROW))
	{
		--m_infoMenuSelectIndex;
		m_infoMenuSelectIndex = ClampWithin(m_infoMenuSelectIndex, 3, 0);

		if (m_mainMenuSelectIndex == 0 && m_infoMenuSelectIndex == 1)
		{
			--m_infoMenuSelectIndex;
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_DOWNARROW))
	{
		++m_infoMenuSelectIndex;
		m_infoMenuSelectIndex = ClampWithin(m_infoMenuSelectIndex, 3, 0);

		if (m_mainMenuSelectIndex == 0 && m_infoMenuSelectIndex == 1)
		{
			++m_infoMenuSelectIndex;
		}
	}

	if (m_currentState == INFO_MENU && !g_console->IsActive() && !m_tryingToConnect)
	{
		if (m_infoMenuSelectIndex == 0)
		{
			if (g_theInputSystem->WasKeyJustPressed(KEY_BACK) && !m_username.empty())
			{
				m_username.pop_back();
			}
		}
		if (m_infoMenuSelectIndex == 1)
		{
			if (g_theInputSystem->WasKeyJustPressed(KEY_BACK) && !m_addrInput.empty())
			{
				m_addrInput.pop_back();
			}
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ENTER))
	{
		switch (m_infoMenuSelectIndex)
		{
		case 2:
		{
			// Join / Start
			if (m_mainMenuSelectIndex == 0)
			{
				// Host
				m_currentState = GAME;
				m_tryingToConnect = true;
			}
			else
			{
				// Client
				net_address_t join_addr = StringToNetAddress(m_addrInput);
				g_theGame->JoinGame(join_addr);
				m_tryingToConnect = true;
			}
			break;
		}
		case 3:
		{
			m_currentState = MAIN_MENU;
			m_infoMenuSelectIndex = 0;
			break;
		}
		default: break;
		}
	}

	if (g_theGame->m_gameSession->m_state == SESSION_CONNECTED)
	{
		// Sync names
		m_currentState = GAME;
		g_theGame->SendName(m_username);

		uint8_t index = g_theGame->m_gameSession->m_myConnection->m_connectionIndex;
		// Make player
		g_theGame->m_playerList[index] = new Player(m_username, index);
		g_theGame->m_myPlayer = g_theGame->m_playerList[index];
	}

	if (g_theGame->m_gameSession->m_state == SESSION_DISCONNECTED && m_tryingToConnect)
	{
		// Sync names
		m_addrInput.clear();
		m_username.clear();
		m_tryingToConnect = false;
	}
}





