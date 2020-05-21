﻿#pragma once

#include <dinput.h>

#define DEFINE_CONFIG_COMMON(name) static constexpr const char* STR_##name = #name;

#define DEFINE_CONFIG(name, dft) int name; \
								static constexpr int DFT_##name = dft;\
								DEFINE_CONFIG_COMMON(name);

#define DEFINE_CONFIG_WMAX(name, dft, max) DEFINE_CONFIG(name, dft)\
											static constexpr int MAX_##name = max;

#define DEFINE_STRCONFIG(name, dft, len) char name[len+1]; \
										static constexpr const char* DFT_##name = dft;\
										DEFINE_CONFIG_COMMON(name);

#define DFT_CONFIG_FILE "voice\\ed_voice.ini"

struct CConfig
{
	static constexpr int ShowInfo_Off = 0;
	static constexpr int ShowInfo_On = 1;
	static constexpr int ShowInfo_WithMark = 2;

	static constexpr int AutoPlay_Off = 0;
	static constexpr int AutoPlay_Voice = 1;
	static constexpr int AutoPlay_ALL = 2;

	static constexpr int OriginalVoice_Both = 0;
	static constexpr int OriginalVoice_EvoOnly = 1;
	static constexpr int OriginalVoice_OriOnly = 2;

	DEFINE_CONFIG_WMAX(Volume, 100, 100);
	DEFINE_CONFIG_WMAX(OriginalVoice, OriginalVoice_EvoOnly, 2);

	DEFINE_CONFIG_WMAX(OriVolumePercent, 100, 200);

	DEFINE_CONFIG_WMAX(AutoPlay, AutoPlay_Off, 2);
	DEFINE_CONFIG(WaitTimePerChar, 60);
	DEFINE_CONFIG(WaitTimeDialog, 800);
	DEFINE_CONFIG(WaitTimeDialogVoice, 500);

	DEFINE_CONFIG(SkipVoice, 1);
	DEFINE_CONFIG(DisableDialogSE, 1);
	DEFINE_CONFIG(DisableDududu, 1);
	DEFINE_CONFIG(TurboMultiplier, 2);
	DEFINE_CONFIG_WMAX(TurboJoypadButton, 0, 32);
	DEFINE_CONFIG(TurboJoypadTriggers, 1);
	DEFINE_CONFIG_WMAX(TurboKeyboardKey, DIK_LCONTROL, DIK_MEDIASELECT);
	DEFINE_CONFIG(FPSTarget, 60);
	DEFINE_CONFIG_WMAX(ShowInfo, ShowInfo_On, 2);

	DEFINE_STRCONFIG(FontName, "Microsoft Yahei", 63);
	DEFINE_CONFIG(FontColor, 0xFFFFFFFF);

	DEFINE_CONFIG(EnableKeys, 1);
	DEFINE_CONFIG(SaveChange, 1);

	const char* ExtraConfig(const char* configNm) const;

	bool LoadConfig(const char* configFn, bool create = false);
	bool SaveConfig(const char* configFn) const;

	void Reset(bool all = false) { load_default(all); }
	~CConfig();
	CConfig();
private:
	void load_default(bool all = true);
	void *pextra_data = nullptr;
};

extern CConfig Config;

