#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct LoadTESObjectARMOHook
{
	// We swap the race being passed to be what the NPC's new appearance is
	static void thunk(RE::TESObjectARMO* a_armor, RE::TESRace* a_race, RE::BipedAnim** a_anim, bool isFemale)
	{
		if (!a_anim || !(*a_anim)->actorRef.get().get() || !(*a_anim)->actorRef.get().get()->As<RE::Actor>()) {
			logger::warn("LoadTESObjectARMOHook: Failed to grab actor, returning without attaching armor");
			return;
		}

		auto actor = (*a_anim)->actorRef.get().get()->As<RE::Actor>();
		if (utils::IsRaceWerewolfOrVampire(a_race)) {
			// Ignore werewolve and vampire form
			return AttachToBiped(a_armor, actor, a_race, a_anim, isFemale);
		}
		auto race = a_race;

		auto NPC = actor->GetActorBase();

		logger::debug("LoadTESObjectARMOHook: Loading {} {:x} for NPC {} {:x}",
			utils::GetFormEditorID(a_armor), a_armor->formID,
			utils::GetFormEditorID(NPC), NPC->formID);

		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(NPC);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			// Swap to new appearance's race
			race = appearance->alteredNPCData.race;
		}
		return AttachToBiped(a_armor, actor, race, a_anim, isFemale);
	}

	using BipedObjectSlot = stl::enumeration<RE::BGSBipedObjectForm::BipedObjectSlot, std::uint32_t>;
	
		
	// Overwrite TESObjectARMO::AttachToBiped functionality. This hook will let us load the armor with
	// the closest valid race
	static void AttachToBiped(RE::TESObjectARMO* a_armor, RE::Actor* a_actor, RE::TESRace* a_race, RE::BipedAnim** a_anim, bool isFemale)
	{
		RE::TESRace* race = utils::GetValidRaceForArmorRecursive(a_armor, a_race);
		if (!race) {
			// Race can't load anything from this armor
			logger::warn("Race {} {:x} cannot load armor {} {:x}",
				utils::GetFormEditorID(a_race), a_race->formID,
				utils::GetFormEditorID(a_armor), a_armor->formID);

			if (isDAVOrDDNGActive()) {
				return CallAttachToBipedDAV(a_armor, a_race, a_actor, a_anim, isFemale);
			} else {
				return;
			}
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

		if (isDAVOrDDNGActive()) {
			CallAttachToBipedDAV(a_armor, race, a_actor, a_anim, isFemale);
		} else {
			for (auto addon : a_armor->armorAddons) {
				if (addon->race == race || utils::is_amongst(addon->additionalRaces, race)) {
					AddToBiped(addon, a_armor, a_anim, isFemale);
				}
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

	static void CallAttachToBipedDAV(RE::TESObjectARMO* a_armor, RE::TESRace* a_appearanceRace, RE::Actor* a_actor, RE::BipedAnim** a_anim, bool isFemale) {
		auto originalRace = a_actor->GetRace();
		a_actor->GetActorRuntimeData().race = a_appearanceRace;
		
		auto originalAppearanceArmorRace = a_appearanceRace->armorParentRace;
		a_appearanceRace->armorParentRace = nullptr;

		AttachToBipedDAV(a_armor, a_actor, a_anim, isFemale);

		a_actor->GetActorRuntimeData().race = originalRace;
		
		a_appearanceRace->armorParentRace = originalAppearanceArmorRace;
	}

	static void AttachToBipedDAV(RE::TESObjectARMO* a_armor, RE::Actor* a_actor, RE::BipedAnim** a_anim, bool isFemale)
	{
		attachToBipedDAV(a_armor, a_actor, a_anim, isFemale);
	}

	// TESObjectARMO::AddToBiped(...)
	static inline REL::Relocation<decltype(thunk)> func;

	// TESObjectARMO::AddToBiped(...)
	static inline REL::Relocation<decltype(AddToBiped)> addToBiped;

	static inline REL::Relocation<decltype(AttachToBipedDAV)> attachToBipedDAV;

	static inline std::map<RE::TESObjectARMO*, BipedObjectSlot> armorSlotMap;

	static const inline bool isDAVOrDDNGActive() {
		static auto DAVHandle = GetModuleHandleA("DynamicArmorVariants");
		static auto DDNGHandle = GetModuleHandleA("DeviousDevices");
		return DAVHandle != nullptr || DDNGHandle != nullptr;
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		if (!isDAVOrDDNGActive()) {
			addToBiped = { RELOCATION_ID(17361, 17759) };

			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24232, 24736), REL::VariantOffset(0x302, 0x302, 0x302) };
			stl::write_thunk_call<LoadTESObjectARMOHook>(target.address());
	
			logger::info("LoadTESObjectARMOHook hooked at address {:x}", target.address());
			logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target.offset());
		} else {
			InstallForDAVAndDDNG();	
		}

		REL::Relocation<std::uintptr_t> target2{ RELOCATION_ID(24233, 24737), REL::VariantOffset(0x78, 0x78, 0x78) };
		stl::write_thunk_call<LoadTESObjectARMOHook>(target2.address());

		REL::Relocation<std::uintptr_t> target3{ RELOCATION_ID(24237, 24741), REL::VariantOffset(0xEE, 0xEE, 0xEE) };
		stl::write_thunk_call<LoadTESObjectARMOHook>(target3.address());


		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target2.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target2.offset());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target3.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target3.offset());
	}

	static inline void InstallForDAVAndDDNG() {
		logger::info("LoadTESObjectARMOHook installing compatibility hook for DAV and/or DDNG");
		byte originalBytes[] = { 
			(byte)0x44, (byte)0x8B, (byte)0x49, (byte)0x38, (byte)0x41, (byte)0x83, (byte)0xE1, (byte)0x01, (byte)0x48,
			(byte)0x8B, (byte)0x91, (byte)0x58, (byte)0x01, (byte)0x00, (byte)0x00, (byte)0x48, (byte)0x8B, (byte)0xCD, (byte)0xE8, (byte)0x79,
			(byte)0x44, (byte)0xEC, (byte)0xFF 
		};

		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24232, 24736), REL::VariantOffset(0x2F0, 0x2F0, 0x2F0) };

		// Grab the DAV hook that replaces AddToBiped() function
		auto attachToBipedDAVMovAddress = target.address() + 0x8;
		attachToBipedDAV = *(int64_t*)(attachToBipedDAVMovAddress);

		// Fill back in the original Skyrim logic. Our hook will run first using the original skyrim logic before handing control to DAV
		REL::safe_write(target.address(), originalBytes, 0x17);
		
		stl::write_thunk_call<LoadTESObjectARMOHook>(target.address() + 0x12);
	}
};
