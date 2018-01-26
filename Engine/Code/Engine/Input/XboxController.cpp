#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput9_1_0.lib") // #Eiserloh: explicitly use Xinput v9.1.0 since Xinput1_4 is not included in Windows7


static const WORD XINPUT_Buttons_BitFlags[] = {
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
	XINPUT_GAMEPAD_LEFT_SHOULDER,
	XINPUT_GAMEPAD_RIGHT_SHOULDER,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_BACK
};

bool XboxController::IsButtonDown(ButtonTypes buttonID) const
{
	return m_buttonStates[buttonID].m_isDown;
}

bool XboxController::WasButtonJustPressed(ButtonTypes buttonID) const
{
	bool ButtonIsDown = m_buttonStates[buttonID].m_isDown;
	bool ButtonWasDown = m_buttonStates[buttonID].m_wasDown;
	return ButtonIsDown && !ButtonWasDown;
}

void XboxController::Update()
{
	// Read controller state from API
	XINPUT_STATE xboxControllerState;
	memset(&xboxControllerState, 0, sizeof(xboxControllerState));
	DWORD errorStatus = XInputGetState(m_controllerID, &xboxControllerState);
	if (errorStatus != ERROR_SUCCESS)
	{
		m_isConnected = false;
		ResetController();
		return;
	}

	// Update button states
	for (int buttonID = 0; buttonID < NUM_BUTTONS; buttonID++)
	{
		XboxButtonState& buttonState = m_buttonStates[buttonID];
		buttonState.m_wasDown = buttonState.m_isDown;
		buttonState.m_isDown = (xboxControllerState.Gamepad.wButtons & XINPUT_Buttons_BitFlags[buttonID]) != 0;
	}

	// Update joystick states
	//Left
	float leftStickX = (float)xboxControllerState.Gamepad.sThumbLX / XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	float leftStickY = (float)xboxControllerState.Gamepad.sThumbLY / XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
	m_leftStickPos = Vector2(leftStickX, leftStickY);

	m_leftStickMagnitude = (float)sqrt((leftStickX * leftStickX) + (leftStickY * leftStickY));
	m_leftStickOrientation = atan2InDegrees(leftStickY, leftStickX);
	float innerDeadZone = 0.3f;
	float outerDeadZone = 0.95f;
	m_leftStickMagnitude = ClampNormalizedFloat(RangeMapFloat(innerDeadZone, outerDeadZone, 0.f, 1.f, m_leftStickMagnitude));

	leftStickX = m_leftStickMagnitude * CosInDegrees(m_leftStickOrientation);
	leftStickY = m_leftStickMagnitude * SinInDegrees(m_leftStickOrientation);
	m_leftStickPos = Vector2(leftStickX, leftStickY);
	//Right
	float rightStickX = (float)xboxControllerState.Gamepad.sThumbRX / XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	float rightStickY = (float)xboxControllerState.Gamepad.sThumbRY / XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
	m_rightStickPos = Vector2(rightStickX, rightStickY);

	m_rightStickMagnitude = (float)sqrt((rightStickX * rightStickX) + (rightStickY * rightStickY));
	m_rightStickOrientation = atan2InDegrees(rightStickY, rightStickX);
	m_rightStickMagnitude = ClampNormalizedFloat(RangeMapFloat(innerDeadZone, outerDeadZone, 0.f, 1.f, m_rightStickMagnitude));

	rightStickX = m_rightStickMagnitude * CosInDegrees(m_rightStickOrientation);
	rightStickY = m_rightStickMagnitude * SinInDegrees(m_rightStickOrientation);
	m_rightStickPos = Vector2(rightStickX, rightStickY);

	float leftTrigger = (float)xboxControllerState.Gamepad.bLeftTrigger / XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	float rightTrigger = (float)xboxControllerState.Gamepad.bRightTrigger / XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	m_leftTriggerMagnitude = ClampNormalizedFloat(RangeMapFloat(innerDeadZone, outerDeadZone, 0.f, 1.f, leftTrigger));
	m_rightTriggerMagnitude = ClampNormalizedFloat(RangeMapFloat(innerDeadZone, outerDeadZone, 0.f, 1.f, rightTrigger));
}

void XboxController::ResetController()
{
	m_leftStickMagnitude = 0.f;
	m_leftStickOrientation = 0.f;
	m_leftStickPos = Vector2(0.f, 0.f);
	m_rightStickMagnitude = 0.f;
	m_rightStickOrientation = 0.f;
	m_rightStickPos = Vector2(0.f, 0.f);

	for (int buttonID = 0; buttonID < NUM_BUTTONS; buttonID++)
	{
		XboxButtonState& buttonState = m_buttonStates[buttonID];
		buttonState.m_isDown = false;
		buttonState.m_wasDown = false;
	}
}

XboxController::XboxController()
	:m_isConnected(false)
	, m_controllerID(-1)
	, m_leftStickMagnitude(0.f)
	, m_leftStickOrientation(0.f)
	, m_leftStickPos(0.f, 0.f)
{
	for (int buttonIndex = 0; buttonIndex < NUM_BUTTONS; buttonIndex++)
	{
		m_buttonStates[buttonIndex].m_isDown = false;
		m_buttonStates[buttonIndex].m_wasDown = false;
	}
}

XboxController::~XboxController()
{}

// void XboxController::Rumble(float a_fLeftMotor, float a_fRightMotor)
// {
// 	XINPUT_VIBRATION VibrationState;
// 
// 	ZeroMemory(&VibrationState, sizeof(XINPUT_VIBRATION));
// 
// 	int iLeftMotor = int(a_fLeftMotor * 65535.0f);
// 	int iRightMotor = int(a_fRightMotor * 65535.0f);
// 
// 	VibrationState.wLeftMotorSpeed = (WORD)iLeftMotor;
// 	VibrationState.wRightMotorSpeed = (WORD)iRightMotor;
// 
// 	XInputSetState(m_iGamepadIndex, &VibrationState);
// }
