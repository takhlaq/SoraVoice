#pragma once

#include <dinput.h>

namespace Hook {

	using HookCallBack = void(*)(void);

	using HookJoyCallBack = void(*)(DIJOYSTATE*);

	void* Hook_D3D_Present(void* pD3DD, int dx9, HookCallBack callback);

	void* Hook_DI_GetDeviceState(void* pDID, HookCallBack callback, void** ppkeys, HookJoyCallBack joypadCallback);
}

