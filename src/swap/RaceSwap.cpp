#include "PCH.h"
#include "RaceSwapUtils.h"
#include "RaceSwap.h"
#include "RaceSwapDatabase.h"

void RaceSwap::applySwap(NPCAppearance::NPCData* a_data, RE::TESRace* a_otherRace) {
	if (!a_otherRace || a_data->race == a_otherRace) {
		// Don't do anything if no other race present, or NPC is already said race
		return;
	}
	auto originalRace = a_data->race;
	a_data->race = a_otherRace;
	a_data->skeletonModel = &a_otherRace->skeletonModels[a_data->isFemale];
	a_data->isBeastRace = a_otherRace->HasKeywordID(constants::Keyword_IsBeastRace);
	a_data->skin = a_otherRace->skin;
	a_data->faceRelatedData = a_otherRace->faceRelatedData[a_data->isFemale];
	a_data->bodyPartData = a_otherRace->bodyPartData;
	a_data->bodyTextureModel = &a_otherRace->bodyTextureModels[a_data->isFemale];
	a_data->behaviorGraph = &a_otherRace->behaviorGraphs[a_data->isFemale];

	util::RandomGen<RE::TESForm> rand_generator(a_data->baseNPC->GetAsForm(), util::UniqueStringFromForm);

	//To do
	// General Race Swapping
	for (std::uint8_t i = 0; i < a_data->numHeadParts; i++) {
		auto newPart = SwitchHeadPart(rand_generator, a_data, a_data->headParts[i]);
		logger::debug("Swapping {} {:x} to {} {:x}", 
			a_data->headParts[i]->type.underlying(), 
			a_data->headParts[i]->formID, 
			newPart->type.underlying(), 
			newPart->formID);
		a_data->headParts[i] = newPart;
	}

	DoTints(rand_generator, a_data, originalRace);
	DoHeadMorphs(rand_generator, a_data);

	return;
}

bool RaceSwap::DoHeadMorphs(util::RandomGen<RE::TESForm> rand_gen, NPCAppearance::NPCData* a_data)
{
	if (!a_data->race->faceRelatedData[a_data->isFemale]) {
		return false;
	}
	// Pick a random preset, and set morphs/parts to match
	auto presetNPCs = a_data->race->faceRelatedData[a_data->isFemale]->presetNPCs;
	auto newNPC = util::random_pick(*presetNPCs, rand_gen.GetNext());

	if (!newNPC) {
		logger::info("  No preset NPC to get morphs from");
		return false;
	}
	auto morphs = newNPC->faceData->morphs;
	auto parts = newNPC->faceData->parts;

	logger::info("  Swapping morphs/parts");
	for (auto i = 0; i < RE::TESNPC::FaceData::Morphs::kTotal; i++) {
		a_data->faceData->morphs[i] = morphs[i];
	}

	for (auto i = 0; i < RE::TESNPC::FaceData::Parts::kTotal; i++) {
		a_data->faceData->parts[i] = parts[i];
	}

	return false;
}

std::uint16_t GetClosestPresetIdx(RE::Color a_color, RE::TESRace::FaceRelatedData::TintAsset::Presets a_presets) {
	std::uint16_t closestPresetIdx = 0;
	float closestPresetMatch = 1000000; // Closer to 0.0 is better
	for (std::uint16_t i = 0; i < a_presets.colors.size(); i++) {
		RE::Color currentColor = a_presets.colors[i]->color;
		
		float currentPresetMatch = std::abs(a_color.alpha - currentColor.alpha) +
		                           std::abs(a_color.blue - currentColor.blue) +
		                           std::abs(a_color.green - currentColor.green) +
		                           std::abs(a_color.red - currentColor.red);

		if (currentPresetMatch < closestPresetMatch) {
			closestPresetMatch = currentPresetMatch;
			closestPresetIdx = i;
		}
	}

	return closestPresetIdx;
}

bool RaceSwap::DoTints(util::RandomGen<RE::TESForm> rand_gen, NPCAppearance::NPCData* a_data, RE::TESRace* a_originalRace)
{
	if (!a_data->tintLayers || 
		!a_originalRace->faceRelatedData[a_data->isFemale] ||
		!a_originalRace->faceRelatedData[a_data->isFemale]->tintMasks || 
		!a_data->race->faceRelatedData[a_data->isFemale] ||
		!a_data->race->faceRelatedData[a_data->isFemale]->tintMasks) {
		logger::info("  No tint layers or tint masks present!");
		return false;
	}
	auto skintone_tintasset = raceswap::DataBase::GetSingleton()->GetRaceSkinTint(static_cast<RE::SEX>(a_data->isFemale), a_data->race);
	auto& originalTintAssets = a_originalRace->faceRelatedData[a_data->isFemale]->tintMasks;
	auto& newTintAssets = a_data->race->faceRelatedData[a_data->isFemale]->tintMasks;

	bool bodyColorFixed = false;
	// Loop through the tints and find equivalent random tints for the other race
	for (auto& tint : *(a_data->tintLayers)) {
		RE::TESRace::FaceRelatedData::TintAsset* originalTintAsset = nullptr;

		auto matchedTints = new RE::BSTArray<RE::TESRace::FaceRelatedData::TintAsset*>();
		matchedTints->clear();
		// Find original tint asset from the original race
		for (auto& asset : *originalTintAssets) {
			if (asset->texture.index == tint->tintIndex) {
				originalTintAsset = asset;
				break;
			}
		}
	
		if (!originalTintAsset) {
			// Can't find original asset, invalid tint? Remove the tint to be safe
			tint->interpolationValue = 0; // TODO: Check if this actually removes the tint from being displayed?
			break;
		}

		// Find all tint assets in new race that is the same type, then pick a pseudo-random one
		for (auto& asset : *newTintAssets) {
			if (asset->texture.skinTone == originalTintAsset->texture.skinTone) {
				matchedTints->push_back(asset);
			}
		}

		if (matchedTints->empty()) {
			// Can't find original asset, invalid tint? Remove the tint to be safe
			tint->interpolationValue = 0;  // TODO: Check if this actually removes the tint from being displayed?
			break;
		}

		auto new_tint = util::random_pick(*matchedTints, rand_gen.GetNext());

		// Replace values of original tint with new tint, keeping closest color match that's within the asset's presets

		tint->tintIndex = new_tint->texture.index;
		
		auto presetIdx = GetClosestPresetIdx(tint->tintColor, new_tint->presets);
		
		tint->preset = presetIdx;
		auto alpha = tint->tintColor.alpha;
		tint->tintColor = new_tint->presets.colors[presetIdx]->color;
		tint->tintColor.alpha = alpha;

		// TODO: Interpolation may need changing?

		// Update body tint color if this is for skin
		if (new_tint->texture.skinTone == RE::TESRace::FaceRelatedData::TintAsset::TintLayer::SkinTone::kSkinTone) {
			a_data->bodyTintColor = tint->tintColor;
			bodyColorFixed = true;
			logger::info("  Skin tone changed to RGB:{}|{}|{}|{}", tint->tintColor.red, tint->tintColor.green, tint->tintColor.blue, tint->tintColor.alpha);
		}
	}

	//If npc doesn't have skin tint layer, assign closest skin tint layer
	if (!bodyColorFixed && skintone_tintasset) {
		RE::TESNPC::Layer* skin_tint = new RE::TESNPC::Layer();
		skin_tint->tintIndex = skintone_tintasset->texture.index;

		auto presetIdx = GetClosestPresetIdx(a_data->bodyTintColor, skintone_tintasset->presets);
		skin_tint->preset = presetIdx;

		skin_tint->tintColor = skintone_tintasset->presets.colors[presetIdx]->color;

		skin_tint->interpolationValue = 100;

		a_data->tintLayers->push_back(skin_tint);

		a_data->bodyTintColor = skin_tint->tintColor;
		
		logger::info("  NPC has no skin tone. Skin tone assigned to RGB:{}|{}|{}", skin_tint->tintColor.red, skin_tint->tintColor.green, skin_tint->tintColor.blue);
	} else if (!skintone_tintasset) {
		logger::info("  NPC has no skin tone. And Race: {} Sex: {} has no default skin tone.", a_data->race->GetFormEditorID(), a_data->isFemale);
		return false;
	}

	return true;
}

RE::BGSHeadPart* RaceSwap::SwitchHeadPart(util::RandomGen<RE::TESForm> rand_gen, NPCAppearance::NPCData* a_data, RE::BGSHeadPart* a_part)
{
	if (a_part == nullptr) {
		return a_part;
	}
	auto head_part_type = a_part->type.get();

	auto database = raceswap::DataBase::GetSingleton();

	bool invalid_item = !database->IsValidHeadPart(a_part);

	if (invalid_item) {
		logger::info("  Invalid head part {:x}", a_part->formID);
		return a_part;
	} else {
		raceswap::DataBase::HDPTData hdptd = *(database->FindOrCalculateHDPTData(a_part));
		auto item_list = database->GetMatchedResults(head_part_type, static_cast<RE::SEX>(a_data->isFemale), a_data->race, hdptd);

		auto new_item = util::random_pick(item_list, rand_gen.GetNext());
		if (new_item) {
			return new_item;
		} else {
			return a_part;
		}
	}
}
