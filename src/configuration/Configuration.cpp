#pragma once
#include "Configuration.h"
#include "Utils.h"

void ConfigurationDatabase::Initialize() {
	logger::info("Reading config APIs...");
	// TODO: Change this path later?
	constexpr auto path = L"Data/SKSE/Plugins/RaceSwap";

	if (!std::filesystem::exists(path)) {
		logger::warn("No RaceSwap folder path to parse!");
		return;
	}
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		// TODO: Certain files can crash when getting the string, add exception handling
		logger::info("Parsing file {}", entry.path().string().c_str());
		std::fstream config;
		config.open(entry.path(), std::ios::in);
		if (!config.is_open()) {
			logger::error("Couldn't open file {}", entry.path().string().c_str());
			continue;
		}
		std::string line;
		while (std::getline(config, line)) {
			auto configEntry = ConfigurationEntry::ConstructNewEntry(line, entry.path().string());
			if (configEntry) {
				entries.push_back(configEntry);
			}
		}
	}

	logger::info("Config APIs fully parsed!");
}

ConfigurationEntry* PickRandomWeightedEntry(std::vector<std::pair<std::uint32_t, ConfigurationEntry*>> a_entries, RE::TESNPC* a_npc)
{
	if (a_entries.empty()) {
		return nullptr;	
	}
	if (a_entries.size() == 1) {
		return a_entries[0].second;
	}
	std::vector<std::uint32_t> weights(a_entries.size(), 0);
	weights[0] = a_entries[0].first;
	for (std::uint32_t i = 1; i < a_entries.size(); i++) {
		weights[i] = a_entries[i].first + weights[i-1]; 
	}

	auto seed = utils::HashForm(a_npc);
	srand((int) seed);
	auto index = std::upper_bound(weights.begin(), weights.end(), rand() % weights.back()) - weights.begin();
	return a_entries[index].second;
}

AppearanceConfiguration* ConfigurationDatabase::GetConfigurationForNPC(RE::TESNPC* a_npc) {
	std::vector<std::pair<std::uint32_t, ConfigurationEntry*>> matchedEntries;
	for (auto entry : entries) {
		if (entry->MatchesNPC(a_npc)) {
			matchedEntries.push_back({ entry->entryData.weight, entry });
		}
	}

	for (auto& entry : matchedEntries) {
		logger::debug("Consider following entry for NPC {:x}: file: \"{}\" entry: \"{}\"",
			a_npc->formID,
			entry.second->entryData.file,
			entry.second->entryData.entry
		);
	}

	if (!matchedEntries.empty()) {
		auto config = new AppearanceConfiguration{ 0 };
		auto matchedEntry = PickRandomWeightedEntry(matchedEntries, a_npc);
		config->otherRace = matchedEntry->entryData.otherRace;
		config->otherNPC = matchedEntry->entryData.otherNPC;
		// Setup config to match vampire/non-vampire NPC to vampire/non-vampire race counterpart
		if (utils::IsVampire(a_npc)) {
			config->otherRace = utils::AsVampireRace(config->otherRace);
		} else {
			config->otherRace = utils::AsNonVampireRace(config->otherRace);
		}
		config->file = matchedEntry->entryData.file;
		config->entry = matchedEntry->entryData.entry;
		return config;
	}

	return nullptr;
}
