#pragma once

#include "NPCSwap.h"
#include "Utils.h"

void NPCSwap::applySwap(NPCAppearance::NPCData* a_data, RE::TESNPC* a_otherNPC) {
	if (!a_otherNPC || a_data->baseNPC == a_otherNPC) {
		return;
	}

	auto otherNPCAppearance = NPCAppearance::GetNPCAppearance(a_otherNPC);
	auto otherNPCSwapped = false;
	if (otherNPCAppearance && otherNPCAppearance->isNPCSwapped) {
		// Keep original so we can apply original NPC data
		otherNPCSwapped = true;
		NPCAppearance::GetNPCAppearance(a_otherNPC)->RevertNewAppearance();
	}


	a_data->baseNPC = a_otherNPC;
	a_data->isBeastRace = a_otherNPC->HasKeywordID(constants::Keyword_IsBeastRace) ||
	                      a_otherNPC->race->HasKeywordID(constants::Keyword_IsBeastRace);


	a_data->height = a_otherNPC->height;
	a_data->weight = a_otherNPC->weight;
	a_data->sex = a_otherNPC->GetSex();
	a_data->bodyTintColor = a_otherNPC->bodyTintColor;
	a_data->skin = a_otherNPC->skin ? a_otherNPC->skin : a_otherNPC->race->skin;
	a_data->farSkin = a_otherNPC->farSkin;
	a_data->race = a_otherNPC->race;
	a_data->skeletonModel = &a_otherNPC->race->skeletonModels[a_otherNPC->GetSex()];
	a_data->faceRelatedData = a_otherNPC->race->faceRelatedData[a_otherNPC->GetSex()];
	a_data->bodyPartData = a_otherNPC->race->bodyPartData;
	a_data->bodyTextureModel = &a_otherNPC->race->bodyTextureModels[a_otherNPC->GetSex()];
	a_data->behaviorGraph = &a_otherNPC->race->behaviorGraphs[a_otherNPC->GetSex()];

	if (a_data->tintLayers) {
		a_data->tintLayers->clear();
	}

	a_data->tintLayers = utils::CopyTintLayers(a_otherNPC->tintLayers);

	a_data->faceNPC = a_otherNPC->faceNPC ? a_otherNPC->faceNPC : a_otherNPC;

	a_data->headRelatedData = utils::CopyHeadRelatedData(a_data->faceNPC->headRelatedData);

	a_data->numHeadParts = a_otherNPC->numHeadParts;
	a_data->headParts = utils::CopyHeadParts(a_data->faceNPC->headParts, a_data->faceNPC->numHeadParts);

	// TODO: Check for default face struct here?
	
	a_data->faceData = utils::DeepCopyFaceData(a_data->faceNPC->faceData);

	if (otherNPCAppearance && otherNPCSwapped) {
		otherNPCSwapped = true;
		NPCAppearance::GetNPCAppearance(a_otherNPC)->ApplyNewAppearance();
	}

	logger::info("Swap complete!");
}

