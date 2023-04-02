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


	// PRANKS
	prankSettings.Load(ini);

	ini.SaveFile(path);
}



void Settings::PrankSettings::Load(CSimpleIniA& a_ini)
{
	//const char* section = "Settings";

	// TODO
	//Prank::SetCurrentPrank(new AllBeast());
	//detail::get_value(a_ini, fixToggleScriptSave, section, "bFixToggleScriptsCommand", ";Fixes ToggleScripts command not persisting when saving/stack dumping\n;Scripts will now stay turned off when toggled off, and on when toggled on.");
}
