#include "Config.h"
#include <SVData.h>
#include <Message.h>

#include <string>
#include <fstream>
#include <unordered_map>
#include <iomanip>
#include <cstring>

using namespace std;

#define MAXCH_ONELINE 1024

using KeyValue = unordered_map<string, string>;

#define REMOVE_VALUE(name, kv) kv.erase(#name)
#define GET_VALUE(name, kv) { _getValue(name, kv, STR_##name); REMOVE_VALUE(name, kv); }
#define GET_VALUE_MAXFIX(name, kv) {_getValue(name, kv, STR_##name);\
				if(name < 0) name = 0; \
				if(name > MAX_##name) name = MAX_##name; \
				REMOVE_VALUE(name, kv);}
#define GET_VALUE_BOOLFIX(name, kv) {_getValue(name, kv, STR_##name); if(name) name = 1; REMOVE_VALUE(name, kv);}

#define OUTPUT_VALUE(name, ofs) ofs << STR_##name << " = " << name << '\n' << Message.CMT.name << '\n' << '\n'
#define OUTPUT_VALUE_WFMT(name, ofs, format) ofs<< STR_##name << " = " << format << name << '\n'  << Message.CMT.name << '\n' << '\n'
#define OUTPUT_EXTRA(p, ofs) { if(p) for(auto& __kv : *(p)) ofs << __kv.first << " = " << __kv.second << '\n' << '\n'; }

#define SET_DEFAULT(name) name = DFT_##name
#define SET_DEFAULT_STR(name) strcpy(name, DFT_##name)

static bool _getValue(int& var, const KeyValue& kv, const char* name) {
	auto it = kv.find(name);
	if (it == kv.end()) return false;

	int rad = 10;
	if (it->second[0] == '0' && (it->second[1] == 'x' || it->second[1] == 'X')) {
		rad = 16;
	}
	char *p;
	var = (int)std::strtoul(it->second.c_str(), &p, rad);
	return true;
}

template<typename ArrayType, typename = std::enable_if_t<std::is_array<ArrayType>::value>>
static bool _getValue(ArrayType &var, const KeyValue& kv, const char* name) {

	auto it = kv.find(name);
	if (it == kv.end()) return false;

	constexpr int len_array = std::extent<ArrayType>::value;

	for (int i = 0; i <= (int)it->second.length() && i < len_array; i++) {
		var[i] = it->second[i];
	}
	var[len_array - 1] = 0;

	return true;
}

const char * CConfig::ExtraConfig(const char * configNm) const
{
	if (!configNm || !pextra_data) return nullptr;
	KeyValue& kv = *(KeyValue*)pextra_data;

	auto it = kv.find(configNm);
	if (it == kv.end()) return nullptr;

	return it->second.c_str();
}

bool CConfig::LoadConfig(const char * configFn, bool create)
{
	load_default();

	ifstream ifs(configFn);

	if (!ifs) {
		if(create) {
			SaveConfig(configFn);
		}
		return false;
	}
	KeyValue& kv = *(KeyValue*)pextra_data;
	kv.clear();

	char buff[MAXCH_ONELINE];
	bool first = true;
	while (ifs.getline(buff, sizeof(buff)))
	{
		char* p = buff;
		if (first) {
			first = false;
			if (p[0] == (char)'\xEF' && p[1] == (char)'\xBB' && p[2] == (char)'\xBF') {
				p += 3;
			}
		}
		if (p[0] == 0 || p[0] == '#' || p[0] == ';') continue;

		string key, value;
		while (*p && (*p == ' ' || *p == '\t')) p++;

		while (*p && *p != '=') key.push_back(*p++);
		if (*p++ != '=') continue;

		while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
		if (key.empty()) continue;

		while (*p && (*p == ' ' || *p == '\t')) p++;
		while (*p) value.push_back(*p++);
		while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) value.pop_back();

		kv.insert({ key, value });
	}

	GET_VALUE_MAXFIX(Volume, kv);
	GET_VALUE_MAXFIX(OriginalVoice, kv);
	GET_VALUE_MAXFIX(OriVolumePercent, kv);

	GET_VALUE_MAXFIX(AutoPlay, kv);
	GET_VALUE(WaitTimePerChar, kv);
	GET_VALUE(WaitTimeDialog, kv);
	GET_VALUE(WaitTimeDialogVoice, kv);

	GET_VALUE_BOOLFIX(SkipVoice, kv);
	GET_VALUE_BOOLFIX(DisableDialogSE, kv);
	GET_VALUE_BOOLFIX(DisableDududu, kv);

	GET_VALUE(TurboMultiplier, kv);
	GET_VALUE_MAXFIX(TurboJoypadButton, kv);
	GET_VALUE(TurboJoypadTriggers, kv);
	GET_VALUE_MAXFIX(TurboKeyboardKey, kv);
	GET_VALUE(FPSTarget, kv);

	GET_VALUE_MAXFIX(ShowInfo, kv);
	GET_VALUE(FontName, kv);
	GET_VALUE(FontColor, kv);

	GET_VALUE_BOOLFIX(EnableKeys, kv);
	GET_VALUE_BOOLFIX(SaveChange, kv);

	if (AutoPlay) SkipVoice = 1;

	return true;
}

bool CConfig::SaveConfig(const char * configFn) const
{
	ofstream ofs(configFn);
	if (!ofs) return false;
	
	ofs << '\n';
	OUTPUT_VALUE(Volume, ofs);
	ofs << '\n';

	if(SV.game == AO) {
		OUTPUT_VALUE(OriginalVoice, ofs);
		OUTPUT_VALUE(OriVolumePercent, ofs);
		ofs << '\n';
	}

	OUTPUT_VALUE(AutoPlay, ofs);
	OUTPUT_VALUE(WaitTimePerChar, ofs);
	OUTPUT_VALUE(WaitTimeDialog, ofs);
	OUTPUT_VALUE(WaitTimeDialogVoice, ofs);
	ofs << '\n';

	OUTPUT_VALUE(SkipVoice, ofs);
	OUTPUT_VALUE(DisableDialogSE, ofs);
	OUTPUT_VALUE(DisableDududu, ofs);
	OUTPUT_VALUE(TurboMultiplier, ofs);
	OUTPUT_VALUE(TurboJoypadButton, ofs);
	OUTPUT_VALUE(TurboJoypadTriggers, ofs);
	OUTPUT_VALUE(TurboKeyboardKey, ofs);
	OUTPUT_VALUE(FPSTarget, ofs);
	OUTPUT_VALUE(ShowInfo, ofs);
	ofs << '\n';

	OUTPUT_VALUE(FontName, ofs);
	OUTPUT_VALUE_WFMT(FontColor, ofs, "0x" << setfill('0') << setw(8) << setiosflags(ios::right | ios::uppercase) << hex);
	ofs << '\n';

	OUTPUT_VALUE(EnableKeys, ofs);
	OUTPUT_VALUE(SaveChange, ofs);

	OUTPUT_EXTRA(((KeyValue*)pextra_data), ofs);

	ofs.close();
	return true;
}

CConfig::~CConfig()
{
	delete (KeyValue*)pextra_data;
}

CConfig::CConfig() : pextra_data(new KeyValue())
{
}

void CConfig::load_default(bool all)
{
	SET_DEFAULT(Volume);
	SET_DEFAULT(OriginalVoice);
	SET_DEFAULT(OriVolumePercent);

	SET_DEFAULT(AutoPlay);
	SET_DEFAULT(WaitTimePerChar);
	SET_DEFAULT(WaitTimeDialog);
	SET_DEFAULT(WaitTimeDialogVoice);

	SET_DEFAULT(SkipVoice);
	SET_DEFAULT(DisableDialogSE);
	SET_DEFAULT(DisableDududu);

	SET_DEFAULT(TurboMultiplier);
	SET_DEFAULT(TurboJoypadButton);
	SET_DEFAULT(TurboJoypadTriggers);
	SET_DEFAULT(TurboKeyboardKey);
	SET_DEFAULT(FPSTarget);

	SET_DEFAULT(ShowInfo);
	SET_DEFAULT_STR(FontName);
	SET_DEFAULT(FontColor);

	if (all) {
		SET_DEFAULT(EnableKeys);
		SET_DEFAULT(SaveChange);
	}
}

CConfig Config;
