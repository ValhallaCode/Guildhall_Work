#include "Game/App.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Audio/AudioSystem.hpp"



App* g_theApp = nullptr;
const float MIN_FRAMES_PER_SECOND = 10.f; // Game time will slow down beneath this framerate
										  //const float TARGET_FRAMES_PER_SECOND = 60.f;
										  //const float TARGET_SECONDS_PER_FRAME = (1.f / TARGET_FRAMES_PER_SECOND);
const float MAX_FRAMES_PER_SECOND = 60.f;
const float MIN_SECONDS_PER_FRAME = (1.f / MAX_FRAMES_PER_SECOND);
const float MAX_SECONDS_PER_FRAME = (1.f / MIN_FRAMES_PER_SECOND);

App::App()
{
	g_theAudioSystem = new AudioSystem();
	g_theInputSystem = new Input();
	g_myRenderer = new Renderer();
	g_theGame = new Game();
}

App::~App()
{
	delete g_theGame;
	g_theGame = nullptr;

	delete g_myRenderer;
	g_myRenderer = nullptr;

	delete g_theInputSystem;
	g_theInputSystem = nullptr;

	delete g_theAudioSystem;
	g_theAudioSystem = nullptr;
}

void App::Update(float deltaSeconds)
{
	if (g_theAudioSystem != nullptr)
		g_theAudioSystem->Update();

	if (g_theInputSystem != nullptr)
		g_theInputSystem->Update();

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
		SleepSeconds(0.001f);
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
	float deltaSeconds = CalculateDeltaSeconds();
	Update(deltaSeconds);
	Render();
}

bool App::IsQuitting() const
{
	return g_theGame->m_isQuitting;
}

void App::OnExitRequested()
{
	g_theGame->m_isQuitting = true;
}

void App::OnKeyDown(unsigned char keyCode)
{
	g_theGame->KeyDown(keyCode);
	g_theInputSystem->OnKeyDown(keyCode);
}

void App::OnKeyUp(unsigned char keyCode)
{
	g_theGame->KeyUp(keyCode);
	g_theInputSystem->OnKeyUp(keyCode);
}
