#pragma once
#include <Windows.h>
#include <Psapi.h>

class Pattern {
public:
	Pattern(const char* pattern):length(0), data{} {
		while (length < MAX_LENGTH && *pattern) {
			if (*pattern == ' ') {
				++pattern;
			}
			else if (*pattern == '?') {
				++pattern;
				if (*pattern == '?') ++pattern;
				data[length] = -1;
				length++;
			}
			else {
				data[length] = 0;
				for (int i = 0; i < 2; i++, pattern++) {
					if (*pattern >= '0' && *pattern <= '9') (data[length] *= 16) += *pattern - '0';
					else if (*pattern >= 'a' && *pattern <= 'f') (data[length] *= 16) += 10 + *pattern - 'a';
					else if (*pattern >= 'A' && *pattern <= 'F') (data[length] *= 16) += 10 + *pattern - 'A';
					else data[length] = 0x100;
				}
				length++;
			}
		}
	}
	bool Check(void* buff) const {
		unsigned char* p = (unsigned char*)buff;
		for (int i = 0; i < length; i++) {
			if (this->data[i] < 0) continue;
			else if (this->data[i] != p[i]) return false;
		}
		return true;
	}

	// Author: RangeMachine
	// Source: https://www.unknowncheats.me/forum/dayz-sa/228971-x64-zscanner-offsets-scanner.html
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
	int Legnth() const { return length; };

	static constexpr int MAX_LENGTH = 16;

private:
	int data[MAX_LENGTH];
	int length;
};

