#include "SexSwap.h"
#include "PCH.h"
#include "RaceSwapDatabase.h"
#include "RaceSwapUtils.h"
#include "Utils.h"



std::string GetSexAsName(RE::SEX a_sex)
{
	switch (a_sex) {
	case RE::SEX::kFemale:
		{
			return "Female";
		}
	case RE::SEX::kMale:
		{
			return "Male";
		}
	default:
		{
			return "Unknown";
		}
	}
}

void SexSwap::applySwap(NPCAppearance::NPCData* a_data, RE::SEX a_otherSex)
{
	if (a_otherSex == RE::SEX::kNone || a_otherSex == RE::SEX::kTotal || a_data->sex == a_otherSex) {
		// Don't do anything if NPC is already the given sex, or the sex is invalid
		return;
	}

	logger::info("Swapping {} {:x} to {}",
		utils::GetFormEditorID(a_data->baseNPC).c_str(),
		a_data->baseNPC->formID,
		GetSexAsName(a_otherSex));

	auto originalSex = a_data->sex;
	a_data->sex = a_otherSex;
	a_data->faceNPC = nullptr;  // Prevents cases where face NPC is configured differently
	a_data->skeletonModel = &a_data->race->skeletonModels[a_otherSex];
	a_data->faceRelatedData = a_data->race->faceRelatedData[a_otherSex];
	a_data->bodyTextureModel = &a_data->race->bodyTextureModels[a_otherSex];
	a_data->behaviorGraph = &a_data->race->behaviorGraphs[a_otherSex];

	raceutils::RandomGen rand_generator(a_data->baseNPC);

	assert(a_data->sex == RE::SEX::kMale || a_data->sex == RE::SEX::kFemale);
	DoHeadData(rand_generator, a_data);
	DoHeadParts(rand_generator, a_data);
	DoTints(rand_generator, a_data, originalSex);
	DoHeadMorphs(rand_generator, a_data);

	return;
}

bool SexSwap::DoHeadData(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data)
{
	auto database = raceswap::DataBase::GetSingleton();
	if (!a_data->race->faceRelatedData[a_data->sex]) {
		return false;
	}

	auto defaultTexture = a_data->race->faceRelatedData[a_data->sex]->defaultFaceDetailsTextureSet;

	if (!a_data->headRelatedData->faceDetails) {
		if (defaultTexture) {
			logger::debug("No skin texture, using new default {} {:x}", utils::GetFormEditorID(defaultTexture).c_str(), defaultTexture->formID);
			a_data->headRelatedData->faceDetails = defaultTexture;
		} else {
			logger::debug("No skin texture, using NONE");
			a_data->headRelatedData->faceDetails = nullptr;
		}
	} else {
		auto item_list = database->GetMatchedSkinTextureResults(static_cast<RE::SEX>(a_data->sex), a_data->race, a_data->headRelatedData->faceDetails);

		auto new_item = raceutils::random_pick(item_list, rand_gen.GetNext());
		if (new_item) {
			logger::debug("Swapping from {} {:x} to skin texture {} {:x}",
				utils::GetFormEditorID(a_data->headRelatedData->faceDetails).c_str(), a_data->headRelatedData->faceDetails->formID,
				utils::GetFormEditorID(new_item).c_str(), new_item->formID);
			a_data->headRelatedData->faceDetails = new_item;
		} else {
			logger::debug("New skin texture null, using new default {} {:x}", utils::GetFormEditorID(defaultTexture).c_str(), defaultTexture->formID);
			a_data->headRelatedData->faceDetails = defaultTexture;
		}
	}

	auto currentHairColor = a_data->headRelatedData->hairColor;
	auto allHairColors = a_data->race->faceRelatedData[a_data->sex]->availableHairColors;
	a_data->headRelatedData->hairColor = raceutils::GetClosestColorForm(currentHairColor, allHairColors);
	return true;
}

bool SexSwap::DoHeadParts(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data)
{
	std::vector<RE::BGSHeadPart*> oldHeadparts;
	std::vector<RE::BGSHeadPart*> newHeadParts;

	// First passthrough, gather all non-extra headparts into arraylist
	for (std::uint8_t i = 0; i < a_data->numHeadParts; i++) {
		if (a_data->headParts[i] && !a_data->headParts[i]->IsExtraPart()) {
			oldHeadparts.push_back(a_data->headParts[i]);
		}
	}

	// Second passthrough, swap to other race headparts

	for (auto headpart : oldHeadparts) {
		auto newPart = SwitchHeadPart(rand_gen, a_data, headpart);
		if (!newPart) {
			continue;
		}
		auto oldPart = headpart;

		logger::debug("{:x} Swapping {} {} {:x} to {} {} {:x}",
			a_data->baseNPC->formID,
			raceutils::GetHeadPartTypeAsName(oldPart->type.get()),
			utils::GetFormEditorID(oldPart),
			oldPart->formID,

			raceutils::GetHeadPartTypeAsName(newPart->type.get()),
			utils::GetFormEditorID(newPart),
			newPart->formID);

		// Add new headpart, along with its extras
		newHeadParts.push_back(newPart);
		for (auto extra : newPart->extraParts) {
			newHeadParts.push_back(extra);
		}
	}

	// Final passthrough, replace the original headparts with the new ones
	auto numHeadParts = (std::uint8_t)newHeadParts.size();
	RE::BGSHeadPart** headparts = RE::calloc<RE::BGSHeadPart*>(numHeadParts);
	for (std::uint8_t i = 0; i < numHeadParts; i++) {
		headparts[i] = newHeadParts.at(i);
	}

	a_data->numHeadParts = numHeadParts;
	a_data->headParts = headparts;

	return true;
}

bool SexSwap::DoHeadMorphs(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data)
{
	if (!a_data->race->faceRelatedData[a_data->sex]) {
		return false;
	}
	// Pick a random preset, and set morphs/parts to match
	auto presetNPCs = a_data->race->faceRelatedData[a_data->sex]->presetNPCs;
	if (!presetNPCs || presetNPCs->empty()) {
		logger::info("  No presets available!");
		return false;
	}
	auto newNPC = raceutils::random_pick(
		*presetNPCs,
		(int)rand_gen.GetNext());

	auto morphs = newNPC->faceData->morphs;
	auto parts = newNPC->faceData->parts;

	logger::debug("  Swapping morphs/parts");
	for (auto i = 0; i < RE::TESNPC::FaceData::Morphs::kTotal; i++) {
		a_data->faceData->morphs[i] = morphs[i];
	}

	for (auto i = 0; i < RE::TESNPC::FaceData::Parts::kTotal; i++) {
		a_data->faceData->parts[i] = parts[i];
	}

	return false;
}

bool SexSwap::DoTints(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::SEX a_originalSex)
{
	if (!a_data->race->faceRelatedData[a_originalSex] ||
		!a_data->race->faceRelatedData[a_originalSex]->tintMasks) {
		logger::info("  No original sex tint masks to use!");
		return false;
	}

	if (!a_data->race->faceRelatedData[a_data->sex] ||
		!a_data->race->faceRelatedData[a_data->sex]->tintMasks) {
		logger::info("  No tint masks to use!");
		return false;
	}

	if (!a_data->tintLayers) {
		// NPC has no tints, but the new race does. Create a tint array to ensure skin matches face later
		logger::warn(" {:x} has no tint layers!", a_data->baseNPC->formID);
		a_data->tintLayers = RE::calloc<RE::BSTArray<RE::TESNPC::Layer*>>(1);
	}

	auto skintone_tintasset = raceswap::DataBase::GetSingleton()->GetRaceSkinTint(static_cast<RE::SEX>(a_data->sex), a_data->race);
	auto& originalTintAssets = a_data->race->faceRelatedData[a_originalSex]->tintMasks;
	auto& newTintAssets = a_data->race->faceRelatedData[a_data->sex]->tintMasks;

	bool bodyColorFixed = false;
	// Loop through the tints and find equivalent random tints for the other race
	for (auto& tint : *(a_data->tintLayers)) {
		RE::TESRace::FaceRelatedData::TintAsset* originalTintAsset = nullptr;

		auto matchedTints = RE::BSTArray<RE::TESRace::FaceRelatedData::TintAsset*>();
		matchedTints.clear();

		if (originalTintAssets) {
			// Find original tint asset from the original race
			for (auto& asset : *originalTintAssets) {
				if (asset->texture.index == tint->tintIndex) {
					originalTintAsset = asset;
					break;
				}
			}
		}

		if (!originalTintAsset) {
			logger::debug("tint index {} not found", tint->tintIndex);
			// Can't find original asset, invalid tint? Remove the tint to be safe
			tint->interpolationValue = 0;  // TODO: Check if this actually removes the tint from being displayed?
			tint->tintIndex = 65535;
			tint->tintColor = RE::Color(255, 255, 255, 255);

			continue;
		}

		if (newTintAssets) {
			// Find all tint assets in new race that is the same type, then pick a pseudo-random one
			for (auto& asset : *newTintAssets) {
				if (asset->texture.skinTone == originalTintAsset->texture.skinTone) {
					matchedTints.push_back(asset);
				}
			}
		}

		if (matchedTints.empty()) {
			logger::debug("tint index {} does not have matching tints", tint->tintIndex);
			// Can't find original asset, invalid tint? Remove the tint to be safe
			tint->interpolationValue = 0;  // TODO: Check if this actually removes the tint from being displayed?
			tint->tintIndex = 65535;
			tint->tintColor = RE::Color(255, 255, 255, 255);
			continue;
		}

		auto new_tint = raceutils::random_pick(matchedTints, rand_gen.GetNext());

		// Replace values of original tint with new tint, keeping closest color match that's within the asset's presets

		tint->tintIndex = new_tint->texture.index;

		auto presetIdx = raceutils::GetClosestPresetIdx(tint->tintColor, new_tint->presets);

		tint->preset = presetIdx;
		auto alpha = tint->tintColor.alpha;
		tint->tintColor = new_tint->presets.colors[presetIdx]->color;
		tint->tintColor.alpha = alpha;

		// Update body tint color if this is for skin
		if (new_tint->texture.skinTone == RE::TESRace::FaceRelatedData::TintAsset::TintLayer::SkinTone::kSkinTone) {
			// This seems to make the skin tint correctly match the body tint
			tint->interpolationValue = 100;
			tint->tintColor.alpha = 255;
			a_data->bodyTintColor = tint->tintColor;
			bodyColorFixed = true;
			logger::info("  Skin tone changed to RGB:{}|{}|{}|{}", tint->tintColor.red, tint->tintColor.green, tint->tintColor.blue, tint->tintColor.alpha);
		}
	}

	//If npc doesn't have skin tint layer, assign closest skin tint layer
	if (!bodyColorFixed && skintone_tintasset) {
		RE::TESNPC::Layer* skin_tint = RE::calloc<RE::TESNPC::Layer>(1);
		skin_tint->tintIndex = skintone_tintasset->texture.index;

		auto presetIdx = raceutils::GetClosestPresetIdx(a_data->bodyTintColor, skintone_tintasset->presets);
		skin_tint->preset = presetIdx;

		skin_tint->tintColor = skintone_tintasset->presets.colors[presetIdx]->color;
		a_data->bodyTintColor = skin_tint->tintColor;

		// This seems to make the skin tint correctly match the body tint
		skin_tint->interpolationValue = 100;
		skin_tint->tintColor.alpha = 255;

		a_data->tintLayers->push_back(skin_tint);

		logger::info("  NPC has no skin tone. Skin tone assigned to RGB:{}|{}|{}", skin_tint->tintColor.red, skin_tint->tintColor.green, skin_tint->tintColor.blue);
	} else if (!skintone_tintasset) {
		logger::info("  NPC has no skin tone. And Race: {} Sex: {} has no default skin tone.", utils::GetFormEditorID(a_data->race).c_str(), a_data->sex);
		return false;
	}

	return true;
}

RE::BGSHeadPart* SexSwap::SwitchHeadPart(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::BGSHeadPart* a_part)
{
	if (a_part == nullptr) {
		return a_part;
	}
	auto head_part_type = a_part->type.get();

	auto database = raceswap::DataBase::GetSingleton();

	auto& hdptd = *(database->FindOrCalculateHDPTData(a_part));

	auto item_list = database->GetMatchedHeadPartResults(head_part_type, static_cast<RE::SEX>(a_data->sex), a_data->race, hdptd);

	auto new_item = raceutils::random_pick(item_list, rand_gen.GetNext());
	if (new_item && !database->IsValidHeadPart(new_item)) {
		// TODO: Exclude invalid head parts?
		logger::warn("  {:x} is an invalid head part!", new_item->formID);
	}
	if (new_item) {
		return new_item;
	} else {
		logger::debug("New headpart is null, excluding!");
		return nullptr;
	}
}
