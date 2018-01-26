#pragma once
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/AABB2D.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

class Window {
public:
	typedef bool(*WindowMessageHandlerCb)(HWND, UINT, WPARAM, LPARAM);	
	WindowMessageHandlerCb m_customMessageHandler;
	HWND m_hWnd;
	HDC m_displayDeviceContext;
	HGLRC m_openGLRenderingContext;
	DWORD m_windowStyleFlags;
	DWORD m_windowStyleExFlags;
	std::string m_windowTitle;
	RECT m_windowRect;
	WNDCLASSEX m_windowClassDescription;
	static std::size_t s_refCount;
	bool m_systemMenu;

	
	
	Window(); //Increment RefCounted Register of Window Class (fancy!)
	Window(const std::string& appName, int offsetFromDesktop, int windowWidth, int windowHeight, DWORD styleFlags, DWORD styleExFlags, bool menu);
	~Window(); //Decrement RefCount - Deregister of Window Class

	void UnregisterWindowClass();
	void SetWindowHandler();
	void RegisterWindowClass();
	void SetWindowFlags(DWORD styleFlags, DWORD styleExFlags);
	AABB2D GetClientRectangle();
	void Open();

	bool IsOpen(); // ::IsWindow(hwnd)

	void Close();
	bool IsClosed();

	void ProcessMessages(); //Normal message loop

	IntVector2 GetClientSize();
	void SetClientSize(const IntVector2& dimensions, const IntVector2& position);

	void SetWindowTitle(const std::string& newTitle);
	void RegisterCustomMessageHandler(WindowMessageHandlerCb cb) { m_customMessageHandler = cb; }
};