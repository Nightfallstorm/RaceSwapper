#pragma once
#include "Configuration.h"

void ConfigurationDatabase::Initialize() {
	// TODO:
}

AppearanceConfiguration* getDebugConfiguration(RE::TESNPC* a_npc) {
	// TODO: REMOVE THIS FUNCTION WHEN DONE

	AppearanceConfiguration* newConfig = new AppearanceConfiguration;

	//newConfig->otherRace = std::pair(RE::TESForm::LookupByID(constants::DragonRace)->As<RE::TESRace>(), 0);
	newConfig->otherNPC = std::pair(RE::TESForm::LookupByID(constants::Urog)->As<RE::TESNPC>(), 0);
	return newConfig;
}

AppearanceConfiguration* ConfigurationDatabase::GetConfigurationForNPC(RE::TESNPC* a_npc) {
	// TODO:
	return getDebugConfiguration(a_npc);
	//return nullptr;
}



bool ConfigurationEntry::MatchesNPC(RE::TESNPC* a_npc) {
	// TODO:
	return false;
}

void ConfigurationEntry::ApplyEntry(AppearanceConfiguration* a_config) {
	// TODO:
}
