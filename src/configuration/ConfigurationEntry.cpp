#pragma once
#include "ConfigurationEntry.h"
#include "Utils.h"

ConfigurationEntry::ConfigurationEntry(std::string line) {
	// TODO:
}

bool ConfigurationEntry::MatchesNPC(RE::TESNPC* a_npc) {
	bool isMatch = false;

	isMatch = isMatch || (entryData.npcMatch != nullptr && entryData.npcMatch->formID == a_npc->formID);
	isMatch = isMatch || (entryData.raceMatch != nullptr && entryData.raceMatch->formID == a_npc->race->formID);
	isMatch = isMatch || (entryData.factionMatch != nullptr && a_npc->IsInFaction(entryData.factionMatch));

	if (isMatch) {
		srand((int) utils::HashForm(a_npc));
		isMatch = ((std::uint32_t) rand() % 100) < entryData.probability;
	}

	return isMatch;
}

