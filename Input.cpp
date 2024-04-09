#include "Input.h"

#include "ErrMsg.h"


Input::Input()
{
	_mousePos = new POINT{ };
	_windRect = new RECT{ };

	for (int i = 0; i < 255; i++)
	{
		_vKeys[i] = (GetAsyncKeyState(i) & 0x8000) ? true : false;
	}

	if (!GetCursorPos(_mousePos))
		ErrMsg("Failed to get cursor position!");

	_mouseX = _mousePos->x;
	_mouseY = _mousePos->y;
}

Input::~Input()
{
	delete _mousePos;
	delete _windRect;
}


bool Input::Update(const HWND window)
{
	_isInFocus = (window == GetFocus());

	if (!GetWindowRect(window, _windRect))
	{
		ErrMsg("Failed to get window rectangle!");
		return false;
	}

	if (!GetCursorPos(_mousePos))
	{
		ErrMsg("Failed to get cursor position!");
		return false;
	}


	if (_cursorLocked)
	{
		_lMouseX = (_windRect->left + _windRect->right) / 2;
		_lMouseY = (_windRect->top + _windRect->bottom) / 2;

		SetCursorPos(_lMouseX, _lMouseY);
	}
	else
	{
		_lMouseX = _mouseX;
		_lMouseY = _mouseY;
	}

	_mouseX = _mousePos->x;
	_mouseY = _mousePos->y;


	for (int i = 0; i < 255; i++)
	{
		_lvKeys[i] = _vKeys[i];
		_vKeys[i] = (GetAsyncKeyState(i) & 0x8000) ? true : false;
	}

	return true;
}

KeyState Input::GetKey(const KeyCode keyCode) const
{
	const unsigned char key = static_cast<unsigned char>(keyCode);
	return GetKey(static_cast<UCHAR>(keyCode));
}

KeyState Input::GetKey(const UCHAR keyCode) const
{
	const bool
		thisFrame = _vKeys[keyCode],
		lastFrame = _lvKeys[keyCode];

	if (thisFrame)
	{
		if (lastFrame)
			return KeyState::Held;

		return KeyState::Pressed;
	}

	if (lastFrame)
		return KeyState::Released;

	return KeyState::None;
}

MouseState Input::GetMouse() const
{
	return {
		_mouseX,
		_mouseY,
		_mouseX - _lMouseX,
		_mouseY - _lMouseY
	};
}


bool Input::IsInFocus() const { return _isInFocus; }
bool Input::IsCursorLocked() const { return _cursorLocked; }
bool Input::IsCursorVisible() const { return _cursorVisible; }

bool Input::ToggleLockCursor()
{
	_cursorLocked = !_cursorLocked;
	return _cursorLocked;
}

bool Input::ToggleShowCursor()
{
	_cursorVisible = !_cursorVisible;

	if (_cursorVisible)
		while (ShowCursor(TRUE) < 0);
	else
		while (ShowCursor(FALSE) >= 0);

	return _cursorVisible;
}
