#include "Game/App.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"


const float MIN_FRAMES_PER_SECOND = 10.f; // Game time will slow down beneath this framerate
										  //const float TARGET_FRAMES_PER_SECOND = 60.f;
										  //const float TARGET_SECONDS_PER_FRAME = (1.f / TARGET_FRAMES_PER_SECOND);
const float MAX_FRAMES_PER_SECOND = 60.f;
const float MIN_SECONDS_PER_FRAME = (1.f / MAX_FRAMES_PER_SECOND);
const float MAX_SECONDS_PER_FRAME = (1.f / MIN_FRAMES_PER_SECOND);

App::App()
	:m_output(nullptr)
	, m_isQuitting(false)
{
	g_theAudioSystem = new AudioSystem();
	g_simpleRenderer = new SimpleRenderer();
	g_theInputSystem = new Input();
	g_theGame = new Game();
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

	if (g_theInputSystem != nullptr)
		g_theInputSystem->Update();

	if( g_simpleRenderer )
		g_simpleRenderer->ProcessMessages();

	if (g_theGame != nullptr)
		g_theGame->Update(deltaSeconds);
}

void App::Render()
{
	if (g_theGame != nullptr)
		g_theGame->Render();
}

//-----------------------------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
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

	if (g_theGame->m_isConsoleActive && !(keyCode == KEY_TILDE))
		g_theGame->m_inputText.push_back(keyCode);
}

void App::OnKeyUp(unsigned char keyCode)
{
	g_theInputSystem->OnKeyUp(keyCode);
}

void App::Initialize(RHIOutput* output)
{
	m_output = output;
	g_theGame->Initialize();
}