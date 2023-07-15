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
		std::uint32_t probability = 100; // 0-100

		// Appearance Data
		RE::TESNPC* otherNPC = nullptr;
		RE::TESRace* otherRace = nullptr;
		std::uint32_t weight = 10; // 0-100
	};

	EntryData entryData;

	bool MatchesNPC(RE::TESNPC* a_npc);

	static ConfigurationEntry* ConstructNewEntry(std::string line);
};
