#pragma once
#include <string>
#include <algorithm>
#include <vector>
#include <functional>
#include <unordered_map>
#include <map>
#include "RaceSwapUtils.h"

namespace raceswap
{
	class DataBase
	{
	public:

		enum HDPTType : std::uint32_t
		{
			Mouth = 1 << 0,
			Beard = 1 << 1,
			Hair = 1 << 2,
			HairLine = 1 << 3,
			Eyes = 1 << 4,
			Scar = 1 << 5,
			Brow = 1 << 6,
			Earring = 1 << 7,
			Mustache = 1 << 8,
			Ear = 1 << 9,
			Mark = 1 << 10,
			Horn = 1 << 11,
		};

		static inline std::map<std::string, HDPTType> HDPTTypeMap{ 
			std::pair("Mouth", Mouth),
			std::pair("Beard", Beard),
			std::pair("Hair", Hair),
			std::pair("HairLine", HairLine),
			std::pair("Scar", Scar),
			std::pair("Brow", Brow),
			std::pair("Earring", Earring),
			std::pair("Mustache", Mustache),
			std::pair("Ear", Ear),
			std::pair("Mark", Mark),
			std::pair("Horn", Horn),
		};

		enum HDPTCharacteristics : std::uint32_t
		{
			// TODO: Finish filling this
			Blind = 1 << 0,
			Left = 1 << 1,
			Right = 1 << 2,
			NoHair = 1 << 4,
			Demon = 1 << 5,
			Vampire = 1 << 6,
			Shaved = 1 << 7,
			NoScar = 1 << 8,
			Gash = 1 << 9,
			NoGash = 1 << 10,
			Small = 1 << 11,
			Medium = 1 << 12,
			Long = 1 << 13,
			NoBeard = 1 << 14,
			Narrow = 1 << 15,
		};

		static inline std::map<std::string, HDPTCharacteristics> HDPTCharMap
		{
			std::pair("Blind", Blind),
			std::pair("Left", Left),
			std::pair("Right", Right),
			std::pair("NoHair", NoHair),
			std::pair("Demon", Demon),
			std::pair("Vampire", Vampire),
			std::pair("Shaved", Shaved),
			std::pair("NoScar", NoScar),
			std::pair("Gash", Gash),
			std::pair("NoGash", NoGash),
			std::pair("Small", Small),
			std::pair("Medium", Medium),
			std::pair("Long", Long),
			std::pair("NoBeard", NoBeard),
			std::pair("Narrow", Narrow),
		};

		enum HDPTColor : std::uint32_t
		{
			Hazel = 1 << 0,
			Brown = 1 << 1,
			Yellow = 1 << 2,
			Blue = 1 << 3,
			Light = 1 << 4,
			Dark = 1 << 5,
			Ice = 1 << 6,
			Orange = 1 << 7,
			Green = 1 << 8,
			Grey = 1 << 9,
			Red = 1 << 10,
			Deep = 1 << 11,
			Violet = 1 << 12,
			Bright = 1 << 13,
			Aqua = 1 << 14,
			Silver = 1 << 15,
			Olive = 1 << 16,
		};

		static inline std::map<std::string, HDPTColor> HDPTColorMap{
			std::pair("Hazel", Hazel),
			std::pair("Brown", Brown),
			std::pair("Yellow", Yellow),
			std::pair("Blue", Blue),
			std::pair("Light", Light),
			std::pair("Dark", Dark),
			std::pair("Ice", Ice),
			std::pair("Orange", Orange),
			std::pair("Green", Green),
			std::pair("Grey", Grey),
			std::pair("Red", Red),
			std::pair("Deep", Deep),
			std::pair("Violet", Violet),
			std::pair("Bright", Bright),
			std::pair("Aqua", Aqua),
			std::pair("Silver", Silver),
			std::pair("Olive", Olive),
		};

		// HeadPartType , Characteristics, Color
		using HDPTData = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;
		
		using _likelihood_t = uint8_t;

		using HeadPartType = RE::BGSHeadPart::HeadPartType;

		using TintType = RE::TESRace::FaceRelatedData::TintAsset::TintLayer::SkinTone;

		std::vector<RE::TESRace*> valid_races;

		std::unordered_map<RE::FormID, std::string_view> formsToEditorIDs;

		std::unordered_map<RE::SEX, std::unordered_map<HeadPartType, std::unordered_map<RE::TESRace*, std::vector<RE::BGSHeadPart*>>>> valid_type_race_headpart_map;

		std::unordered_map<RE::SEX, std::unordered_map<HeadPartType, std::unordered_map<RE::TESRace*, std::vector<HDPTData*>>>> valid_type_race_HDPTdata_map;

		std::unordered_map<RE::SEX, std::unordered_map<RE::TESRace*, RE::TESRace::FaceRelatedData::TintAsset*>> default_skintint_for_each_race;

		bool dumplists;

		inline static DataBase* & GetSingleton(bool _dumplists = true)
		{
			static DataBase* _this_database = nullptr;
			if (!_this_database)
				_this_database = new DataBase(_dumplists);
			return _this_database;
		}

		HDPTData* FindOrCalculateHDPTData(RE::BGSHeadPart* hdpt)
		{
			auto iter = _hdptd_cache.find(hdpt);
			if (iter == _hdptd_cache.end()) {
				HDPTData* hdptd_ptr = new HDPTData(util::ExtractKeywords(hdpt));
				_hdptd_cache[hdpt] = hdptd_ptr;
				return hdptd_ptr;
			} else {
				return iter->second;
			}
		}

		inline std::vector<RE::BGSHeadPart*> GetHeadParts(HeadPartType type, RE::SEX sex, RE::TESRace* race)
		{
			if (util::is_amongst(valid_type_race_headpart_map, sex)) {
				if (util::is_amongst(valid_type_race_headpart_map[sex], type)) {
					if (util::is_amongst(valid_type_race_headpart_map[sex][type], race)) {
						return valid_type_race_headpart_map[sex][type][race];
					}
				}
			}
			return std::vector<RE::BGSHeadPart*>();
		}

		inline std::vector<HDPTData*> GetHDPTData(HeadPartType type, RE::SEX sex, RE::TESRace* race)
		{
			if (util::is_amongst(valid_type_race_HDPTdata_map, sex)) {
				if (util::is_amongst(valid_type_race_HDPTdata_map[sex], type)) {
					if (util::is_amongst(valid_type_race_HDPTdata_map[sex][type], race)) {
						return valid_type_race_HDPTdata_map[sex][type][race];
					}
				}
			}
			return std::vector<HDPTData*>();
		}

		inline std::vector<RE::BGSHeadPart*> GetMatchedResults(HeadPartType type, RE::SEX sex, RE::TESRace* race, RE::BGSHeadPart* hdpt) {
			auto hdpts = GetHeadParts(type, sex, race);
			auto hdptd = GetHDPTData(type, sex, race);
			auto dst_hdptd = FindOrCalculateHDPTData(hdpt);
			return util::MatchHDPTData(*dst_hdptd, hdpts, hdptd);
		}

		inline std::vector<RE::BGSHeadPart*> GetMatchedResults(HeadPartType type, RE::SEX sex, RE::TESRace* race, HDPTData hdptdata)
		{
			auto hdpts = GetHeadParts(type, sex, race);
			auto hdptd = GetHDPTData(type, sex, race);
			return util::MatchHDPTData(hdptdata, hdpts, hdptd);
		}

		RE::TESRace::FaceRelatedData::TintAsset* GetRaceSkinTint(RE::SEX sex, RE::TESRace* race) {
			return default_skintint_for_each_race[sex][race];
		}

		RE::TESRace* GetDefaultRace() {
			return RE::TESForm::LookupByID(0x19)->As<RE::TESRace>();
		}

		const char* GetFormEditorID(RE::FormID formid) {
			auto iter = formsToEditorIDs.find(formid);
			if (iter != formsToEditorIDs.end()) {
				return iter->second.data();
			}
			const char* empty_char = std::string_view("").data();
			return empty_char;
		}

		bool IsValidHeadPart(RE::BGSHeadPart* hdpt)
		{
			if (hdpt->extraParts.empty() && hdpt->model.empty() && std::string(hdpt->GetFormEditorID()).find("No") == std::string::npos) {
				return false;
			}

			if (hdpt->validRaces == nullptr) {
				return false;
			}

			return true;
		}


		/*
		@brief Safely de-allocate the memory space used by DataBase.
		*/
		static void Dealloc() {
			delete GetSingleton();
			GetSingleton() = nullptr;
		}

	private:
		DataBase(bool _dumplists): dumplists(_dumplists)
		{
			_initialize();
		}

		template<class Elem_T>
		void _dump_list(std::vector<Elem_T*> list, const char* name, int indent = 2)
		{
			logger::info("{}", name);

			std::string spaces;
			while (indent--) {
				spaces += " ";
			}

			for (auto item : list) {
				if (item) {
					std::string_view a = GetFormEditorID(item->formID);
					logger::info("{}EditorID: {}, FormID: {:x}", spaces.c_str(), a, item->GetFormID());
				}
				else
					logger::info("{}[nullptr]", spaces.c_str());
			}
		}

		void _initialize() {
			//To do
			auto [editorIDMap, editorIDLock] = RE::TESForm::GetAllFormsByEditorID();

			editorIDLock.get().LockForRead();
			for (auto const& [editorID, form] : *editorIDMap) {
				/*if (form->Is(RE::FormType::NPC))*/
				formsToEditorIDs[form->formID] = editorID;
			}
			editorIDLock.get().UnlockForRead();

			auto const& [map, lock] = RE::TESForm::GetAllForms();
			
			lock.get().LockForRead();

			logger::info("Begin categorizing...");

			//Categorizing head parts and calculate their discriptors (HDPTData) for matching
			for (auto const& [formid, form] : *map) {
				//To do
				if ((formid & 0xFF000000) == 0xFF000000)
					continue;

				if (form->Is(RE::FormType::HeadPart)) {
					auto hdpt = form->As<RE::BGSHeadPart>();

					auto _append_to_list_male = [hdpt, this](RE::TESForm& form) { 
						valid_type_race_headpart_map[RE::SEX::kMale][hdpt->type.get()][form.As<RE::TESRace>()].push_back(hdpt);
						valid_type_race_HDPTdata_map[RE::SEX::kMale][hdpt->type.get()][form.As<RE::TESRace>()].push_back(FindOrCalculateHDPTData(hdpt));
						return RE::BSContainer::ForEachResult::kContinue;
					};
					auto _append_to_list_female = [hdpt, this](RE::TESForm& form) {
						valid_type_race_headpart_map[RE::SEX::kFemale][hdpt->type.get()][form.As<RE::TESRace>()].push_back(hdpt);
						valid_type_race_HDPTdata_map[RE::SEX::kFemale][hdpt->type.get()][form.As<RE::TESRace>()].push_back(FindOrCalculateHDPTData(hdpt));
						return RE::BSContainer::ForEachResult::kContinue;
					};

					if (IsValidHeadPart(hdpt)) {
						if (hdpt->flags.any(RE::BGSHeadPart::Flag::kMale)) {
							hdpt->validRaces->ForEachForm(_append_to_list_male);
						} else if (hdpt->flags.any(RE::BGSHeadPart::Flag::kFemale)) {
							hdpt->validRaces->ForEachForm(_append_to_list_female);
						}
					}
				} else if (form->Is(RE::FormType::Race)) {
					if (form->As<RE::TESRace>()->faceRelatedData[RE::SEX::kMale] != nullptr && 
						form->As<RE::TESRace>()->faceRelatedData[RE::SEX::kMale]->tintMasks != nullptr) {
						// TODO: Could we support animal races with no face related data?
						// TODO: We could make this cleaner/simpler
						valid_races.push_back(form->As<RE::TESRace>());
					}
					
				}
			}

			//Collecting other assets
			for (auto race : valid_races) {

				auto race_male_tints = race->faceRelatedData[RE::SEX::kMale]->tintMasks;

				auto race_female_tints = race->faceRelatedData[RE::SEX::kFemale]->tintMasks;

				for (auto race_tint : *race_male_tints) {
					if (race_tint->texture.skinTone == DataBase::TintType::kSkinTone) {
						this->default_skintint_for_each_race[RE::SEX::kMale][race] = race_tint;
						break;
					}
				}
				
				if (!this->default_skintint_for_each_race[RE::SEX::kMale][race]) {
					logger::info("  Race: {} has no skin tone tint layer for male.", race->GetFormEditorID());
				}

				for (auto race_tint : *race_female_tints) {
					if (race_tint->texture.skinTone == DataBase::TintType::kSkinTone) {
						this->default_skintint_for_each_race[RE::SEX::kFemale][race] = race_tint;
						break;
					}
				}

				if (!this->default_skintint_for_each_race[RE::SEX::kFemale][race]) {
					logger::info("  Race: {} has no skin tone tint layer for female.", race->GetFormEditorID());
				}
			}
			lock.get().UnlockForRead();

			logger::info("Finished categorizing...");
			//if (dumplists)
				// TODO:
				//_dump_info();

		}

		void _dump_info() {
			_dump_list(valid_races, "Valid Races: ");

			logger::info("Valid list:");
			for (auto sub_list1 : valid_type_race_headpart_map) {
				for (auto sub_list2 : sub_list1.second) {
					for (auto sub_list3 : sub_list2.second) {
						std::string list_name;
						list_name += sub_list1.first ? "  Female " : "  Male ";
						if (sub_list2.first == HeadPartType::kHair)
							list_name += "Hair ";
						else if (sub_list2.first == HeadPartType::kEyes)
							list_name += "Eyes ";
						else if (sub_list2.first == HeadPartType::kScar)
							list_name += "Scars ";
						else if (sub_list2.first == HeadPartType::kFacialHair)
							list_name += "Beards ";

						list_name += sub_list3.first->GetFormEditorID();

						_dump_list(sub_list3.second, list_name.c_str(), 4);
					}
				}
			}
		}

		std::unordered_map<RE::BGSHeadPart*, HDPTData*> _hdptd_cache;

	};

}
