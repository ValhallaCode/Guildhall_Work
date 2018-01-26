#pragma once
#include "Engine/Math/Vector2.hpp"


const int NUM_XBOX_BUTTONS = 16;

enum ButtonTypes
{
	Button_A,
	Button_B,
	Button_X,
	Button_Y,
	Button_DPAD_UP,
	Button_DPAD_DOWN,
	Button_DPAD_LEFT,
	Button_DPAD_RIGHT,
	Button_LEFT_SHOULDER,
	Button_RIGHT_SHOULDER,
	Button_LEFT_THUMB,
	Button_RIGHT_THUMB,
	Button_START,
	Button_BACK,
	NUM_BUTTONS
};

class XboxButtonState
{
public:
	XboxButtonState()
		: m_isDown(false)
		, m_wasDown(false)
	{};
	bool m_isDown;
	bool m_wasDown;
	ButtonTypes m_button;
}; 

class XboxController
{
	friend class Input;

public:
	bool IsConnected() const { return m_isConnected; }
	bool IsButtonDown(ButtonTypes buttonID) const;
	bool WasButtonJustPressed(ButtonTypes buttonID) const;
	Vector2 GetLeftStickPosition() const { return m_leftStickPos; }
	Vector2 GetRightStickPosition() const { return m_rightStickPos; }
	float GetLeftStickOrientation() const { return m_leftStickOrientation; }
	float GetRightStickOrientation() const { return m_rightStickOrientation; }
	float GetLeftTriggerMagnitude() const { return m_leftTriggerMagnitude; }
	float GetRightTriggerMagnitude() const { return m_rightTriggerMagnitude; }
	//void Rumble(float a_fLeftMotor = 0.0f, float a_fRightMotor = 0.0f);

private:
	void Update();      
	void ResetController();
	XboxController();
	~XboxController();

private:
	int m_controllerID;
	bool m_isConnected;
	XboxButtonState m_buttonStates[NUM_XBOX_BUTTONS];
	Vector2 m_leftStickPos;
	float m_leftStickOrientation;
	float m_leftStickMagnitude;
	Vector2 m_rightStickPos;
	float m_rightStickOrientation;
	float m_rightStickMagnitude;
	float m_leftTriggerMagnitude;
	float m_rightTriggerMagnitude;
};
