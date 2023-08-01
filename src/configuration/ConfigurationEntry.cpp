#pragma once
#include "ConfigurationEntry.h"
#include "Utils.h"

bool IsValidEntry(std::string line) {
	if (line.find("match=") == std::string::npos) {
		logger::error("line: \"{}\" is missing a 'match=' line and cannot be parsed!", line);
		return false;
	}

	if (line.find("match=") != 0) {
		logger::error("line: \"{}\" is invalid, 'match=' must be first in the line!", line);
	}

	if (line.find("swap=") == std::string::npos) {
		logger::error("line: \"{}\" is missing a 'swap=' line and cannot be parsed!", line);
		return false;
	}

	return true;
}

RE::TESForm* GetFormFromString(std::string line) {
	if (line.find('~') == std::string::npos) {
		logger::error("missing plugin: {}", line);
	}
	auto plugin = line.substr(line.find('~') + 1);
	auto formID = std::stoul(line.substr(0, line.find('~')), nullptr, 16);
	auto form = RE::TESDataHandler::GetSingleton()->LookupForm(formID, plugin);
	if (form == nullptr) {
		logger::error("invalid form ID: {}", line);
	}

	return form;
}

bool ConstructMatchData(std::string line, ConfigurationEntry::EntryData* a_data)
{
	std::string match = "match=";
	line.erase(0, match.size());
	auto data = utils::split_string(line, '|');
	for (auto& entry : data) {
		if (entry.find('%') != std::string::npos) {
			auto percent = std::stoul(entry.substr(0, entry.find('%')), nullptr, 10);
			percent = max(0, min(100, percent));
			a_data->probability = percent;
		} else {
			auto form = RE::TESForm::LookupByEditorID(entry);
			form = form ? form : GetFormFromString(entry);
			if (!form) {
				logger::error("{} is not a valid form ID or editor ID!", entry);
				return false;
			}
			if (form->Is(RE::FormType::NPC)) {
				a_data->npcMatch = form->As<RE::TESNPC>();
			} else if (form->Is(RE::FormType::Race)) {
				a_data->raceMatch = form->As<RE::TESRace>();
			} else if (form->Is(RE::FormType::Faction)) {
				a_data->factionMatch = form->As<RE::TESFaction>();
			} else {
				logger::error(
					"{} {:x} is not a valid NPC, Race or Faction!",
					utils::GetFormEditorID(form).c_str(),
					form->formID
				);
				return false;
			}
		}
	}

	return true;
}

bool ConstructSwapData(std::string line, ConfigurationEntry::EntryData* a_data)
{
	std::string swap = "swap=";
	line.erase(0, swap.size());
	auto data = utils::split_string(line, '|');
	for (auto& entry : data) {
		if (entry.find('%') != std::string::npos) {
			auto percent = std::stoul(entry.substr(0, entry.find('%')), nullptr, 10);
			percent = max(0, min(100, percent));
			a_data->weight = percent;
		} else {
			auto form = RE::TESForm::LookupByEditorID(entry);
			form = form ? form : GetFormFromString(entry);
			if (!form) {
				logger::error("{} is not a valid form ID or editor ID!", entry);
				return false;
			}
			if (form->Is(RE::FormType::NPC)) {
				a_data->otherNPC = form->As<RE::TESNPC>();
			} else if (form->Is(RE::FormType::Race)) {
				a_data->otherRace = form->As<RE::TESRace>();
			} else {
				logger::error(
					"{} {:x} is not a valid NPC or Race!",
					utils::GetFormEditorID(form).c_str(),
					form->formID
				);
				return false;
			}
		}
	}

	return true;
}

ConfigurationEntry* ConfigurationEntry::ConstructNewEntry(std::string line)
{
	auto parsingLine = std::string(line);
	ConfigurationEntry::EntryData entryData{ 0 };

	/////////// Default Values /////////////
	entryData.weight = 10;
	entryData.probability = 100;
	////////////////////////////////////////

	// Strip the comments and the whitespace
	if (parsingLine.find('#') != std::string::npos) {
		parsingLine.erase(parsingLine.find('#'));
	}

	// Remove the whitespace
	parsingLine.erase(remove(parsingLine.begin(), parsingLine.end(), ' '), parsingLine.end());

	if (parsingLine.empty() || !IsValidEntry(parsingLine)) {
		// Line is invalid, or was just a comment. Either way, don't parse it
		return nullptr;
	}
	logger::info("Parsing: {}", line);

	auto swapIndex = parsingLine.find("swap=");

	auto matchLine = parsingLine.substr(0, swapIndex);
	auto swapLine = parsingLine.substr(swapIndex);

	bool success = false;
	try {
		success = ConstructMatchData(matchLine, &entryData) && ConstructSwapData(swapLine, &entryData); 
	} catch (...) {
		logger::error("line: \"{}\" is invalid", line);
	}
	

	if (success) {
		auto newEntry = new ConfigurationEntry();
		newEntry->entryData = entryData;
		return newEntry;
	}
	logger::error("line: \"{}\" is invalid", line);
	return nullptr;	
}

bool ConfigurationEntry::MatchesNPC(RE::TESNPC* a_npc) {
	bool isMatch = false;

	isMatch = isMatch || (entryData.npcMatch != nullptr && entryData.npcMatch->formID == a_npc->formID);
	isMatch = isMatch || (entryData.raceMatch != nullptr && entryData.raceMatch->formID == a_npc->race->formID);
	isMatch = isMatch || (entryData.factionMatch != nullptr && a_npc->IsInFaction(entryData.factionMatch));

	// Prevents child NPCs matching for adult swaps and vice-versa
	isMatch = isMatch && (!entryData.otherRace || a_npc->race->IsChildRace() == entryData.otherRace->IsChildRace());
	isMatch = isMatch && (!entryData.otherNPC || a_npc->race->IsChildRace() == entryData.otherNPC->race->IsChildRace());

	if (isMatch) {
		// TODO: Hash should include the entry itself to prevent all entries with the same weight
		// matching the same exact NPCs
		srand((int) utils::HashForm(a_npc));
		isMatch = ((std::uint32_t) rand() % 100) < entryData.probability;
	}

	return isMatch;
}

