#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Input/Input.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/EngineConfig.hpp"


Input::Input()
	:m_doesAppHaveFocus(true)
	,m_hideMouseCursorWhenFocused(false)
	, m_mouseScrollIsDown(false)
	, m_mouseScrollWasDown(false)
	, m_mouseScrollIsUp(false)
	, m_mouseScrollWasUp(false)
{
	for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; controllerIndex++)
		m_controllers[controllerIndex].m_controllerID = controllerIndex;
}

Input::~Input()
{

}

void Input::Update()
{
	if (!DoesAppHaveFocus())
		Sleep(100);

	UpdateKeyboard();
	UpdateController();
	UpdateMouseScroll();
}

bool Input::IsKeyDown(int keyCode) const
{
	return m_keyStates[keyCode].m_isDown;
}

bool Input::WasKeyJustPressed(int keyCode) const
{
	return m_keyStates[keyCode].m_wasJustPressed;
}

bool Input::WasKeyJustReleased(int keyCode) const 
{
	return m_keyStates[keyCode].m_wasJustReleased;
}

void Input::OnKeyDown(int keyCode)
{
	m_keyStates[keyCode].m_isDown = true;
	m_keyStates[keyCode].m_wasJustPressed = true;
}

void Input::OnKeyUp(int keyCode)
{
	m_keyStates[keyCode].m_isDown = false;
	m_keyStates[keyCode].m_wasJustReleased = true;
}

Vector2 Input::GetMouseScreenCoords() const
{
	POINT cursorPosInt;
	GetCursorPos(&cursorPosInt);
	Vector2 cursorPosFloat((float)cursorPosInt.x, (float)cursorPosInt.y);
	return cursorPosFloat;
}

Vector2 Input::GetScreenSize()
{
	HWND desktopHandler = GetDesktopWindow();
	RECT desktopRect;
	GetWindowRect(desktopHandler, &desktopRect);

	Vector2 screenSize;
	screenSize.x = (float)(desktopRect.right - desktopRect.left);
	screenSize.y = (float)(desktopRect.bottom - desktopRect.top);
	return screenSize;
}

void Input::SetMouseCursorPosition(const Vector2& newMouseCoords)
{
	int x = (int)newMouseCoords.x;
	int y = (int)newMouseCoords.y;
	SetCursorPos(x, y);
}

void Input::OnAppGainFocus()
{
	m_doesAppHaveFocus = true;
	if (m_hideMouseCursorWhenFocused)
		HideMouseCursor();
}

void Input::OnAppLoseFocus()
{
	m_doesAppHaveFocus = false;
	if (!m_hideMouseCursorWhenFocused)
		ShowMouseCursor();
}

bool Input::DoesAppHaveFocus() const
{
	return m_doesAppHaveFocus;
}

void Input::HideMouseCursor()
{
	for (;;)
	{
		int mouseShowvalue = ShowCursor(FALSE);
		if (mouseShowvalue < 0)
			break;
	}
}

void Input::ShowMouseCursor()
{
	for (;;)
	{
		int mouseShowvalue = ShowCursor(TRUE);
		if (mouseShowvalue >= 0)
			break;
	}
}

void Input::SetMouseHiddenWhenFocused(bool hideMouseWhenNotFocused)
{
	m_hideMouseCursorWhenFocused = hideMouseWhenNotFocused;
	if (m_hideMouseCursorWhenFocused && m_doesAppHaveFocus)
		HideMouseCursor();
	else
		ShowMouseCursor();
}

bool Input::WasMouseWheelScrollingUp() const
{
	return m_mouseScrollIsUp && !m_mouseScrollWasUp;
}

bool Input::WasMouseWheelScrollingDown() const
{
	return m_mouseScrollIsDown && !m_mouseScrollWasDown;
}

bool Input::IsMouseWheelScrollingUp() const
{
	return m_mouseScrollIsUp;
}

bool Input::IsMouseWheelScrollingDown() const
{
	return m_mouseScrollIsDown;
}

void Input::MouseWheelInput(short wmParam)
{
	//m_mouseWheel = wmParam;
	//m_mouseScrollIsUp = IsMouseWheelScrollingUp();
	//m_mouseScrollIsDown = IsMouseWheelScrollingDown();
	if (wmParam > 0)
	{
		m_mouseScrollIsUp = true;
		m_mouseScrollIsDown = false;
	}
	else if (wmParam < 0)
	{
		m_mouseScrollIsUp = false;
		m_mouseScrollIsDown = true;
	}
}

void Input::UpdateMouseScroll()
{
	m_mouseScrollWasDown = m_mouseScrollIsDown;
	m_mouseScrollWasUp = m_mouseScrollIsUp;
	m_mouseScrollIsDown = false;
	m_mouseScrollIsUp = false;
}

void Input::UpdateKeyboard()
{
	for (int keyIndex = 0; keyIndex < NUM_KEYS; keyIndex++)
	{
		m_keyStates[keyIndex].m_wasJustPressed = false;
		m_keyStates[keyIndex].m_wasJustReleased = false;
	}
}

void Input::UpdateController()
{
	for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; controllerIndex++)
	{
		m_controllers[controllerIndex].Update();
	}
}

Vector2 Input::GetCursorNormalizedPosition(float worldWidth, float worldHeight, SimpleRenderer& renderer)
{
	POINT cursorPosInt;
	GetCursorPos(&cursorPosInt);
	ScreenToClient(renderer.m_output->m_window->m_hWnd, &cursorPosInt);
	Vector2 cursorInScreenSpace((float)cursorPosInt.x, (float)cursorPosInt.y);

	IntVector2 screenSize = renderer.m_output->m_window->GetClientSize();

	float xInOrtho = RangeMapFloat(0.0f, (float)screenSize.x, 0.0f, worldWidth, cursorInScreenSpace.x);
	float yInOrtho = RangeMapFloat(0.0f, (float)screenSize.y, 0.0f, worldHeight, cursorInScreenSpace.y);

	return Vector2(xInOrtho, -yInOrtho);
}