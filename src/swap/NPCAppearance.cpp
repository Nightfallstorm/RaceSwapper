#pragma once

#include "NPCAppearance.h"
#include "configuration/Configuration.h"
#include "NPCSwap.h"
#include "RaceSwap.h"
#include "SexSwap.h"
#include "Utils.h"

static void UpdateLoadedActors(RE::TESNPC* a_npc) {
	for (auto actorHandle : RE::ProcessLists::GetSingleton()->highActorHandles) {
		RE::Actor* actor = actorHandle.get().get();
		if (actor && actor->GetActorBase() == a_npc && actor->Is3DLoaded()) {
			logger::info("Updated loaded actor {:x} NPC {}{:x}",
				actor->formID,
				utils::GetFormEditorID(a_npc),
				a_npc->formID
			);
			actor->GetActorRuntimeData().currentProcess->Update3DModel(actor);
		}
	}
}

bool NPCAppearance::ApplyNewAppearance(bool updateLoadedActors)
{
	if (isNPCSwapped) {
		return false;
	}

	ApplyAppearance(&alteredNPCData);
	if (updateLoadedActors) {
		UpdateLoadedActors(npc);
	}
	
	isNPCSwapped = true;
	return true;
}

bool NPCAppearance::RevertNewAppearance(bool updateLoadedActors)
{
	if (!isNPCSwapped) {
		return false;
	}

	ApplyAppearance(&originalNPCData);
	if (updateLoadedActors) {
		UpdateLoadedActors(npc);
	}

	isNPCSwapped = false;
	return true;
}

void NPCAppearance::ApplyAppearance(NPCData* a_data)
{
	npc->height = a_data->height;
	npc->weight = a_data->weight;
	if (a_data->sex == RE::SEX::kFemale) {
		npc->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kFemale);
	} else {
		npc->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kFemale);
	}
	npc->bodyTintColor = a_data->bodyTintColor;
	npc->skin = a_data->skin;
	npc->farSkin = a_data->farSkin;

	// skeletonModel applied from hooks for race
	// faceRelatedData applied from hooks for race
	// isBeastRace keyword applied from hooks for race

	if (npc->tintLayers) {
		npc->tintLayers->clear();
	}

	npc->tintLayers = utils::CopyTintLayers(a_data->tintLayers);

	npc->faceNPC = a_data->faceNPC;
	if (npc->faceNPC == npc) {
		npc->faceNPC = nullptr;
	}

	npc->headRelatedData = utils::CopyHeadRelatedData(a_data->headRelatedData);

	npc->numHeadParts = a_data->numHeadParts;
	npc->headParts = utils::CopyHeadParts(a_data->headParts, a_data->numHeadParts);

	// TODO: Check for default face struct here?
	npc->faceData = utils::DeepCopyFaceData(a_data->faceData);
}

void NPCAppearance::InitializeNPCData(NPCData* a_data)
{
	a_data->baseNPC = npc;
	a_data->faceNPC = npc->faceNPC ? npc->faceNPC : npc;
	a_data->race = npc->race;

	a_data->skin = npc->skin ? npc->skin : npc->race->skin;
	a_data->farSkin = npc->farSkin;

	a_data->weight = npc->weight;
	a_data->height = npc->height;
	a_data->sex = npc->GetSex();

	a_data->bodyTintColor = npc->bodyTintColor;

	a_data->tintLayers = utils::CopyTintLayers(npc->tintLayers);
	CopyFaceData(a_data);
	a_data->skeletonModel = &npc->race->skeletonModels[npc->GetSex()];

	a_data->isBeastRace = npc->HasKeywordID(constants::Keyword_IsBeastRace) ||
	                         npc->race->HasKeywordID(constants::Keyword_IsBeastRace);

	a_data->bodyPartData = npc->race->bodyPartData;

	a_data->bodyTextureModel = &npc->race->bodyTextureModels[npc->GetSex()];
	a_data->behaviorGraph = &npc->race->behaviorGraphs[npc->GetSex()];

}

void NPCAppearance::CopyFaceData(NPCData* a_data)
{
	auto memoryManager = RE::MemoryManager::GetSingleton();

	if (a_data->headRelatedData) {
		memoryManager->Deallocate(a_data->headRelatedData, 0);
	}
	a_data->headRelatedData = utils::CopyHeadRelatedData(a_data->faceNPC->headRelatedData);

	a_data->numHeadParts = a_data->faceNPC->numHeadParts;
	a_data->headParts = utils::CopyHeadParts(a_data->faceNPC->headParts, a_data->faceNPC->numHeadParts);

	// TODO: Check for default face struct here?
	a_data->faceData = utils::DeepCopyFaceData(a_data->faceNPC->faceData);

	a_data->faceRelatedData = a_data->faceNPC->race->faceRelatedData[npc->GetSex()];
}

void NPCAppearance::SetupNewAppearance() {
	RaceSwap::applySwap(&alteredNPCData, config->otherRace);
	NPCSwap::applySwap(&alteredNPCData, config->otherNPC);
	SexSwap::applySwap(&alteredNPCData, config->otherSex);
	// TODO add more swaps here
}

NPCAppearance::NPCAppearance(RE::TESNPC* a_npc, AppearanceConfiguration* a_config)
{
	this->npc = a_npc;
	this->config = a_config;
	logger::info("	Creating new NPC data");
	InitializeNPCData(&this->originalNPCData);
	InitializeNPCData(&this->alteredNPCData);
	logger::info("	Setting up appearance");
	SetupNewAppearance();
}

void ClearNPCAppearanceData(NPCAppearance::NPCData a_data) {
	RE::free(a_data.faceData);
	RE::free(a_data.headParts);
	RE::free(a_data.headRelatedData);
	if (a_data.tintLayers) {
		for (auto layer : *a_data.tintLayers) {
			RE::free(layer);
		}
	}
	RE::free(a_data.tintLayers);	
}

void NPCAppearance::dtor() {
	ClearNPCAppearanceData(alteredNPCData);
	ClearNPCAppearanceData(originalNPCData);
}

// Filter for only NPCs this swapping can work on
static bool IsNPCValid(RE::TESNPC* a_npc)
{
	return a_npc && a_npc->race &&
	       !a_npc->IsPlayer() &&
	       !a_npc->IsPreset() /* &&
	       a_npc->race->HasKeywordID(constants::Keyword_ActorTypeNPC) */;
}

// Gets or create a new NPC appearance. Will be null if NPC has no altered appearance to take
NPCAppearance* NPCAppearance::GetOrCreateNPCAppearance(RE::TESNPC* a_npc) {
	auto faceNPC = a_npc->GetRootFaceNPC();
	if (!IsNPCValid(a_npc)) {
		return nullptr;
	}
	appearanceMapLock.lock();
	if (appearanceMap.contains(a_npc->formID)) {
		appearanceMapLock.unlock();
		return appearanceMap.at(a_npc->formID);
	}

	// Template actors are based on a face NPC. Always use face NPC as original appearance to get configuration for
	auto config = ConfigurationDatabase::GetSingleton()->GetConfigurationForNPC(faceNPC); 

	if (config == nullptr) {
		logger::debug("No appearance config for {:x} face NPC: {:x}", a_npc->formID, faceNPC->formID);
		appearanceMapLock.unlock();
		return nullptr;
	}

	logger::debug("NPC {:x} matched entry \"{}\" from file \"{}\"", a_npc->formID, config->entry, config->file);
	logger::info("Creating new appearance for {:x}. Face NPC used for appearance: {:x}", a_npc->formID, faceNPC->formID);
	NPCAppearance* appearance = new NPCAppearance(a_npc, config);
	appearanceMap.insert(std::pair(a_npc->formID, appearance));
	appearanceMapLock.unlock();
	return appearance;
};

// Templated actors rely on the face NPC for swaps, so our appearance data will be based on the faceNPC as well
NPCAppearance* NPCAppearance::GetNPCAppearance(RE::TESNPC* a_npc) {
	appearanceMapLock.lock();
	if (appearanceMap.contains(a_npc->formID)) {
		appearanceMapLock.unlock();
		return appearanceMap.at(a_npc->formID);
	}
	appearanceMapLock.unlock();
	return nullptr;
};

void NPCAppearance::EraseNPCAppearance(RE::TESNPC* a_npc) {
	EraseNPCAppearance(a_npc->formID);
};

void NPCAppearance::EraseNPCAppearance(RE::FormID a_formID)
{
	appearanceMapLock.lock();
	if (appearanceMap.contains(a_formID)) {
		auto appearance = appearanceMap.at(a_formID);
		appearanceMap.erase(a_formID);
		appearance->dtor();
		delete appearance;
		
	}
	appearanceMapLock.unlock();
};

// Native Papyrus function version of enable
static void ObjectReference__Enable(RE::TESObjectREFR* a_self, bool a_abFadeIn, bool a_wait, RE::BSScript::Internal::VirtualMachine* a_vm, RE::VMStackID a_stackID)
{
	using func_t = decltype(&ObjectReference__Enable);
	REL::Relocation<func_t> func{ RELOCATION_ID(56038, 56158) };
	return func(a_self, a_abFadeIn, a_wait, a_vm, a_stackID);
}

void ResetCharacter(RE::Character* a_refr)
{
	if (a_refr == nullptr) {
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
