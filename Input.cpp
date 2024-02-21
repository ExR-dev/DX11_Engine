#include "Input.h"

#include <Windows.h>

#include "ErrMsg.h"


void Input::Update()
{
	for (int i = 0; i < 255; i++)
	{
		_lvKeys[i] = _vKeys[i];
		_vKeys[i] = (GetAsyncKeyState(i) & 0x8000) ? true : false;
	}
}

bool Input::GetKey(const KeyCode keyCode) const
{
	const unsigned char key = static_cast<unsigned char>(keyCode);

	if (key < 0 || key > 255)
	{
		ErrMsg("Invalid key code");
		return false;
	}

	return _vKeys[key];
}
