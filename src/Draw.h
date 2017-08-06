#pragma once

#include "Type.h"
#include <string>

class Draw {
public:
	enum class InfoType
	{
		Hello = 0,
		InfoOnoff,
		AutoPlayMark,

		Volume,
		OriginalVoice,
		OriVolumePercent,

		AutoPlay,
		SkipVoice,
		DisableDialogSE,
		DisableDududu,

		ConfigReset,

		All,
		Dead
	};

	static constexpr unsigned ShowTimeInfinity = 0;

	static Draw * CreateDraw(u8& showing, void * hWnd, void * pD3DD, const char* fontName);
	static void DestoryDraw(Draw * draw);

	virtual void DrawInfos() = 0;

	virtual void AddInfoText(InfoType type, unsigned time, unsigned color, const char* text) = 0;

	template<typename... Texts>
	void AddInfo(InfoType type, unsigned time, unsigned color, Texts... texts) {
		return AddInfoText(type, time, color, GetStr(texts...).c_str());
	}

	virtual void RemoveInfo(InfoType type) = 0;

	const u8& Showing() const { return showing; }

protected:
	Draw(u8& showing) : showing(showing) { }
	u8 &showing;
	virtual ~Draw() { };

	inline static std::string GetStr(const char* first) {
		return first;
	}

	template<typename First, typename = std::enable_if_t<std::is_integral_v<First>>>
	inline static std::string GetStr(First first) {
		return std::to_string(first);
	}

	template<typename First, typename... Remain>
	inline static std::string GetStr(First first, Remain... remains) {
		return GetStr(first) + GetStr(remains...);
	}
};
