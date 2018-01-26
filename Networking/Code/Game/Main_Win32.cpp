#include "Game/App.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Window.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Config.hpp"
#include "Engine/RHI/RHI.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Network/TCPSession.hpp"
#include "Engine/Network/NetConnection.hpp"
#include "Engine/Network/Net.hpp"
#include "Game/Game.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include <string>
#include <stdint.h>

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);

//-----------------------------------------------------------------------------------------------
bool WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	UNUSED(lParam);
	UNUSED(windowHandle);

	unsigned char asKey = (unsigned char)wParam;
	switch (wmMessageCode)
	{
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_QUIT:
	{
		g_theApp->OnExitRequested();
		return true;
	}
	case WM_KEYDOWN:
	{
		g_theApp->OnKeyDown(asKey);
		return true;
	}
	case WM_CHAR:
	{
		g_theApp->CaptureConsole(asKey);
		return true;
	}
	case WM_KEYUP:
	{
		g_theApp->OnKeyUp(asKey);
		return true;
	}
	case WM_MOUSEWHEEL:
	{
		g_theInputSystem->MouseWheelInput(HIWORD(wParam));
		return true;
	}
	}

	return false;
}

//-----------------------------------------------------------------------------------------------
void Initialize(HINSTANCE applicationInstanceHandle)
{
	g_config->ConfigList();
	UNUSED(applicationInstanceHandle);
	SetProcessDPIAware();

	bool net_startup = NetSystemStartup();
	ASSERT_RECOVERABLE(net_startup, "Failed to Start Network System!");

	g_theApp = new App();

	unsigned int width;
	unsigned int height;
	std::string title = "";
	g_config->ConfigGetUInt(width, "window_res_x");
	g_config->ConfigGetUInt(height, "window_res_y");
	g_config->ConfigGetString(title, "window_title");
	g_simpleRenderer->Setup(width, height, title);

	g_simpleRenderer->m_output->m_window->RegisterCustomMessageHandler(WindowsMessageHandlingProcedure);

	g_theApp->Initialize(g_simpleRenderer->m_output);
}


//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	delete g_config;

	delete g_theApp;
	g_theApp = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	UNUSED(commandLineString);
	g_config = new Config("Data/Config/a01.config");
	Initialize(applicationInstanceHandle);

	while (!g_theApp->IsQuitting())
	{
		g_theApp->RunFrame();
		g_simpleRenderer->Present();
	}

	if (g_theGame->m_gameSession->AmIClient())
	{
		NetMessage msg(LEAVE);
		msg.m_sender = g_theGame->m_gameSession->m_myConnection;
		msg.write_bytes(&g_theGame->m_gameSession->m_myConnection->m_connectionIndex, sizeof(uint8_t));
		g_theGame->m_gameSession->SendMessageToOthers(msg);
	}

	g_theGame->m_gameSession->Update();

	Shutdown();
	return 0;
}

