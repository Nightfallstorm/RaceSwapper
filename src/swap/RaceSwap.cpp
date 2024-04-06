#include "PCH.h"
#include "RaceSwapUtils.h"
#include "Utils.h"
#include "RaceSwap.h"
#include "RaceSwapDatabase.h"

std::string GetHeadPartTypeAsName(RE::BGSHeadPart::HeadPartType a_type) {
	if (a_type == RE::BGSHeadPart::HeadPartType::kEyebrows) {
		return "Eyebrows";
	} else if (a_type == RE::BGSHeadPart::HeadPartType::kEyes) {
		return "Eyes";
	} else if (a_type == RE::BGSHeadPart::HeadPartType::kFace) {
		return "Face";
	} else if (a_type == RE::BGSHeadPart::HeadPartType::kFacialHair) {
		return "Facial Hair";
	} else if (a_type == RE::BGSHeadPart::HeadPartType::kHair) {
		return "Hair";
	} else if (a_type == RE::BGSHeadPart::HeadPartType::kMisc) {
		return "Misc";
	} else if (a_type == RE::BGSHeadPart::HeadPartType::kScar) {
		return "Scar";
	}
	return "Unknown";
}

void RaceSwap::applySwap(NPCAppearance::NPCData* a_data, RE::TESRace* a_otherRace)
{
	if (!a_otherRace || a_data->race == a_otherRace) {
		// Don't do anything if no other race present, or NPC is already said race
		return;
	}

	if (a_data->baseNPC->race->IsChildRace() != a_otherRace->IsChildRace()) {
		// Don't allow child NPCs swapping to non-child races and vice versa
		logger::warn("Attempting to swap {:x} to race {} {:x}. Childen cannot be swapped to non-child races and vice versa!",
			a_data->baseNPC->formID, utils::GetFormEditorID(a_otherRace), a_otherRace->formID);
		return;
	}

	logger::info("Swapping {} {:x} to {} {:x}", 
		utils::GetFormEditorID(a_data->baseNPC).c_str(),
		a_data->baseNPC->formID, 
		utils::GetFormEditorID(a_otherRace),
		a_otherRace->formID
	);

	auto originalRace = a_data->race;
	a_data->race = a_otherRace;
	a_data->faceNPC = nullptr; // Prevents cases where face NPC is configured differently
	a_data->skeletonModel = &a_otherRace->skeletonModels[a_data->isFemale];
	a_data->isBeastRace = a_otherRace->HasKeywordID(constants::Keyword_IsBeastRace);
	a_data->skin = a_otherRace->skin;
	a_data->faceRelatedData = a_otherRace->faceRelatedData[a_data->isFemale];
	a_data->bodyPartData = a_otherRace->bodyPartData;
	a_data->bodyTextureModel = &a_otherRace->bodyTextureModels[a_data->isFemale];
	a_data->behaviorGraph = &a_otherRace->behaviorGraphs[a_data->isFemale];

	raceutils::RandomGen rand_generator(a_data->baseNPC);	

	DoHeadData(rand_generator, a_data);
	DoHeadParts(rand_generator, a_data);
	DoTints(rand_generator, a_data, originalRace);
	DoHeadMorphs(rand_generator, a_data);

	return;
}

RE::BGSColorForm* GetClosestColorForm(RE::BGSColorForm* a_colorForm, RE::BSTArray<RE::BGSColorForm*>* a_colors)
{
	if (!a_colorForm || !a_colors || a_colors->empty()) {
		return nullptr;
	}

	RE::BGSColorForm* closestColor = nullptr;
	int closestPresetMatch = 1000000;  // Closer to 0.0 is better
	RE::Color origColor = a_colorForm->color;
	for (auto colorForm : *a_colors) {
		RE::Color currentColor = colorForm->color;

		int currentPresetMatch = std::abs(origColor.blue - currentColor.blue) +
		                         std::abs(origColor.green - currentColor.green) +
		                         std::abs(origColor.red - currentColor.red);

		if (currentPresetMatch < closestPresetMatch) {
			closestPresetMatch = currentPresetMatch;
			closestColor = colorForm;
		}
	}

	return closestColor;
}

bool RaceSwap::DoHeadData(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data)
{
	auto database = raceswap::DataBase::GetSingleton();
	if (!a_data->race->faceRelatedData[a_data->isFemale]) {
		return false;
	}

	auto defaultTexture = a_data->race->faceRelatedData[a_data->isFemale]->defaultFaceDetailsTextureSet;
	
	if (!a_data->headRelatedData->faceDetails) {
		if (defaultTexture) {
			logger::debug("No skin texture, using new default {} {:x}", utils::GetFormEditorID(defaultTexture).c_str(), defaultTexture->formID);
			a_data->headRelatedData->faceDetails = defaultTexture;
		} else {
			logger::debug("No skin texture, using NONE");
			a_data->headRelatedData->faceDetails = nullptr;
		}	
	} else {
		auto item_list = database->GetMatchedSkinTextureResults(static_cast<RE::SEX>(a_data->isFemale), a_data->race, a_data->headRelatedData->faceDetails);

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
	auto allHairColors = a_data->race->faceRelatedData[a_data->isFemale]->availableHairColors;
	a_data->headRelatedData->hairColor = GetClosestColorForm(currentHairColor, allHairColors);
	return true;
}

bool RaceSwap::DoHeadParts(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data)
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
		auto oldPart = headpart;

		logger::debug("{:x} Swapping {} {} {:x} to {} {} {:x}",
			a_data->baseNPC->formID,
			GetHeadPartTypeAsName(oldPart->type.get()),
			utils::GetFormEditorID(oldPart),
			oldPart->formID,

			GetHeadPartTypeAsName(newPart->type.get()),
			utils::GetFormEditorID(newPart),
			newPart->formID);

		// Add new headpart, along with its extras
		newHeadParts.push_back(newPart);
		for (auto extra : newPart->extraParts) {
			newHeadParts.push_back(extra);
		}
	}

	// Final passthrough, replace the original headparts with the new ones
	auto numHeadParts = (std::uint8_t) newHeadParts.size();
	RE::BGSHeadPart** headparts = RE::calloc<RE::BGSHeadPart*>(numHeadParts);
	for (std::uint8_t i = 0; i < numHeadParts; i++) {
		headparts[i] = newHeadParts.at(i);
	}

	a_data->numHeadParts = numHeadParts;
	a_data->headParts = headparts;

	return true;
}

bool RaceSwap::DoHeadMorphs(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data)
{
	if (!a_data->race->faceRelatedData[a_data->isFemale]) {
		return false;
	}
	// Pick a random preset, and set morphs/parts to match
	auto presetNPCs = a_data->race->faceRelatedData[a_data->isFemale]->presetNPCs;
	if (!presetNPCs || presetNPCs->empty()) {
		logger::info("  No presets available!");
		return false;
	}
	auto newNPC = raceutils::random_pick(
		*presetNPCs, 
		(int) rand_gen.GetNext()
	);

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

std::uint16_t GetClosestPresetIdx(RE::Color a_color, RE::TESRace::FaceRelatedData::TintAsset::Presets a_presets) {
	std::uint16_t closestPresetIdx = 0;
	int closestPresetMatch = 1000000;  // Closer to 0.0 is better
	for (std::uint16_t i = 0; i < a_presets.colors.size(); i++) {
		RE::Color currentColor = a_presets.colors[i]->color;
		
		int currentPresetMatch = std::abs(a_color.blue - currentColor.blue) +
		                           std::abs(a_color.green - currentColor.green) +
		                           std::abs(a_color.red - currentColor.red);

		if (currentPresetMatch < closestPresetMatch) {
			closestPresetMatch = currentPresetMatch;
			closestPresetIdx = i;
		}
	}

	return closestPresetIdx;
}

bool RaceSwap::DoTints(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::TESRace* a_originalRace)
{
	if (!a_originalRace->faceRelatedData[a_data->isFemale] ||
		!a_originalRace->faceRelatedData[a_data->isFemale]->tintMasks) {
		logger::info("  No tint masks for the original race!");
		return false;
	}

	if (!a_data->race->faceRelatedData[a_data->isFemale] ||
		!a_data->race->faceRelatedData[a_data->isFemale]->tintMasks) {
		logger::info("  No tint masks for the new race!");
		return false;
	}

	if (!a_data->tintLayers) {
		// NPC has no tints, but the new race does. Create a tint array to ensure skin matches face later
		logger::warn(" {:x} has no tint layers!", a_data->baseNPC->formID);
		a_data->tintLayers = RE::calloc<RE::BSTArray<RE::TESNPC::Layer*>>(1);
	}

	auto skintone_tintasset = raceswap::DataBase::GetSingleton()->GetRaceSkinTint(static_cast<RE::SEX>(a_data->isFemale), a_data->race);
	auto& originalTintAssets = a_originalRace->faceRelatedData[a_data->isFemale]->tintMasks;
	auto& newTintAssets = a_data->race->faceRelatedData[a_data->isFemale]->tintMasks;

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
			tint->interpolationValue = 0; // TODO: Check if this actually removes the tint from being displayed?
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
		
		auto presetIdx = GetClosestPresetIdx(tint->tintColor, new_tint->presets);
		
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

		auto presetIdx = GetClosestPresetIdx(a_data->bodyTintColor, skintone_tintasset->presets);
		skin_tint->preset = presetIdx;

		skin_tint->tintColor = skintone_tintasset->presets.colors[presetIdx]->color;
		a_data->bodyTintColor = skin_tint->tintColor;

		// This seems to make the skin tint correctly match the body tint
		skin_tint->interpolationValue = 100;
		skin_tint->tintColor.alpha = 255;

		a_data->tintLayers->push_back(skin_tint);

		
		
		logger::info("  NPC has no skin tone. Skin tone assigned to RGB:{}|{}|{}", skin_tint->tintColor.red, skin_tint->tintColor.green, skin_tint->tintColor.blue);
	} else if (!skintone_tintasset) {
		logger::info("  NPC has no skin tone. And Race: {} Sex: {} has no default skin tone.", utils::GetFormEditorID(a_data->race).c_str(), a_data->isFemale);
		return false;
	}

	return true;
}

RE::BGSHeadPart* RaceSwap::SwitchHeadPart(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::BGSHeadPart* a_part)
{
	if (a_part == nullptr) {
		return a_part;
	}
	auto head_part_type = a_part->type.get();

	auto database = raceswap::DataBase::GetSingleton();

	auto& hdptd = *(database->FindOrCalculateHDPTData(a_part));

	auto item_list = database->GetMatchedHeadPartResults(head_part_type, static_cast<RE::SEX>(a_data->isFemale), a_data->race, hdptd);

	auto new_item = raceutils::random_pick(item_list, rand_gen.GetNext());
	if (new_item && !database->IsValidHeadPart(new_item)) {
		// TODO: Exclude invalid head parts?
		logger::warn("  {:x} is an invalid head part!", new_item->formID);
	}
	if (new_item) {
		return new_item;
	} else {
		logger::debug("New headpart is null, keeping original!");
		return a_part;
	}
}
