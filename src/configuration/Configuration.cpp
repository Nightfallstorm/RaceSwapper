#pragma once
#include "Configuration.h"

void ConfigurationDatabase::Initialize() {
	// TODO:
}

AppearanceConfiguration* getDebugConfiguration() {
	// TODO: REMOVE THIS FUNCTION WHEN DONE
	AppearanceConfiguration* newConfig = new AppearanceConfiguration;

	newConfig->otherNPC = std::pair(RE::TESForm::LookupByID(constants::Maiq)->As<RE::TESNPC>(), 0);

	return newConfig;
}

AppearanceConfiguration* ConfigurationDatabase::GetConfigurationForNPC(RE::TESNPC* a_npc) {
	// TODO:
	return getDebugConfiguration();
	//return nullptr;
}



bool ConfigurationEntry::MatchesNPC(RE::TESNPC* a_npc) {
	// TODO:
	return false;
}

void ConfigurationEntry::ApplyEntry(AppearanceConfiguration* a_config) {
	// TODO:
}
