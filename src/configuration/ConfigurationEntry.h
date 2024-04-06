#pragma once
#include <utility>

class ConfigurationEntry
{
public:
	struct EntryData
	{
		// Matching Data
		RE::TESNPC* npcMatch = nullptr;
		RE::TESRace* raceMatch = nullptr;
		RE::TESFaction* factionMatch = nullptr;
		RE::SEX sexMatch = RE::SEX::kNone;
		std::uint32_t probability = 100; // 0-100

		// Exclude Data
		std::set<RE::TESNPC*> excludedNPCs;
		std::set<RE::TESRace*> excludedRaces;
		std::set<RE::TESFaction*> excludedFactions;
		std::set<RE::SEX> excludedSexes;

		// Appearance Data
		RE::TESNPC* otherNPC = nullptr;
		RE::TESRace* otherRace = nullptr;
		std::uint32_t weight = 10; // 0-100

		// Metadata
		std::string file;
		std::string entry;
	};

	EntryData entryData;

	bool MatchesNPC(RE::TESNPC* a_npc);

	static ConfigurationEntry* ConstructNewEntry(std::string line, std::string a_file);
};
