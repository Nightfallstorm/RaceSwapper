#pragma once

#include "NPCAppearance.h"
#include "configuration/Configuration.h"
#include "NPCSwap.h"
#include "RaceSwap.h"
#include "Utils.h"

bool NPCAppearance::ApplyNewAppearance(bool updateLoadedActors) {
	if (isNPCSwapped) {
		return false;
	}

	ApplyAppearance(&alteredNPCData);
	// TODO: Update loaded actors
	return true;
}

bool NPCAppearance::RevertNewAppearance(bool updateLoadedActors) {
	if (!isNPCSwapped) {
		return false;
	}

	ApplyAppearance(&originalNPCData);
	// TODO: Update loaded actors
	return true;
}

void NPCAppearance::ApplyAppearance(NPCData* a_data)
{
	isNPCSwapped = true;

	if (a_data->isBeastRace && !npc->HasKeywordID(constants::Keyword_IsBeastRace)) {
		npc->AddKeyword(RE::TESForm::LookupByID(constants::Keyword_IsBeastRace)->As<RE::BGSKeyword>());
	} else {
		// TODO: Can't remove beast keyword from race for NPC without hook?
	}

	npc->height = a_data->height;
	npc->weight = a_data->weight;
	if (a_data->isFemale) {
		npc->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kFemale);
	} else {
		npc->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kFemale);
	}
	npc->bodyTintColor = a_data->bodyTintColor;
	npc->skin = a_data->skin;
	npc->farSkin = a_data->farSkin;

	// skeletonModel applied from hooks for race
	// faceRelatedData applied from hooks for race

	if (npc->tintLayers) {
		npc->tintLayers->clear();
	}

	npc->tintLayers = CopyTintLayers(a_data->tintLayers);

	npc->faceNPC = a_data->faceNPC;
	if (npc->faceNPC == npc) {
		npc->faceNPC = nullptr;
	}

	npc->headRelatedData = CopyHeadRelatedData(a_data->headRelatedData);

	npc->numHeadParts = a_data->numHeadParts;
	npc->headParts = CopyHeadParts(a_data->headParts, a_data->numHeadParts);

	// TODO: Check for default face struct here?
	npc->faceData = DeepCopyFaceData(a_data->faceData);
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
	a_data->isFemale = npc->IsFemale();

	a_data->bodyTintColor = npc->bodyTintColor;

	a_data->tintLayers = CopyTintLayers(npc->tintLayers);
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
	a_data->headRelatedData = CopyHeadRelatedData(a_data->faceNPC->headRelatedData);

	a_data->numHeadParts = a_data->faceNPC->numHeadParts;
	a_data->headParts = CopyHeadParts(a_data->faceNPC->headParts, a_data->faceNPC->numHeadParts);

	// TODO: Check for default face struct here?
	a_data->faceData = DeepCopyFaceData(a_data->faceNPC->faceData);

	a_data->faceRelatedData = a_data->faceNPC->race->faceRelatedData[npc->GetSex()];
}

void NPCAppearance::SetupNewAppearance() {
	RaceSwap::applySwap(&alteredNPCData, config->otherRace.first);
	NPCSwap::applySwap(&alteredNPCData, config->otherNPC.first);
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

void NPCAppearance::dtor() {
	// TODO: Clear/delete the data inside like tint layers, head data, etc.?
}

// Gets or create a new NPC appearance. Will be null if NPC has no altered appearance to take
NPCAppearance* NPCAppearance::GetOrCreateNPCAppearance(RE::TESNPC* a_npc) {
	if (appearanceMap.contains(a_npc->formID)) {
		return appearanceMap.at(a_npc->formID);
	}

	auto config = ConfigurationDatabase::GetSingleton()->GetConfigurationForNPC(a_npc);

	if (config == nullptr) {
		return nullptr;
	}

	logger::info("Creating new appearance for {:x}", a_npc->formID);
	NPCAppearance* appearance = new NPCAppearance(a_npc, config);
	appearanceMap.insert(std::pair(a_npc->formID, appearance));
	return appearance;
};

NPCAppearance* NPCAppearance::GetNPCAppearance(RE::TESNPC* a_npc) {
	if (appearanceMap.contains(a_npc->formID)) {
		return appearanceMap.at(a_npc->formID);
	}

	return nullptr;
};

void NPCAppearance::EraseNPCAppearance(RE::TESNPC* a_npc) {
	EraseNPCAppearance(a_npc->formID);
};

void NPCAppearance::EraseNPCAppearance(RE::FormID a_formID)
{
	if (appearanceMap.contains(a_formID)) {
		appearanceMap.at(a_formID)->dtor();
		appearanceMap.erase(a_formID);
	}
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
