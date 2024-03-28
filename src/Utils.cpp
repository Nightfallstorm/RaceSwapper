#include "Utils.h"
#include "PCH.h"
#include "settings/Settings.h"

namespace utils
{
	std::string UniqueStringFromForm(RE::TESForm* a_form_seed)
	{
		if (!a_form_seed) {
			return std::string();
		}

		auto fileName = "DynamicForm";
		if (!a_form_seed->IsDynamicForm()) {
			fileName = a_form_seed->GetFile()->fileName;
		}

		auto rawFormID = std::to_string(a_form_seed->GetFormID() & 0x00FFFFFF);
		if (!a_form_seed->IsDynamicForm() && a_form_seed->GetFile()->IsLight()) {
			rawFormID = std::to_string(a_form_seed->GetFormID() & 0x00000FFF);
		}

		std::string playthroughID = "";
		if (Settings::GetSingleton()->features.any(Settings::Features::kPlaythroughRandomization)) {
			playthroughID = std::to_string(RE::BGSSaveLoadManager::GetSingleton()->currentPlayerID);
		}

		return rawFormID + "_" + fileName + "_" + playthroughID;
	}

	size_t HashForm(RE::TESForm* a_form_seed)
	{
		std::string data = UniqueStringFromForm(a_form_seed);

		long p = 16777619;
		size_t hash = 2166136261L;
		for (int i = 0; i < data.length(); i++) {
			hash = (hash ^ data[i]) * p;
			hash += hash << 13;
			hash ^= hash >> 7;
			hash += hash << 3;
			hash ^= hash << 17;
			hash += hash >> 5;
		}

		return hash;
	}

	RE::BSTArray<RE::TESNPC::Layer*>* CopyTintLayers(RE::BSTArray<RE::TESNPC::Layer*>* a_tintLayers)
	{
		if (!a_tintLayers) {
			return nullptr;
		}
		auto copiedTintLayers = RE::calloc<RE::BSTArray<RE::TESNPC::Layer*>>(1);

		if (!a_tintLayers->empty()) {
			for (auto tint : *a_tintLayers) {
				auto newLayer = RE::calloc<RE::TESNPC::Layer>(1);
				newLayer->tintColor = tint->tintColor;
				newLayer->tintIndex = tint->tintIndex;
				newLayer->preset = tint->preset;
				newLayer->interpolationValue = tint->interpolationValue;
				copiedTintLayers->emplace_back(newLayer);
			}
		}

		return copiedTintLayers;
	}

	RE::TESNPC::HeadRelatedData* CopyHeadRelatedData(RE::TESNPC::HeadRelatedData* a_data)
	{
		auto newHeadData = RE::calloc<RE::TESNPC::HeadRelatedData>(1);
		if (a_data) {
			newHeadData->hairColor = a_data->hairColor;
			newHeadData->faceDetails = a_data->faceDetails;
		}
		return newHeadData;
	}

	RE::BGSHeadPart** CopyHeadParts(RE::BGSHeadPart** a_parts, std::uint32_t a_numHeadParts)
	{
		if (!a_parts) {
			return nullptr;
		}
		auto newHeadParts = RE::calloc<RE::BGSHeadPart*>(a_numHeadParts);
		for (std::uint32_t index = 0; index < a_numHeadParts; index++) {
			newHeadParts[index] = a_parts[index];
		}
		return newHeadParts;
	}

	RE::TESNPC::FaceData* DeepCopyFaceData(RE::TESNPC::FaceData* a_faceData)
	{
		if (!a_faceData) {
			return nullptr;
		}

		auto newFaceData = RE::calloc<RE::TESNPC::FaceData>(1);

		for (std::uint32_t i = 0; i < 19; i++) {
			newFaceData->morphs[i] = a_faceData->morphs[i];
		}

		for (std::uint32_t i = 0; i < 4; i++) {
			newFaceData->parts[i] = a_faceData->parts[i];
		}
		return newFaceData;
	}

	std::vector<std::string> split_string(std::string& a_string, char a_delimiter)
	{
		std::vector<std::string> list;
		std::string strCopy = a_string;
		size_t pos = 0;
		std::string token;
		while ((pos = strCopy.find(a_delimiter)) != std::string::npos) {
			token = strCopy.substr(0, pos);
			list.push_back(token);
			strCopy.erase(0, pos + 1);
		}
		list.push_back(strCopy);
		return list;
	}

	std::string GetEditorID(RE::FormID a_formID)
	{
		static auto tweaks = GetModuleHandle(L"po3_Tweaks");
		static auto function = reinterpret_cast<_GetFormEditorID>(GetProcAddress(tweaks, "GetFormEditorID"));
		if (function) {
			return function(a_formID);
		}
		return {};
	}

	std::string GetFormEditorID(const RE::TESForm* a_form)
	{
		if (!a_form) {
			return {};
		}
		if (a_form->IsDynamicForm()) {
			return a_form->GetFormEditorID();
		}
		switch (a_form->GetFormType()) {
		case RE::FormType::Keyword:
		case RE::FormType::LocationRefType:
		case RE::FormType::Action:
		case RE::FormType::MenuIcon:
		case RE::FormType::Global:
		case RE::FormType::HeadPart:
		case RE::FormType::Race:
		case RE::FormType::Sound:
		case RE::FormType::Script:
		case RE::FormType::Navigation:
		case RE::FormType::Cell:
		case RE::FormType::WorldSpace:
		case RE::FormType::Land:
		case RE::FormType::NavMesh:
		case RE::FormType::Dialogue:
		case RE::FormType::Quest:
		case RE::FormType::Idle:
		case RE::FormType::AnimatedObject:
		case RE::FormType::ImageAdapter:
		case RE::FormType::VoiceType:
		case RE::FormType::Ragdoll:
		case RE::FormType::DefaultObject:
		case RE::FormType::MusicType:
		case RE::FormType::StoryManagerBranchNode:
		case RE::FormType::StoryManagerQuestNode:
		case RE::FormType::StoryManagerEventNode:
		case RE::FormType::SoundRecord:
			return a_form->GetFormEditorID();
		default:
			return GetEditorID(a_form->GetFormID());
		}
	};

	RE::TESRace* GetValidRaceForArmorRecursive(RE::TESObjectARMO* a_armor, RE::TESRace* a_race)
	{
		if (a_race == nullptr || a_armor == nullptr) {
			return nullptr;
		}
		bool isValidRace = false;
		for (auto addon : a_armor->armorAddons) {
			if (addon && (addon->race == a_race || is_amongst(addon->additionalRaces, a_race))) {
				isValidRace = true;
				break;
			}
		}

		return isValidRace ? a_race : GetValidRaceForArmorRecursive(a_armor, a_race->armorParentRace);
	}

	bool IsVampire(RE::TESNPC* a_npc)
	{
		auto currentRace = a_npc->race;
		return currentRace->HasKeywordID(0xA82BB);  // Vampire keyword
	}

	static inline std::map<RE::TESRace*, RE::TESRace*> GetRaceCompatibilityMap(bool isVampireKey)
	{
		std::map<RE::TESRace*, RE::TESRace*> raceMap;

		auto dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BGSListForm* raceList = nullptr;
		RE::BGSListForm* raceVampireList = nullptr;

		raceList = raceList ? raceList : dataHandler->LookupForm<RE::BGSListForm>(0xD62, "RaceCompatibility.esm");
		raceVampireList = raceVampireList ? raceVampireList : dataHandler->LookupForm<RE::BGSListForm>(0xD63, "RaceCompatibility.esm");

		raceList = raceList ? raceList : RE::TESForm::LookupByEditorID<RE::BGSListForm>("PlayableRaceList");
		raceVampireList = raceVampireList ? raceVampireList : RE::TESForm::LookupByEditorID<RE::BGSListForm>("PlayableVampireList");

		if (!raceList || !raceVampireList ||
			raceList->scriptAddedFormCount != raceVampireList->scriptAddedFormCount ||
			raceList->forms.size() != raceVampireList->forms.size()) {
			return raceMap;
		}

		auto keyList = isVampireKey ? raceVampireList : raceList;
		auto valueList = isVampireKey ? raceList : raceVampireList;

		for (std::uint32_t i = 0; i < raceList->forms.size(); i++) {
			auto key = keyList->forms[i]->As<RE::TESRace>();
			auto value = valueList->forms[i]->As<RE::TESRace>();
			if (key && value) {
				raceMap.emplace(key, value);
			}
			
		}

		if (!keyList->scriptAddedTempForms || !valueList->scriptAddedTempForms) {
			return raceMap;
		}

		RE::TESForm::GetAllForms().second.get().LockForRead();

		auto keyFormList = keyList->scriptAddedTempForms;
		auto valueFormList = valueList->scriptAddedTempForms;

		for (std::uint32_t i = 0; i < raceList->scriptAddedFormCount; i++) {
			auto key = RE::TESForm::LookupByID<RE::TESRace>((*keyFormList)[i]);
			auto value = RE::TESForm::LookupByID<RE::TESRace>((*valueFormList)[i]);
			if (key && value) {
				raceMap.emplace(key, value);
			}
		}

		RE::TESForm::GetAllForms().second.get().UnlockForRead();

		return raceMap;
	}

	static inline RE::TESRace* ConvertRace(RE::TESRace* a_race, bool toVampire)
	{
		auto isVampire = a_race->HasKeywordID(0xA82BB);  // Vampire keyword
		if (isVampire == toVampire) {
			return a_race;
		}

		// First attempt, use race compatibility list to lookup vampire -> non-vampire swaps
		std::map<RE::TESRace*, RE::TESRace*> map = GetRaceCompatibilityMap(!toVampire);
		if (map.contains(a_race)) {
			return map[a_race];
		}

		// Second attempt, use editor ID manipulation to find the vampire race
		auto editorID = utils::GetFormEditorID(a_race);
		auto newEditorID = editorID + "";
		if (toVampire) {
			newEditorID = editorID + "Vampire";
		} else {
			auto vampireIndex = editorID.find("Vampire");
			if (vampireIndex != std::string::npos) {
				newEditorID = editorID.erase(vampireIndex);
			} else {
				newEditorID = "N/A";
			}
		}
		auto race = RE::TESForm::LookupByEditorID(newEditorID);
		if (race && race->As<RE::TESRace>()) {
			return race->As<RE::TESRace>();
		}

		// Fallback, return nord vampire race (this is what RaceCompatibility does as well)
		if (toVampire) {
			return RE::TESForm::LookupByID(0x88794)->As<RE::TESRace>(); // Nord Vampire
		} else {
			return RE::TESForm::LookupByID(0x13746)->As<RE::TESRace>(); // Nord
		}
		
	}

	RE::TESRace* AsNonVampireRace(RE::TESRace* a_race)
	{
		return ConvertRace(a_race, false);
	}

	RE::TESRace* AsVampireRace(RE::TESRace* a_race)
	{
		return ConvertRace(a_race, true);
	}
}
