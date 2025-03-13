#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

// TODO: Consider if this hook should be a separate mod to allow armors to load differently
struct AttachTESObjectARMOHook
{
	using BipedObjectSlot = stl::enumeration<RE::BGSBipedObjectForm::BipedObjectSlot, std::uint32_t>;
	// Overwrite TESObjectARMO::AttachToBiped functionality. This hook will let us load the armor with
	// the closest valid race
	static void AttachToBiped(RE::TESObjectARMO* a_armor, RE::TESRace* a_race, RE::BipedAnim** a_anim, bool isFemale)
	{
		RE::TESRace* race = utils::GetValidRaceForArmorRecursive(a_armor, a_race);
		if (!race) {
			// Race can't load anything from this armor
			logger::warn("Race {} {:x} cannot load armor {} {:x}",
				utils::GetFormEditorID(a_race), a_race->formID,
				utils::GetFormEditorID(a_armor), a_armor->formID);
			return;
		}

		// Preserve SOS slot
		BipedObjectSlot ppSlot = a_armor->bipedModelData.bipedObjectSlots & RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary;

		if (armorSlotMap.contains(a_armor)) {
			a_armor->bipedModelData.bipedObjectSlots = armorSlotMap.at(a_armor);
		} else {
			armorSlotMap.emplace(a_armor, a_armor->bipedModelData.bipedObjectSlots);
		}

		auto origSlots = a_armor->bipedModelData.bipedObjectSlots;
		a_armor->bipedModelData.bipedObjectSlots = GetCorrectBipedSlots(a_armor, race);

		// Preserver SOS slot for TNG and similar mods
		if (ppSlot == RE::BGSBipedObjectForm::BipedObjectSlot::kNone) {
			// Remove SOS slot since it was never present
			a_armor->bipedModelData.bipedObjectSlots.reset(RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary);
		} else {
			// Keep SOS slot since it was present before corrections
			a_armor->bipedModelData.bipedObjectSlots.set(RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary);
		}

		logger::debug("Loading {:x} as race {} {:x} with new slots {:x}, old slots {:x}",
			a_armor->formID,
			utils::GetFormEditorID(race),
			race->formID,
			a_armor->bipedModelData.bipedObjectSlots.underlying(),
			origSlots.underlying());

		for (auto addon : a_armor->armorAddons) {
			if (addon->race == race || utils::is_amongst(addon->additionalRaces, race)) {
				AddToBiped(addon, a_armor, a_anim, isFemale);
			}
		}

		// TODO: Revert somehow? For now, use a cache to store the slots
		// This has the bug of player inventory potentially showing inaccurate icon for armor
		//a_armor->bipedModelData.bipedObjectSlots = origSlots;
	}

	// Take the armor's biped slots, and remove the slots that no valid addon for the race supports
	static stl::enumeration<RE::BGSBipedObjectForm::BipedObjectSlot, std::uint32_t> GetCorrectBipedSlots(RE::TESObjectARMO* a_armor, RE::TESRace* a_race)
	{
		BipedObjectSlot addonSlots = RE::BGSBipedObjectForm::BipedObjectSlot::kNone;
		BipedObjectSlot armorSlots = a_armor->bipedModelData.bipedObjectSlots;
		for (auto addon : a_armor->armorAddons) {
			if (addon->race == a_race || utils::is_amongst(addon->additionalRaces, a_race)) {
				addonSlots |= addon->bipedModelData.bipedObjectSlots;
			}
		}

		return armorSlots & addonSlots;
	}

	static void AddToBiped(RE::TESObjectARMA* a_addon, RE::TESObjectARMO* a_armor, RE::BipedAnim** a_anim, bool isFemale)
	{
		addToBiped(a_addon, a_armor, a_anim, isFemale);
	}

	// TESObjectARMO::AddToBiped(...)
	static inline REL::Relocation<decltype(AddToBiped)> addToBiped;

	static inline std::map<RE::TESObjectARMO*, BipedObjectSlot> armorSlotMap;

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(17392, 17792) };

		addToBiped = { RELOCATION_ID(17361, 17759) };

		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);
		trampoline.write_branch<5>(target.address(), AttachToBiped);

		logger::info("AttachTESObjectARMOHook hooked at address {:x}", target.address());
		logger::info("AttachTESObjectARMOHook hooked at offset {:x}", target.offset());
	}
};
