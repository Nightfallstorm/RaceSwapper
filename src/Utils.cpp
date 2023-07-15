#include "Utils.h"
#include "PCH.h"

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
		return std::to_string(a_form_seed->GetFormID() & 0x00FFFFFF) + "_" + fileName;
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
		auto memoryManager = RE::MemoryManager::GetSingleton();
		auto copiedTintLayers = new RE::BSTArray<RE::TESNPC::Layer*>(*a_tintLayers);

		copiedTintLayers->clear();
		if (!a_tintLayers->empty()) {
			for (auto tint : *a_tintLayers) {
				auto newLayer = reinterpret_cast<RE::TESNPC::Layer*>(memoryManager->Allocate(0xC, 0, 0));
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
		auto memoryManager = RE::MemoryManager::GetSingleton();
		if (a_data && (a_data->hairColor || a_data->faceDetails)) {
			auto newHeadData = reinterpret_cast<RE::TESNPC::HeadRelatedData*>(memoryManager->Allocate(0x10, 0, 0));
			newHeadData->hairColor = 0;
			newHeadData->faceDetails = 0;
			return newHeadData;
		} else {
			return nullptr;
		}
	}

	RE::BGSHeadPart** CopyHeadParts(RE::BGSHeadPart** a_parts, std::uint32_t a_numHeadParts)
	{
		auto memoryManager = RE::MemoryManager::GetSingleton();
		if (!a_parts) {
			return nullptr;
		}

		auto newHeadParts = reinterpret_cast<RE::BGSHeadPart**>(memoryManager->Allocate(sizeof(void*) * a_numHeadParts, 0, 0));
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

		auto memoryManager = RE::MemoryManager::GetSingleton();
		auto newFaceData = reinterpret_cast<RE::TESNPC::FaceData*>(memoryManager->Allocate(0x5c, 0, 0));
		for (std::uint32_t i = 0; i < 19; i++) {
			newFaceData->morphs[i] = a_faceData->morphs[i];
		}

		for (std::uint32_t i = 0; i < 4; i++) {
			newFaceData->parts[i] = a_faceData->parts[i];
		}
		return newFaceData;
	}
}

