#pragma once
#include "Configuration.h"

void ConfigurationDatabase::Initialize() {
	// TODO:
	
	////////// DEBUG /////////////////
	auto debugConfigEntry = new ConfigurationEntry("");
	debugConfigEntry->entryData.raceMatch = RE::TESForm::LookupByID(constants::NordRace)->As<RE::TESRace>();
	debugConfigEntry->entryData.otherRace = RE::TESForm::LookupByID(constants::KhajiitRace)->As<RE::TESRace>();
	debugConfigEntry->entryData.probability = 50;
	entries.push_back(debugConfigEntry);
	//////////////////////////////////
}

AppearanceConfiguration* ConfigurationDatabase::GetConfigurationForNPC(RE::TESNPC* a_npc) {
	
	AppearanceConfiguration* config = nullptr;

	for (auto entry : entries) {
		if (entry->MatchesNPC(a_npc)) {
			if (config == nullptr) {
				config = new AppearanceConfiguration;
			}
			ApplyConfiguration(entry, config);
		}
	}

	return config;
}

void ConfigurationDatabase::ApplyConfiguration(ConfigurationEntry* a_entry, AppearanceConfiguration* a_config) {
	auto& entryData = a_entry->entryData;
	if (!a_config->otherNPC.first || a_config->otherNPC.second < entryData.priority) {
		if (entryData.otherNPC) {
			a_config->otherNPC.first = entryData.otherNPC;
			a_config->otherNPC.second = entryData.priority;
		}
	}

	if (!a_config->otherRace.first || a_config->otherRace.second < entryData.priority) {
		if (entryData.otherRace) {
			a_config->otherRace.first = entryData.otherRace;
			a_config->otherRace.second = entryData.priority;
		}
	}
}
