#pragma once
#include "Engine/Input/XboxController.hpp"

class SimpleRenderer;

const int NUM_KEYS = 260;
const int MAX_CONTROLLERS = 4;
const int NUM_MOUSE_BUTTONS = 4;

enum KeyCode
{
	KEY_ESCAPE = 0x1B,
	KEY_UPARROW = 0x26,
	KEY_DOWNARROW = 0x28,
	KEY_LEFTARROW = 0x25,
	KEY_RIGHTARROW = 0x27,
	KEY_SPACEBAR = 0x20,
	KEY_BACK = 0x08,
	KEY_ENTER = 0x0D,
	KEY_CONTROL = 0x11,
	KEY_TAB = 0x09,
	KEY_SHIFT = 0x10,
	KEY_F1 = 0x70,
	KEY_F2 = 0x71,
	KEY_F3 = 0x72,
	KEY_F4 = 0x73,
	KEY_F5 = 0x74,
	KEY_F6 = 0x75,
	KEY_F7 = 0x76,
	KEY_F8 = 0x77,
	KEY_F9 = 0x78,
	KEY_F10 = 0x79,
	KEY_F11 = 0x7A,
	KEY_F12 = 0x7B,
	LEFT_MOUSE = 0x01,
	RIGHT_MOUSE = 0x02,
	KEY_TILDE = 0xC0,
	NUMPAD_0 = 0x60,
	NUMPAD_1 = 0x61,
	NUMPAD_2 = 0x62,
	NUMPAD_3 = 0x63,
	NUMPAD_4 = 0x64,
	NUMPAD_5 = 0x65,
	NUMPAD_6 = 0x66,
	NUMPAD_7 = 0x67,
	NUMPAD_8 = 0x68,
	NUMPAD_9 = 0x69,
};

class KeyState
{
public:
	KeyState()
		: m_isDown(false)
		, m_wasJustPressed(false)
		, m_wasJustReleased(false)
	{};
	bool m_isDown;
	bool m_wasJustPressed;
	bool m_wasJustReleased;
};

class Input
{
	KeyState m_keyStates[NUM_KEYS];
	XboxController m_controllers[MAX_CONTROLLERS];
	bool m_doesAppHaveFocus;
	bool m_hideMouseCursorWhenFocused;
	bool m_mouseScrollIsDown;
	bool m_mouseScrollWasDown;
	bool m_mouseScrollIsUp;
	bool m_mouseScrollWasUp;

public:
	Input();
	~Input();
	void Update();
	bool IsKeyDown(int keyCode) const;
	bool WasKeyJustPressed(int keyCode) const;
	bool WasKeyJustReleased(int keyCode) const;
	void OnKeyDown(int keyCode);
	void OnKeyUp(int keyCode);
	const XboxController& GetControllerState(int controllerID){return m_controllers[controllerID]; }
	Vector2 GetMouseScreenCoords() const;
	Vector2 GetScreenSize();
	void SetMouseCursorPosition(const Vector2& newMouseCoords);
	void OnAppGainFocus();
	void OnAppLoseFocus();
	bool DoesAppHaveFocus() const;
	void HideMouseCursor();
	void ShowMouseCursor();
	void SetMouseHiddenWhenFocused(bool hideMouseWhenNotFocused);
	bool WasMouseWheelScrollingUp() const;
	bool WasMouseWheelScrollingDown() const;
	bool IsMouseWheelScrollingUp() const;
	bool IsMouseWheelScrollingDown() const;
	void MouseWheelInput(short wmParam);
	Vector2 GetCursorNormalizedPosition(float worldWidth, float worldHeight, SimpleRenderer& renderer);

private:
	void UpdateMouseScroll();
	void UpdateKeyboard();
	void UpdateController();
	
};