#pragma once

namespace utils
{
	using _GetFormEditorID = const char* (*)(std::uint32_t);

	std::string UniqueStringFromForm(RE::TESForm* a_form_seed);

	size_t HashForm(RE::TESForm* a_form_seed);

	RE::BSTArray<RE::TESNPC::Layer*>* CopyTintLayers(RE::BSTArray<RE::TESNPC::Layer*>* a_tintLayers);

	RE::TESNPC::HeadRelatedData* CopyHeadRelatedData(RE::TESNPC::HeadRelatedData* a_data);

	RE::BGSHeadPart** CopyHeadParts(RE::BGSHeadPart** a_parts, std::uint32_t a_numHeadParts);

	RE::TESNPC::FaceData* DeepCopyFaceData(RE::TESNPC::FaceData* a_faceData);

	std::vector<std::string> split_string(std::string& a_string, char a_delimiter);

	std::string GetFormEditorID(const RE::TESForm* a_form);

	template <class T>
	bool is_amongst(std::vector<T> item_list, T elem)
	{
		return std::find(item_list.begin(), item_list.end(), elem) != item_list.end();
	}

	template <class T>
	bool is_amongst(RE::BSTArray<T> item_list, T elem)
	{
		return std::find(item_list.begin(), item_list.end(), elem) != item_list.end();
	}

	template <class _First_T, class _Second_T>
	bool is_amongst(std::unordered_map<_First_T, _Second_T> item_map, _First_T elem)
	{
		return item_map.find(elem) != item_map.end();
	}

	RE::TESRace* GetValidRaceForArmorRecursive(RE::TESObjectARMO* a_armor, RE::TESRace* a_race);
}

