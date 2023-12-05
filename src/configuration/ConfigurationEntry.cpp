#pragma once
#include "ConfigurationEntry.h"
#include "Utils.h"
#include "MergeMapperPluginAPI.h"

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

template <class T>
T* GetFormFromString(std::string line) {
	auto form = RE::TESForm::LookupByEditorID(line);
	if (form && form->As<T>()) {
		return form->As<T>();
	}

	if (line.find('~') == std::string::npos) {
		logger::error("missing plugin: {}", line);
		return nullptr;
	}

	auto plugin = line.substr(line.find('~') + 1);
	RE::FormID formID = std::stoul(line.substr(0, line.find('~')), nullptr, 16);
	if (g_mergeMapperInterface) {
		auto mergeForm = g_mergeMapperInterface->GetNewFormID(plugin.c_str(), formID);
		plugin = mergeForm.first;
		formID = mergeForm.second;
	}

	form = RE::TESDataHandler::GetSingleton()->LookupForm(formID, plugin);
	if (form == nullptr) {
		logger::error("invalid form ID: {}", line);
		return nullptr;
	}

	if (form->As<T>()) {
		return form->As<T>();
	}
	return nullptr;
}

RE::TESForm* GetFormFromString(std::string line)
{
	return GetFormFromString<RE::TESForm>(line);
}

std::uint32_t GetPercentageFromString(std::string line)
{
	if (line.find('%') != std::string::npos) {
		auto percent = std::stoul(line.substr(0, line.find('%')), nullptr, 10);
		return (std::uint32_t) max(0, min(100, percent));
	}

	return (std::uint32_t) -1;
};

RE::SEX GetSexFromString(std::string line) {
	std::transform(line.begin(), line.end(),
		line.begin(),  // write to the same location
		[](unsigned char c) { return (char) std::toupper(c); });
	
	if (line == "MALE") {
		return RE::SEX::kMale;
	} else if (line == "FEMALE") {
		return RE::SEX::kFemale;
	} else {
		return RE::SEX::kNone;
	}
}

bool ConstructMatchData(std::string line, ConfigurationEntry::EntryData* a_data)
{
	std::string match = "match=";
	line.erase(0, match.size());
	auto filters = utils::split_string(line, '|');
	for (auto& entry : filters) {
		if (auto percent = GetPercentageFromString(entry); percent != (std::uint32_t) -1) {
			a_data->probability = percent;
		} else if (auto sex = GetSexFromString(entry); sex != RE::SEX::kNone) {
			a_data->sexMatch = sex;
		} else if (auto form = GetFormFromString(entry); form && form->Is(RE::FormType::NPC)) {
			a_data->npcMatch = form->As<RE::TESNPC>();
		} else if (form && form->Is(RE::FormType::Race)) {
			a_data->raceMatch = form->As<RE::TESRace>();
		} else if (form && form->Is(RE::FormType::Faction)) {
			a_data->factionMatch = form->As<RE::TESFaction>();
		}
	}

	return true;
}

bool ConstructSwapData(std::string line, ConfigurationEntry::EntryData* a_data)
{
	std::string swap = "swap=";
	line.erase(0, swap.size());
	auto filters = utils::split_string(line, '|');
	for (auto& entry : filters) {
		if (auto percent = GetPercentageFromString(entry); percent != (std::uint32_t) -1) {
			a_data->weight = percent;
		} else if (auto form = GetFormFromString(entry); form && form->Is(RE::FormType::NPC)) {
			a_data->otherNPC = form->As<RE::TESNPC>();
		} else if (form && form->Is(RE::FormType::Race)) {
			a_data->otherRace = form->As<RE::TESRace>();
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

	// TODO: Make this case insensitive
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
	bool isMatch = true;

	isMatch = isMatch && (entryData.sexMatch == RE::SEX::kNone || entryData.sexMatch == a_npc->GetSex());
	isMatch = isMatch && (!entryData.npcMatch || entryData.npcMatch->formID == a_npc->formID);
	isMatch = isMatch && (!entryData.raceMatch || entryData.raceMatch->formID == a_npc->race->formID);
	isMatch = isMatch && (!entryData.factionMatch || a_npc->IsInFaction(entryData.factionMatch));

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

