#pragma once

#include "NPCSwap.h"
#include "Database.h"

// TODO: Switch name

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

RE::TESNPC::HeadRelatedData* CopyHeadRelatedData(RE::TESNPC::HeadRelatedData* a_data) {
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

RE::BGSHeadPart** CopyHeadParts(RE::BGSHeadPart** a_parts, std::uint32_t a_numHeadParts) {
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

RE::TESNPC::FaceData* DeepCopyFaceData(RE::TESNPC::FaceData* a_faceData) {
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

NPCSwapper::NPCSwapper(RE::TESNPC* a_baseNPC)
{
	this->oldNPCData = new NPCData();
	this->CopyNPCData(this->oldNPCData, a_baseNPC);
	this->CopySkins(this->oldNPCData, a_baseNPC);
	this->CopyStats(this->oldNPCData, a_baseNPC);
	this->CopyTints(this->oldNPCData, a_baseNPC);
	this->CopyFaceData(this->oldNPCData, a_baseNPC);
	this->CopySkeletons(this->oldNPCData, a_baseNPC);
	this->CopyBeastKeyword(this->oldNPCData, a_baseNPC);
	this->oldNPCData->valid = true;
	this->currentNPCAppearanceID = a_baseNPC->formID;
}

NPCSwapper* NPCSwapper::GetOrPutNPCSwapper(RE::FormID a_formID)
{
	auto form = RE::TESForm::LookupByID(a_formID);

	if (!form || (!form->As<RE::TESNPC>() && !form->As<RE::Character>())) {
		return nullptr;
	}

	if (form->As<RE::TESNPC>()) {
		return GetOrPutNPCSwapper(form->As<RE::TESNPC>());
	} else {
		return GetOrPutNPCSwapper(form->As<RE::Character>());
	}
};

NPCSwapper* NPCSwapper::GetOrPutNPCSwapper(RE::Character* a_refr){
	if (!a_refr || !a_refr->GetBaseObject() || !a_refr->GetBaseObject()->As<RE::TESNPC>()) {
		return nullptr;
	}
	auto baseNPC = a_refr->GetBaseObject()->As<RE::TESNPC>();
	return GetOrPutNPCSwapper(baseNPC);
};

NPCSwapper* NPCSwapper::GetOrPutNPCSwapper(RE::TESNPC* a_npc)
{
	if (!a_npc) {
		return nullptr;
	}
	if (!NPCSwapMap.contains(a_npc->formID)) {
		NPCSwapMap.emplace(a_npc->formID, new NPCSwapper(a_npc));
	}
	return NPCSwapMap.at(a_npc->formID);
};

NPCSwapper* NPCSwapper::GetNPCSwapper(RE::FormID a_formID)
{
	if (!NPCSwapMap.contains(a_formID)) {
		return nullptr;
	}
	return NPCSwapMap.at(a_formID);
};

void NPCSwapper::RemoveNPCSwapper(RE::FormID a_formID) {
	if (!NPCSwapMap.contains(a_formID)) {
		return;
	}
	delete NPCSwapMap.at(a_formID);
	NPCSwapMap.erase(a_formID);
}

void NPCSwapper::SetupNewNPCSwap(NPCData* a_newNPCData)
{
	if (newNPCData && newNPCData->baseNPC->formID == a_newNPCData->baseNPC->formID) {
		logger::info("Already have new NPC data, ignoring");
		return;
	}
	newNPCData = a_newNPCData;
}

void NPCSwapper::Apply()
{
	assert(oldNPCData->valid);
	assert(newNPCData->valid);
	if (currentNPCAppearanceID != oldNPCData->baseNPC->formID) {
		//RE::DebugMessageBox("ID not as expected!!!!");
		//return;
	}
	if (currentNPCAppearanceID != newNPCData->baseNPC->formID) {
		logger::info("Apply swap from {:x} NPC as {:x} to {:x}", oldNPCData->baseNPC->formID, currentNPCAppearanceID, newNPCData->baseNPC->formID);
		ApplyNPCData(newNPCData, oldNPCData->baseNPC);		
		currentNPCAppearanceID = newNPCData->baseNPC->formID;
	}
}

void NPCSwapper::Revert()
{
	assert(oldNPCData->valid);
	assert(newNPCData->valid);
	if (this->currentNPCAppearanceID != oldNPCData->baseNPC->formID) {
		logger::info("Reverting swap from {:x} to {:x}", currentNPCAppearanceID, oldNPCData->baseNPC->formID);
		ApplyNPCData(oldNPCData, oldNPCData->baseNPC);
		this->currentNPCAppearanceID = oldNPCData->baseNPC->formID;
	}	
}

// Native Papyrus function version of enable
static void ObjectReference__Enable(RE::TESObjectREFR* a_self, bool a_abFadeIn, bool a_wait, RE::BSScript::Internal::VirtualMachine* a_vm, RE::VMStackID a_stackID)
{
	using func_t = decltype(&ObjectReference__Enable);
	REL::Relocation<func_t> func{ RELOCATION_ID(56038, 56158) };
	return func(a_self, a_abFadeIn, a_wait, a_vm, a_stackID);
}

void NPCSwapper::ApplySwapToReference(RE::Character* a_refr, NPCSwapper* a_npcToSwapTo, bool fullReset, bool revertToOriginalAppearance)
{
	auto refrSwapper = NPCSwapper::GetOrPutNPCSwapper(a_refr);
	if (revertToOriginalAppearance) {
		refrSwapper->Revert();
	} else {
		refrSwapper->SetupNewNPCSwap(a_npcToSwapTo->oldNPCData);
		refrSwapper->Apply();
	}

	if (!a_refr->Is3DLoaded()) {
		return;
	}

	if (!fullReset) {
		a_refr->DoReset3D(true);
		return;
	}

	RE::ObjectRefHandle origParentHandle;
	RE::ExtraEnableStateParent* enableStateParent = nullptr;
	// Remove enable state parent temporarily if it exists, so we can disable/enable freely to refresh the NPC
	enableStateParent = a_refr->extraList.GetByType<RE::ExtraEnableStateParent>();
	if (enableStateParent) {
		origParentHandle = enableStateParent->parent;
		enableStateParent->parent = RE::ObjectRefHandle();
	}
	a_refr->Disable();
	ObjectReference__Enable(a_refr, false, false, RE::BSScript::Internal::VirtualMachine::GetSingleton(), 0);
	if (enableStateParent) {
		enableStateParent->parent = origParentHandle;
	}
	
}

void NPCSwapper::ApplyNPCData(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	assert(a_baseNPC);

	if (a_data->isBeastRace && !a_baseNPC->HasKeywordID(constants::IsBeastRace)) {
		a_baseNPC->AddKeyword(RE::TESForm::LookupByID(constants::IsBeastRace)->As<RE::BGSKeyword>());
	} else {
		// TODO: Can't remove beast keyword from race for NPC without hook?
	}	

	
	// TODO: Make this new function as stats that don't need deep copying
	{
		a_baseNPC->height = a_data->height;
		a_baseNPC->weight = a_data->weight;
		if (a_data->isFemale) {
			a_baseNPC->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kFemale);
		} else {
			a_baseNPC->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kFemale);
		}
		a_baseNPC->bodyTintColor = a_data->bodyTintColor;
		a_baseNPC->skin = a_data->skin;
		a_baseNPC->farSkin = a_data->farSkin;
	}
	

	// skeletonModel applied from hooks
	// faceRelatedData applied from hooks

	if (a_baseNPC->tintLayers) {
		a_baseNPC->tintLayers->clear();
	}
	
	a_baseNPC->tintLayers = CopyTintLayers(a_data->tintLayers);

	a_baseNPC->faceNPC = a_data->faceNPC;
	if (a_baseNPC->faceNPC == a_baseNPC) {
		a_baseNPC->faceNPC = nullptr;
	}
	
	a_baseNPC->headRelatedData = CopyHeadRelatedData(a_data->headRelatedData);


	a_baseNPC->numHeadParts = a_data->numHeadParts;
	a_baseNPC->headParts = CopyHeadParts(a_data->headParts, a_data->numHeadParts);

	// TODO: Check for default face struct here?
	a_baseNPC->faceData = DeepCopyFaceData(a_data->faceData);	

	logger::info("Swap complete!");
}

void NPCSwapper::CopyNPCData(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->baseNPC = a_baseNPC;
	a_data->faceNPC = a_baseNPC->faceNPC;
	a_data->race = a_baseNPC->race;
}

void NPCSwapper::CopySkins(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	auto& newSkin = a_baseNPC->skin ? a_baseNPC->skin : a_baseNPC->race->skin;
	auto& newFarSkin = a_baseNPC->farSkin;

	a_data->skin = newSkin; 
	a_data->farSkin = newFarSkin;
}

void NPCSwapper::CopyStats(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->weight = a_baseNPC->weight;
	a_data->height = a_baseNPC->height;
	a_data->isFemale = a_baseNPC->IsFemale();
}

void NPCSwapper::CopyTints(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->bodyTintColor = a_baseNPC->bodyTintColor;

	if (a_data->tintLayers) {
		a_data->tintLayers->clear();
	}
	a_data->tintLayers = CopyTintLayers(a_baseNPC->tintLayers);
}

void NPCSwapper::CopyFaceData(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	auto memoryManager = RE::MemoryManager::GetSingleton();

	if (a_data->headRelatedData) {
		memoryManager->Deallocate(a_data->headRelatedData, 0);
	}
	a_data->headRelatedData = CopyHeadRelatedData(a_baseNPC->headRelatedData);


	a_data->numHeadParts = a_baseNPC->numHeadParts;
	a_data->headParts = CopyHeadParts(a_baseNPC->headParts, a_data->numHeadParts);

	// TODO: Check for default face struct here?
	a_data->faceData = DeepCopyFaceData(a_baseNPC->faceData);
	
	a_data->faceRelatedData = a_baseNPC->race->faceRelatedData[a_baseNPC->GetSex()];
}
void NPCSwapper::CopySkeletons(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->skeletonModel = &a_baseNPC->race->skeletonModels[a_baseNPC->GetSex()];
}
void NPCSwapper::CopyBeastKeyword(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->isBeastRace = a_baseNPC->HasKeywordID(constants::IsBeastRace) ||
	                      a_baseNPC->race->HasKeywordID(constants::IsBeastRace);
}
