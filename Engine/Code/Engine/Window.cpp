#pragma once
#include "Engine/Window.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Math/Vector2.hpp"

std::size_t Window::s_refCount = 0;

Window* GetWindowFromHWND(HWND hwnd)
{
	Window* wnd = (Window*) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
	return wnd;
}

static LRESULT CALLBACK GameWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//Do custom handling...
	Window* window = GetWindowFromHWND(hwnd);
	if (window != nullptr && window->m_customMessageHandler != nullptr)
	{
		bool stopProcess = window->m_customMessageHandler(hwnd, msg, wparam, lparam);
		if (stopProcess)
			return 0;
	}

	switch (msg) 
	{
	case WM_CREATE:
		{
			CREATESTRUCT *cp = (CREATESTRUCT*)lparam;
			window = (Window *)cp->lpCreateParams;
			window->m_hWnd = hwnd;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
		}
	}


	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void Window::RegisterWindowClass() 
{
	memset(&m_windowClassDescription, 0, sizeof(m_windowClassDescription));
	m_windowClassDescription.cbSize = sizeof(m_windowClassDescription);
	m_windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	m_windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(GameWndProc); // Assign a win32 message-handling function
	m_windowClassDescription.hInstance = GetModuleHandle(NULL);
	m_windowClassDescription.hIcon = NULL;
	m_windowClassDescription.hCursor = NULL;
	m_windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&m_windowClassDescription);
	++s_refCount;
}

void Window::SetWindowFlags(DWORD styleFlags, DWORD styleExFlags) 
{
	m_windowStyleFlags = styleFlags;
	m_windowStyleExFlags = styleExFlags;
	::AdjustWindowRectEx(&m_windowRect, m_windowStyleFlags, m_systemMenu, m_windowStyleExFlags);
}

AABB2D Window::GetClientRectangle()
{
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow(); //handle to desktop
	GetClientRect(desktopWindowHandle, &desktopRect); //Gets size

	Vector2 mins(-(float)desktopRect.left * 0.5f, -(float)desktopRect.bottom * 0.5f);
	Vector2 maxs((float)desktopRect.right * 0.5f, (float)desktopRect.top * 0.5f);

	return AABB2D(mins, maxs);
}

Window::Window()
	: m_windowTitle ("Application"),
		m_hWnd (nullptr),
		m_displayDeviceContext (nullptr),
		m_openGLRenderingContext (nullptr),
		m_customMessageHandler (nullptr),
		m_systemMenu(false)
{
	int OFFSET_FROM_WINDOWS_DESKTOP = 50;
	int WINDOW_PHYSICAL_WIDTH = 800;
	int WINDOW_PHYSICAL_HEIGHT = 450;

	if (s_refCount == 0) 
	{
		RegisterWindowClass();
	}

	m_windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED;
	m_windowStyleExFlags = WS_EX_APPWINDOW;

	GetClientRectangle();

	m_windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx(&m_windowRect, m_windowStyleFlags, m_systemMenu, m_windowStyleExFlags);
}

Window::Window(const std::string& appName, int offsetFromDesktop, int windowWidth, int windowHeight, DWORD styleFlags, DWORD styleExFlags, bool menu)
	: m_windowTitle(appName),
		m_hWnd (nullptr),
		m_displayDeviceContext (nullptr),
		m_openGLRenderingContext (nullptr),
		m_customMessageHandler (nullptr),
		m_systemMenu(menu)
{
	int OFFSET_FROM_WINDOWS_DESKTOP = offsetFromDesktop;
	int WINDOW_PHYSICAL_WIDTH = windowWidth;
	int WINDOW_PHYSICAL_HEIGHT = windowHeight;

	if (s_refCount == 0)
	{
		RegisterWindowClass();
	}
	SetWindowFlags(styleFlags, styleExFlags);

	GetClientRectangle();

	m_windowRect = { OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_WIDTH, OFFSET_FROM_WINDOWS_DESKTOP + WINDOW_PHYSICAL_HEIGHT };
	AdjustWindowRectEx(&m_windowRect, m_windowStyleFlags, m_systemMenu, m_windowStyleExFlags);
}

Window::~Window()
{
	Close();
	if (s_refCount != 0)
	{
		--s_refCount;
		if (s_refCount == 0)
		{
			UnregisterWindowClass();
		}
	}
}

void Window::UnregisterWindowClass() 
{
	UnregisterClass(m_windowClassDescription.lpszClassName, nullptr);
}

void Window::SetWindowHandler() 
{
	m_hWnd = CreateWindowExA(
		m_windowStyleExFlags,
		"Simple Window Class",
		&m_windowTitle[0],
		m_windowStyleFlags,
		m_windowRect.left,
		m_windowRect.top,
		m_windowRect.right - m_windowRect.left,
		m_windowRect.bottom - m_windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		this);

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	m_displayDeviceContext = GetDC(m_hWnd);
}

void Window::Open()
{
	SetWindowHandler();

	HCURSOR cursor = ::LoadCursor(NULL, IDC_ARROW);
	::SetCursor(cursor);

}

bool Window::IsOpen()
{
	return ::IsWindow(m_hWnd) != 0;
}

void Window::Close()
{
	::DestroyWindow(m_hWnd);
}

bool Window::IsClosed()
{
	return !IsOpen();
}

void Window::ProcessMessages()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, m_hWnd, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage);
	}
}

IntVector2 Window::GetClientSize()
{
	RECT window;
	::GetClientRect(m_hWnd, &window);
	int width = window.right - window.left;
	int height = window.bottom - window.top;
	m_windowRect = window;
	return IntVector2(width, height);
}

void Window::SetClientSize(const IntVector2& dimensions, const IntVector2& position)
{
	m_windowRect.left = position.x;
	m_windowRect.top = position.y;
	m_windowRect.bottom = m_windowRect.top + dimensions.y;
	m_windowRect.right = m_windowRect.left + dimensions.x;
	::AdjustWindowRectEx(&m_windowRect, m_windowStyleFlags, m_systemMenu, m_windowStyleExFlags);


}

void Window::SetWindowTitle(const std::string& newTitle) 
{
	m_windowTitle = newTitle;
	::SetWindowTextA(m_hWnd, m_windowTitle.c_str());
}