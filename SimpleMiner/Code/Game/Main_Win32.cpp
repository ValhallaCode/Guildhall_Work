#include "Game/App.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Input/Input.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include <string>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//-----------------------------------------------------------------------------------------------
#define UNUSED(x) (void)(x);


//-----------------------------------------------------------------------------------------------
const int OFFSET_FROM_WINDOWS_DESKTOP = 50;
const int WINDOW_PHYSICAL_WIDTH = SCREEN_WIDTH;
const int WINDOW_PHYSICAL_HEIGHT = SCREEN_HEIGHT;


//-----------------------------------------------------------------------------------------------

HWND g_hWnd = nullptr;
HDC g_displayDeviceContext = nullptr;
HGLRC g_openGLRenderingContext = nullptr;
const char* APP_NAME = "SimpleMiner By Dylan Fansler";


//-----------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	unsigned char asKey = (unsigned char)wParam;
	short highKey = HIWORD(wParam);
	switch (wmMessageCode)
	{
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
		{
			g_theApp->OnExitRequested();
			return 0;
		}

		case WM_KEYDOWN:
		{

			g_theApp->OnKeyDown(asKey);
			break;
		}

		case WM_KEYUP:
		{
			g_theApp->OnKeyUp(asKey);
			break;
		}

		case WM_LBUTTONUP:
		{
			if (asKey == 0x00)
				asKey = 0x01;

			g_theApp->OnKeyUp(asKey);
			break;
		}

		case WM_LBUTTONDOWN:
		{
			g_theApp->OnKeyDown(asKey);
			break;
		}

		case WM_RBUTTONUP:
		{
			if (asKey == 0x00)
				asKey = 0x02;

			g_theApp->OnKeyUp(asKey);
			break;
		}

		case WM_RBUTTONDOWN:
		{
			g_theApp->OnKeyDown(asKey);
			break;
		}

		case WM_MOUSEWHEEL:
		{
			g_theInputSystem->MouseWheelInput(highKey);
			break;
		}

		case WM_SETFOCUS:
		{
			if (g_theInputSystem)
				g_theInputSystem->OnAppGainFocus();
			break;
		}

		case WM_KILLFOCUS:
		{
			if (g_theInputSystem)
				g_theInputSystem->OnAppLoseFocus();
			break;
		}
	}
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


//-----------------------------------------------------------------------------------------------
void CreateOpenGLWindow(HINSTANCE applicationInstanceHandle)
{
	// Define a window class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Assign a win32 message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect(desktopWindowHandle, &desktopRect);

	RECT windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, APP_NAME, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	g_hWnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		applicationInstanceHandle,
		NULL);

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);

	g_displayDeviceContext = GetDC(g_hWnd);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);

	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	int pixelFormatCode = ChoosePixelFormat(g_displayDeviceContext, &pixelFormatDescriptor);
	SetPixelFormat(g_displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
	g_openGLRenderingContext = wglCreateContext(g_displayDeviceContext);
	wglMakeCurrent(g_displayDeviceContext, g_openGLRenderingContext);

	glEnable(GL_BLEND); // Enable pixel blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Use alpha (translucency) blending
	glLineWidth(2.f); // Use thicker (2px thick) lines
	glEnable(GL_LINE_SMOOTH); // Use smooth (anti-aliased) lines
}

//-----------------------------------------------------------------------------------------------
void Initialize(HINSTANCE applicationInstanceHandle)
{
	SetProcessDPIAware();
	CreateOpenGLWindow(applicationInstanceHandle);
	g_theApp = new App();
}


//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	delete g_theApp;
	g_theApp = nullptr;
}


//-----------------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int)
{
	UNUSED(commandLineString);
	Initialize(applicationInstanceHandle);

	while (!g_theApp->IsQuitting())
	{
		g_theApp->RunFrame();
		SwapBuffers(g_displayDeviceContext);
	}

	Shutdown();

	//_CrtDumpMemoryLeaks();

	return 0;
}

