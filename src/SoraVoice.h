#pragma once

#ifndef SVCALL
#define SVCALL __stdcall
#endif

#include <dinput.h>

namespace SoraVoice
{
	void Play(const char* v);
	void Stop();
	void JoypadInput(DIJOYSTATE* joypadState);
	void Input();
	void Show(void* pD3DD);

	bool Init();
	bool End();
};


