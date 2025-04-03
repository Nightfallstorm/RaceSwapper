#include "Settings.h"

Settings* Settings::GetSingleton()
{
	static Settings singleton;
	return std::addressof(singleton);
}

void Settings::Load()
{
	constexpr auto path = L"Data/SKSE/Plugins/RaceSwapper.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	const char* section = "Features";

    get_value(ini, Features::kPlaythroughRandomization, false, section, "bRandomizePerPlaythrough", "; In each playthough, RaceSwapper will alter NPCs differently, giving a different appearance");

	get_value(ini, Features::kStrictHeadPartMatching, false, section, "bStrictHeadParts", "; RaceSwapper will be stricter when choosing valid headparts for NPCs to reduces chances of using the wrong headpart (eg Khajiit head when swapping to a nord race). This can reduce variety of headpart distribution as a result!");

	get_value(ini, Features::kDebugLogging, false, section, "bDebugLogging", "; Enable debug logging. WARNING: Debug logs are quite spammy when loading lots of NPCs");

	#ifdef _DEBUG 
		Settings::GetSingleton()->features.set(Settings::Features::kDebugLogging)
	#endif

	ini.SaveFile(path);
}
