#include <ed_voice.h>
#include <Windows.h>

#include <d3d9/d3d9.h>

#include <cstdint>
#include <Psapi.h>

#pragma comment lib("d3d9.lib")

#ifdef DINPUT8
#define DLL_NAME dinput8
#define API_NAME DirectInput8Create
#define HOOKED_API Hooked_DirectInput8Create
#define CALL_PARAM_DCL (void * hinst, unsigned dwVersion, void * riidltf, void ** ppvOut, void * punkOuter)
#define CALL_PARAM (hinst, dwVersion, riidltf, ppvOut, punkOuter)
#define ERR_CODE 0x80070057L
#elif defined(DSOUND)
#define DLL_NAME dsound
#define API_NAME DirectSoundCreate
#define HOOKED_API Hooked_DirectSoundCreate
#define CALL_PARAM_DCL (void* pcGuidDevice, void **ppDS, void* pUnkOuter)
#define CALL_PARAM (pcGuidDevice, ppDS, pUnkOuter)
#define ERR_CODE 0x88780032
#endif

#define _S(V) #V
#define S(V) _S(V)

#ifndef SVCALL
#define SVCALL __stdcall
#endif // !SVCALL

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	long SVCALL HOOKED_API CALL_PARAM_DCL;

#ifdef __cplusplus
}
#endif // __cplusplus

#define OLD_NAME_DLL S(DLL_NAME) "_old.dll"
#define SYS_PATH_DLL "%systemroot%\\System32\\" S(DLL_NAME) ".dll"
#define STR_HOOKAPI_NAME S(API_NAME)

#define STR_ED_VOICE_DLL "voice/ed_voice.dll"
#define STR_START "Start"
#define STR_END "End"
#define STR_INIT "Init"
#define STR_UINIT "Uninit"

#define MAX_PATH_LEN 512

static HMODULE dll = NULL;
static HMODULE dll_ed_voice = NULL;
static bool init = false;
static struct {
	decltype(::Start)* Start;
	decltype(::End)* End;
	decltype(::Init)* Init;
	decltype(::Uninit)* Uninit;
} ed_voice_apis;

using Call_Create = decltype(HOOKED_API)*;
static Call_Create ori_api = nullptr;

// todo: someday hook this and make borderless windowed mode

///////////////////////////////////////////////
// d3d hook defs
///////////////////////////////////////////////
using FUNC_CreateDevice_t = HRESULT (WINAPI*)( 
	UINT					     Adapter,
	D3DDEVTYPE             DeviceType,
	HWND                   hFocusWindow,
	DWORD                  BehaviorFlags,
	D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9**	  ppReturnedDeviceInterface
);

using FUNC_Direct3DCreate9_t = IDirect3D9* (WINAPI*)( UINT ver );

using FUNC_AdjustWindowRect_t = HRESULT (WINAPI*)( LPRECT lpRect,
	DWORD  dwStyle,
	BOOL   bMenu 
);

using FUNC_Present_t = HRESULT ( WINAPI* )( const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion );
using FUNC_Reset_t = HRESULT( WINAPI* )( D3DPRESENT_PARAMETERS* pPresentationParameters );

FUNC_CreateDevice_t _pCreateDeviceFunc;
FUNC_Direct3DCreate9_t _pDirect3DCreate8Func;

HRESULT DETOURED_CreateDevice( UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface )
{
	
	return _pCreateDeviceFunc( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface );
}

IDirect3D9* DETOURED_Direct3DCreate9( UINT ver )
{
	IDirect3D9* pRet = nullptr;

	return pRet;
}

HRESULT DETOURED_SwapPresent( IDirect3DSwapChain9* pDevice, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags )
{
}

static BOOL Compare( const BYTE* pData, const BYTE* bMask, const char* szMask )
{
	for( ; *szMask; ++szMask, ++pData, ++bMask )
	{
		if( *szMask == 'x' && *pData != *bMask )
			return 0;
	}

	return ( *szMask ) == NULL;
}

static DWORD64 FindPattern( BYTE* bMask, char* szMask )
{
	MODULEINFO moduleInfo = { 0 };
	GetModuleInformation( GetCurrentProcess(), GetModuleHandle( NULL ), &moduleInfo, sizeof( MODULEINFO ) );

	DWORD64 dwBaseAddress = (DWORD64)moduleInfo.lpBaseOfDll;
	DWORD64 dwModuleSize = (DWORD64)moduleInfo.SizeOfImage;

	for( DWORD64 i = 0; i < dwModuleSize; i++ )
	{
		if( Compare( (BYTE*)( dwBaseAddress + i ), bMask, szMask ) )
			return (DWORD64)( dwBaseAddress + i );
	}

	return 0;
}

void ResolutionPatch()
{
	DWORD64 cmpAddr1 = FindPattern( (BYTE*)"\x81\x7D\x08\x00\x08\x00\x00\x7E\x07\x33\xC0\xE9\x00\x00\x00\x00", "xxxxxxxxxxxx????" );
	DWORD64 cmpAddr2 = FindPattern( (BYTE*)"\x81\x7D\x0C\x00\x08\x00\x00\x7E\x07\x33\xC0\xE9\x00\x00\x00\x00", "xxxxxxxxxxxx????" );

	uint16_t* pCmpAddr = 0;
	uint16_t* pCmpAddr2 = 0;

	if( cmpAddr1 )
		pCmpAddr = (uint16_t*)( cmpAddr1 + 0x03 );
	if( cmpAddr2 )
		pCmpAddr2 = (uint16_t*)( cmpAddr2 + 0x03 );

	if( pCmpAddr && pCmpAddr2 )
	{
		DWORD old;
		//LOG( "Attempting resolution patch.." );
		// Half the sleeping on the rendering/presenting function (16 to 8):
		if( cmpAddr1 && VirtualProtect( pCmpAddr, 2, PAGE_EXECUTE_READWRITE, &old ) != 0 ) {
			*pCmpAddr = 8192;
		}
		else {
			//LOG( "Could not unlock cmpAddr1 to patch it" );
		}
		if( cmpAddr2 && VirtualProtect( pCmpAddr2, 2, PAGE_EXECUTE_READWRITE, &old ) != 0 ) {
			*pCmpAddr2 = 8192;
		}
		else {
			//LOG( "Could not unlock cmpAddr2 to patch it" );
		}
	}
	else
	{
		//LOG( "Could not find signatures for resolution unlock!" );
	}
}


///////////////////////////////////////////////////////
// SoraVoice
///////////////////////////////////////////////////////
long SVCALL HOOKED_API CALL_PARAM_DCL
{	
	auto rst = ori_api ? ori_api CALL_PARAM : ERR_CODE;

	if (!init) {
		init = true;
		if (ed_voice_apis.Start) {
			ed_voice_apis.Start();
		}
	}

	return rst;
}

BOOL Initialize(PVOID /*BaseAddress*/) {
	if (!dll) {
		ResolutionPatch();
		dll = LoadLibraryA(OLD_NAME_DLL);
		if (!dll) {
			char buff[MAX_PATH_LEN + 1];
			ExpandEnvironmentStringsA(SYS_PATH_DLL, buff, sizeof(buff));
			dll = LoadLibraryA(buff);
		}

		if (dll) {
			ori_api = (Call_Create)GetProcAddress(dll, STR_HOOKAPI_NAME);
		}

		dll_ed_voice = LoadLibraryA(STR_ED_VOICE_DLL);
		if (dll_ed_voice) {
#if _DEBUG
			MessageBox(0, "Stop", "Stop", 0);
#endif // _DEBUG
			ed_voice_apis.Start = (decltype(ed_voice_apis.Start))GetProcAddress(dll_ed_voice, STR_START);
			ed_voice_apis.End = (decltype(ed_voice_apis.End))GetProcAddress(dll_ed_voice, STR_END);
			ed_voice_apis.Init = (decltype(ed_voice_apis.Start))GetProcAddress(dll_ed_voice, STR_INIT);
			ed_voice_apis.Uninit = (decltype(ed_voice_apis.End))GetProcAddress(dll_ed_voice, STR_UINIT);

			if (ed_voice_apis.Init) {
				if (!ed_voice_apis.Init()) {
					FreeLibrary(dll_ed_voice);
					ed_voice_apis.Init = ed_voice_apis.Uninit = ed_voice_apis.Start = ed_voice_apis.End = nullptr;
					dll_ed_voice = nullptr;
				}
			}
		}
	}

	return TRUE;
}

BOOL Uninitialize(PVOID /*BaseAddress*/) {
	return TRUE;
}

BOOL WINAPI DllMain(PVOID BaseAddress, ULONG Reason, PVOID /*Reserved*/)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		return Initialize(BaseAddress);

	case DLL_PROCESS_DETACH:
		return Uninitialize(BaseAddress);
	}

	return TRUE;
}
