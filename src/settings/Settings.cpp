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

	ini.SaveFile(path);
}
