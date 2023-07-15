#pragma once
#include <utility>

class ConfigurationEntry
{
public:
	struct EntryData
	{
		// Matching Data
		RE::TESNPC* npcMatch;
		RE::TESRace* raceMatch;
		RE::TESFaction* factionMatch;
		std::uint32_t probability; // 0-100

		// Appearance Data
		RE::TESNPC* otherNPC;
		RE::TESRace* otherRace;
		std::uint32_t priority;
	};

	EntryData entryData = { 0 };

	bool MatchesNPC(RE::TESNPC* a_npc);

	ConfigurationEntry(std::string line);
};
