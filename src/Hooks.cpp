#pragma once
#include "Hooks.h"
#include "swap/NPCAppearance.h"
#include "Utils.h"

struct GetTESModelHook
{
	static RE::TESModel* GetTESModel(RE::TESNPC* a_npc)
	{
		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.skeletonModel;
		}

		return OriginalModel(a_npc);
	}

	static RE::TESModel* OriginalModel(RE::TESNPC* a_npc) {
		// Original logic
		if (!a_npc->race->skeletonModels[a_npc->GetSex()].model.empty()) {
			return &a_npc->race->skeletonModels[a_npc->GetSex()];
		} else {
			return a_npc->race->skeletonModels;
		}
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(19322, 19749), REL::VariantOffset(0x5F, 0x6B, 0x5F) };
		// Fill the gender and skeleton calls with NOP, as we will handle both gender and skeleton access ourselves
		if (REL::Module::IsAE()) {
			REL::safe_fill(target.address(), REL::NOP, 0x32);
		} else {
			REL::safe_fill(target.address(), REL::NOP, 0xF);
		}

		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);
		trampoline.write_call<5>(target.address(), reinterpret_cast<uintptr_t>(GetTESModel));

		if (REL::Module::IsAE()) {
			// AE inlines and uses rbx for skeleton model
			std::byte fixReturnValue[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xC3 }; // mov rbx, rax
			REL::safe_write(target.address() + 0x5, fixReturnValue, 3);
		}

		logger::info("GetTESModelHook hooked at address {:x}", target.address());
		logger::info("GetTESModelHook hooked at offset {:x}", target.offset());
	}
};

struct GetBodyPartDataHook
{
	static RE::BGSBodyPartData* thunk(RE::TESRace* a_actor)
	{
		// a_npc WAS the race, but we kept it as Actor for our purposes >:)
		auto actor = reinterpret_cast<RE::Actor*>(a_actor);
		logger::debug("Getting body part data for {:x}", actor->formID);
		auto appearance = NPCAppearance::GetNPCAppearance(actor->GetActorBase());
		if (appearance && appearance->isNPCSwapped) {
			logger::debug("Using new NPC body part data");
			return appearance->alteredNPCData.bodyPartData;
		}

		if (!actor->GetActorRuntimeData().race) {
			logger::debug("Returning no body part data for {:x}", actor->formID);
			return nullptr;
		}

		logger::debug("Returning default body part data for {:x}", actor->formID);
		return func(actor->GetActorRuntimeData().race);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> load3DTarget{ RELOCATION_ID(36198, 37177), REL::VariantOffset(0x5A, 0x57, 0x5A) };
		// Remove call to replace RCX (actor) with actor's race. This lets our hook have access to the actor data
		if (REL::Module::IsAE()) {
			byte useActorInstructions[] = { 0x48, 0x89, 0xd9 }; // mov rcx, rbx
			REL::safe_fill(load3DTarget.address() - 0x3, REL::NOP, 0x3);
			REL::safe_write(load3DTarget.address() - 0x3, useActorInstructions, 0x3);
		} else {
			// SE/VR
			byte useActorInstructions[] = { 0x48, 0x89, 0xF1 }; 
			REL::safe_fill(load3DTarget.address() - 0x11, REL::NOP, 0x7);
			REL::safe_write(load3DTarget.address() - 0x11, useActorInstructions, 0x3);
		}
		
		stl::write_thunk_call<GetBodyPartDataHook>(load3DTarget.address());

		// TODO: May need to hook other areas?
		logger::info("GetBodyPartData hooked at address {:x}", load3DTarget.address());
		logger::info("GetBodyPartData hooked at offset {:x}", load3DTarget.offset());
	}
};

struct GetBaseMoveTypes
{
	static RE::BGSMovementType* thunk(RE::TESRace* a_actor, std::uint64_t a_type)
	{
		// a_npc WAS the race, but we kept it as Actor for our purposes >:)
		auto actor = reinterpret_cast<RE::Actor*>(a_actor);

		auto appearance = NPCAppearance::GetNPCAppearance(actor->GetActorBase());
		if (appearance && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.race->baseMoveTypes[a_type];
		}

		return func(actor->GetActorRuntimeData().race, a_type);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: Hook usages of 140386D90 (1.5.97) for race swapping
		// Mainly for swaps to creatures
	}
};

struct GetFaceRelatedDataHook
{
	static RE::TESRace::FaceRelatedData* GetFaceData(RE::TESNPC* a_npc)
	{
		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.faceRelatedData;
		}

		// Original logic
		return a_npc->race->faceRelatedData[a_npc->GetSex()];
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24226, 24730), REL::VariantOffset(0x8B, 0x5A, 0x8B) };
		REL::safe_fill(target.address(), REL::NOP, 0x10);                                     // Fill with NOP
		if (REL::Module::IsAE()) {
			std::byte newInstructions[] = { (std::byte)0x49, (std::byte)0x89, (std::byte)0xC6 };  // mov r14, rax
			REL::safe_write(target.address() + 0x5, newInstructions, 3);
		} else {
			std::byte newInstructions[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xC3 };  // mov rbx, rax
			REL::safe_write(target.address() + 0x5, newInstructions, 3);
		}
		
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);
		trampoline.write_call<5>(target.address(), reinterpret_cast<uintptr_t>(GetFaceData));

		logger::info("GetFaceRelatedDataHook hooked at address {:x}", target.address());
		logger::info("GetFaceRelatedDataHook hooked at offset {:x}", target.offset());
	}
};

struct GetFaceRelatedDataHook2
{
	static std::uint64_t thunk(std::uint64_t a_unk, std::uint64_t a_unk1, std::uint64_t a_unk2, RE::TESNPC* a_npc)
	{
		// Swap faceRelatedData for the duration of this function call
		auto oldFaceRelatedData = a_npc->race->faceRelatedData[a_npc->GetSex()];

		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			auto faceRelatedData = appearance->alteredNPCData.faceRelatedData;
			a_npc->race->faceRelatedData[a_npc->GetSex()] = faceRelatedData;
		}

		auto result = func(a_unk, a_unk1, a_unk2, a_npc);

		a_npc->race->faceRelatedData[a_npc->GetSex()] = oldFaceRelatedData;
		return result;
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: The function is used elsewhere, may need to hook that too?
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(26258, 26837), REL::VariantOffset(0x95, 0x7D, 0x95) };

		stl::write_thunk_call<GetFaceRelatedDataHook2>(target.address());

		logger::info("GetFaceRelatedDataHook2 hooked at address {:x}", target.address());
		logger::info("GetFaceRelatedDataHook2 hooked at offset {:x}", target.offset());
	}
};

struct LoadTESObjectARMOHook
{
	using BipedObjectSlot = stl::enumeration<RE::BGSBipedObjectForm::BipedObjectSlot, std::uint32_t>;
	static inline std::map<RE::TESObjectARMO*, BipedObjectSlot> armorSlotMap;

	// We swap the race being passed to be what the NPC's new appearance is
	// Afterwards, we apply the armor race rework to use the correct race for the given armor and appearance race
	static void thunk(RE::TESObjectARMO* a_armor, RE::TESRace* a_race, RE::BipedAnim** a_anim, bool isFemale)
	{
		auto race = a_race;
		if (!a_anim || !(*a_anim)->actorRef.get().get() || !(*a_anim)->actorRef.get().get()->As<RE::Actor>()) {
			return func(a_armor, race, a_anim, isFemale);
		}

		auto NPC = (*a_anim)->actorRef.get().get()->As<RE::Actor>()->GetActorBase();

		logger::debug("LoadTESObjectARMOHook: Loading {} {:x} for NPC {} {:x}",
			utils::GetFormEditorID(a_armor), a_armor->formID,
			utils::GetFormEditorID(NPC), NPC->formID);

		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(NPC);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			// Swap to new appearance's race
			race = appearance->alteredNPCData.race;
		}

		race = ApplyArmorRaceRework(a_armor, race);

		// Remove armor race entry to prevent original function using it
		RE::TESRace* origArmorRace = race->armorParentRace;
		race->armorParentRace = nullptr;
		func(a_armor, race, a_anim, isFemale);
		race->armorParentRace = origArmorRace;
	}

	static RE::TESRace* ApplyArmorRaceRework(RE::TESObjectARMO* a_armor, RE::TESRace* a_race)
	{
		RE::TESRace* race = utils::GetValidRaceForArmorRecursive(a_armor, a_race);
		if (!race) {
			// Race can't load anything from this armor
			logger::warn("Race {} {:x} cannot load armor {} {:x}",
				utils::GetFormEditorID(a_race), a_race->formID,
				utils::GetFormEditorID(a_armor), a_armor->formID);
			return a_race;
		}
		if (armorSlotMap.contains(a_armor)) {
			a_armor->bipedModelData.bipedObjectSlots = armorSlotMap.at(a_armor);
		} else {
			armorSlotMap.emplace(a_armor, a_armor->bipedModelData.bipedObjectSlots);
		}

		auto origSlots = a_armor->bipedModelData.bipedObjectSlots;
		a_armor->bipedModelData.bipedObjectSlots = GetCorrectBipedSlots(a_armor, race);

		logger::debug("Loading {:x} as race {} {:x} with new slots {:x}, old slots {:x}",
			a_armor->formID,
			utils::GetFormEditorID(race),
			race->formID,
			a_armor->bipedModelData.bipedObjectSlots.underlying(),
			origSlots.underlying());

		return race;
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

	// TESObjectARMO::AddToBiped(...)
	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24232, 24736), REL::VariantOffset(0x302, 0x302, 0x302) };
		stl::write_thunk_call<LoadTESObjectARMOHook>(target.address());

		REL::Relocation<std::uintptr_t> target2{ RELOCATION_ID(24233, 24737), REL::VariantOffset(0x78, 0x78, 0x78) };
		stl::write_thunk_call<LoadTESObjectARMOHook>(target2.address());

		REL::Relocation<std::uintptr_t> target3{ RELOCATION_ID(24237, 24741), REL::VariantOffset(0xEE, 0xEE, 0xEE) };
		stl::write_thunk_call<LoadTESObjectARMOHook>(target3.address());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target.offset());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target2.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target2.offset());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target3.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target3.offset());
	}
};

struct LoadSkinHook
{
	// Based off 1.5.97 TESObjectARMA::ContainsRace_140226D70(v9[i], a_race)
	// We swap the race being passed to be what the NPC's new appearance is
	// We also swap the armor skin to make sure it is correct
	static std::uint64_t thunk(RE::TESObjectARMO* a_skin, RE::TESRace* a_race, RE::BipedAnim** a_anim, bool isFemale)
	{
		auto race = a_race;
		auto NPC = (*a_anim)->actorRef.get().get()->As<RE::Actor>()->GetActorBase();
		auto skin = a_skin;

		NPCAppearance* appearance = ApplyAppearanceToNPC(NPC);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			// Swap to new appearance's race and skin
			race = appearance->alteredNPCData.race;
			skin = appearance->alteredNPCData.skin;
			logger::debug("LoadSkinHook: Swap occurred!");
		}
		logger::debug("LoadSkinHook: Loading {} {:x} for NPC {} {:x}",
			utils::GetFormEditorID(skin).c_str(), skin->formID,
			utils::GetFormEditorID(NPC).c_str(), NPC->formID);
		return func(skin, race, a_anim, isFemale);
	}

	// This hook seems to be the best earliest hook spot for applying appearance as the NPC loads
	static NPCAppearance* ApplyAppearanceToNPC(RE::TESNPC* a_npc)
	{
		logger::debug("Loading {:x} appearance from LoadSkinHook hook", a_npc->formID);

		auto appearance = NPCAppearance::GetOrCreateNPCAppearance(a_npc);

		if (appearance && !appearance->isNPCSwapped) {
			appearance->ApplyNewAppearance(true);
		}

		return appearance;
	}

	// TESObjectARMO::AddToBiped(...)
	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(15499, 15676), REL::VariantOffset(0x173, 0x359, 0x173) };
		stl::write_thunk_call<LoadSkinHook>(target.address());

		logger::info("LoadSkinHook hooked at address {:x}", target.address());
		logger::info("LoadSkinHook hooked at offset {:x}", target.offset());
	}
};

struct SetRaceHook
{
	// When actor's race is set, we will intercept and reapply appearances as if it was a brand new actor
	static std::uint64_t thunk(RE::TESNPC* a_npc, RE::TESRace* a_newRace)
	{
		logger::info("SetRace called on {}{:x}, re-doing appearance on new race {}{:x}", 
			utils::GetFormEditorID(a_npc),
			a_npc->formID,
			utils::GetFormEditorID(a_newRace),
			a_newRace->formID
		);
		auto appearance = NPCAppearance::GetOrCreateNPCAppearance(a_npc);
		if (appearance && appearance->isNPCSwapped) {
			appearance->RevertNewAppearance();
		}
		NPCAppearance::EraseNPCAppearance(a_npc);
		auto result = func(a_npc, a_newRace);
		
		appearance = NPCAppearance::GetOrCreateNPCAppearance(a_npc);
		if (appearance) {
			appearance->ApplyNewAppearance(true);
		}

		return result;
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36901, 37925), REL::VariantOffset(0xD5, 0xCC, 0xD5) };
		stl::write_thunk_call<SetRaceHook>(target.address());

		logger::info("SetRaceHook hooked at address {:x}", target.address());
		logger::info("SetRaceHook hooked at offset {:x}", target.offset());
	}
};

struct HasOverlaysHook
{
	struct ASMJmp : Xbyak::CodeGenerator
	{
		ASMJmp(std::uintptr_t func, std::uintptr_t jmpAddr)
		{
			Xbyak::Label funcLabel;

			if (REL::Module::IsAE()) {
				mov(rcx, rbx);  // Moves the TESNPC to first argument for our thunk
			} else {
				mov(rcx, rdi);  // Moves the TESNPC to first argument for our thunk
			}
			sub(rsp, 0x20);
			call(ptr[rip + funcLabel]);
			add(rsp, 0x20);
			mov(rcx, jmpAddr);
			jmp(rcx);

			L(funcLabel);
			dq(func);
		}
	};

	// When actor has an altered race (eg: Nord -> NordVampire) we will tell the game
	// the actor is NOT altered when it comes to RaceSwapper. This will ensure the actor
	// always uses the headparts we give to it and not from a pre-populated list
	static std::uint64_t thunk(RE::TESNPC* a_npc)
	{
		auto originalResult = a_npc->originalRace && a_npc->originalRace != a_npc->race;

		auto appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance && appearance->isNPCSwapped) {
			return false; // Tell Skyrim NPC is NOT altered, so it will use our headparts 
		} else {
			return originalResult;
		}
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24274, 24790), REL::VariantOffset(0xD2, 0xC9, 0xD2) };
		auto range = 0x1D;
		if (REL::Module::IsAE()) {
			range = 0x1F;
		}
		std::uintptr_t start = target.address();
		std::uintptr_t end = target.address() + 0x1D;
		REL::safe_fill(start, REL::NOP, end - start);
		
		auto jmp = ASMJmp((std::uintptr_t) thunk, end);
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(jmp.getSize());
		auto result = trampoline.allocate(jmp);
		SKSE::AllocTrampoline(14);
		trampoline.write_branch<5>(start, (std::uintptr_t) result); 

		logger::info("HasOverlaysHook hooked at address {:x}", target.address());
		logger::info("HasOverlaysHook hooked at offset {:x}", target.offset());
	}
};

struct PopulateGraphHook
{
	static std::uint64_t thunk(RE::Actor* a_actor, std::uint64_t a_unk1, std::uint64_t a_unk2)
	{
		auto appearance = NPCAppearance::GetNPCAppearance(a_actor->GetActorBase());
		auto origRace = a_actor->GetActorRuntimeData().race;
		if (appearance && appearance->isNPCSwapped) {
			a_actor->GetActorBase()->race = appearance->alteredNPCData.race;
		}

		auto result = func(a_actor, a_unk1, a_unk2);

		if (appearance && appearance->isNPCSwapped) {
			a_actor->GetActorBase()->race = origRace;
		}

		return result;
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x72;
	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::Character, PopulateGraphHook>();

		logger::info("PopulateGraphHook hooked!");
	}
};

struct IsBeastRaceHook
{
	static bool thunk(RE::Actor* a_self, RE::BGSKeyword* a_keyword)
	{
		if (!a_keyword || a_keyword->formID != constants::Keyword_IsBeastRace) {
			return func(a_self, a_keyword);
		}
		auto appearance = NPCAppearance::GetOrCreateNPCAppearance(a_self->GetActorBase());
		auto race = a_self->GetRace();
		if (appearance && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.isBeastRace;
		}
		return race->HasKeyword(a_keyword);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x48;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::Character, 0, IsBeastRaceHook>();

		logger::info("IsBeastRace hook set");
	}
};

struct HeightHook
{
	static float thunk(RE::TESNPC* a_self)
	{
		auto race = a_self->race;
		auto appearance = NPCAppearance::GetNPCAppearance(a_self);
		if (appearance && appearance->isNPCSwapped) {
			race = appearance->alteredNPCData.race;
		}

		return OrigRaceLogic(a_self, race);
	}

	static inline float OrigRaceLogic(RE::TESNPC* a_npc, RE::TESRace* a_appearanceRace) {
		if (!a_appearanceRace) {
			return a_npc->height;
		}

		const auto sex = a_npc->GetSex();
		switch (sex) {
		case RE::SEX::kMale :
		case RE::SEX::kFemale :
			return a_appearanceRace->data.height[sex] * a_npc->height;
		default:
			return 0.0;
		}
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ REL::VariantID(24256, 24763, 0x375F90) };

		SKSE::AllocTrampoline(0x14);
		SKSE::GetTrampoline().write_branch<5>(target.address(), thunk);

		logger::info("HeightHook set at address {:x}", target.address());
		logger::info("HeightHook set at offset {:x}", target.offset());
	}
};

struct CopyFromTemplate
{
	static void thunk(RE::TESActorBaseData* a_self, RE::TESActorBase* a_template)
	{
		
		auto NPC = skyrim_cast<RE::TESNPC*, RE::TESActorBaseData>(a_self);
		auto templateNPC = skyrim_cast<RE::TESNPC*, RE::TESActorBase>(a_template);

		if (!NPC || !templateNPC) {
			return func(a_self, a_template);
		}


		// Remove any existing appearance data
		// Since this NPC is being copied from a template, we are treating this NPC as brand new
		NPCAppearance::EraseNPCAppearance(NPC);
		

		func(a_self, a_template);

		// Process base NPC now that it grabbed the new template data
		ProcessBaseNPC(NPC);
		return;

			
	}

	static void ProcessBaseNPC(RE::TESNPC* NPC) {
		auto NPCAppearance = NPCAppearance::GetOrCreateNPCAppearance(NPC);
		if (NPCAppearance) {
			NPCAppearance->ApplyNewAppearance();
		}
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x4;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 1, CopyFromTemplate>();

		logger::info("CopyFromTemplate hook set");

	}
};

struct CopyNPC
{
	// Maintain Swapper data when copying data between NPCs
	static void thunk(RE::TESNPC* a_self, RE::TESForm* a_other)
	{
		if (!a_other->As<RE::TESNPC>()) {
			func(a_self, a_other); // This should just NOP, but invoke to be safe
			return;
		}

		bool otherNPCSwapped = false;
		NPCAppearance* otherNPCAppearance = NPCAppearance::GetNPCAppearance(a_other->As<RE::TESNPC>());
		if (otherNPCAppearance) {
			// Swapper data existed for other NPC, revert to original appearance for the copy
			if (otherNPCAppearance->isNPCSwapped) {
				otherNPCAppearance->RevertNewAppearance();
				otherNPCSwapped = true;
			}
		}
		
		func(a_self, a_other);

		// Erase Swapper data, and set it up to match other Swapper data if it exists
		NPCAppearance::EraseNPCAppearance(a_self);
		if (otherNPCAppearance != nullptr) {
			// This should get the exact same appearance as the old NPC
			// No user would realistically specify a template/non-unique NPC for an entry, 
			// so the new appearance data should be the same as the old one 
			NPCAppearance::GetOrCreateNPCAppearance(a_self); 
			if (otherNPCSwapped && NPCAppearance::GetNPCAppearance(a_self)) {
				NPCAppearance::GetNPCAppearance(a_self)->ApplyNewAppearance();
			}
		}

		// Reapply swap to other NPC if necessary

		if (otherNPCAppearance && otherNPCSwapped) {
			otherNPCAppearance->ApplyNewAppearance();
		}	
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x2F;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 0, CopyNPC>();

		logger::info("CopyNPC hook set");
	}
};

struct DtorNPC
{
	// Remove Swapper data when NPC being cleared
	static void thunk(RE::TESNPC* a_self, std::byte unk)
	{
		NPCAppearance::EraseNPCAppearance(a_self);
		func(a_self, unk);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 0, DtorNPC>();

		logger::info("DtorNPC hook set");
	}
};

struct SaveNPC
{
	// Revert any swaps before saving to prevent presistence
	static void thunk(RE::TESNPC* a_self, std::uint64_t unkSaveStruct)
	{
		auto appearance = NPCAppearance::GetNPCAppearance(a_self);
		if (!appearance) {
			// No appearance data means no need to revert anything
			return func(a_self, unkSaveStruct);
		}
		bool appliedSwap = appearance->isNPCSwapped;
		if (appearance && appliedSwap) {
			logger::info("Reverting NPC for save: {:x}", a_self->formID);
			appearance->RevertNewAppearance();
			
		}
		func(a_self, unkSaveStruct);

		if (appearance && appliedSwap) {
			appearance->ApplyNewAppearance();
		}
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0xE;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 0, SaveNPC>();

		logger::info("SaveNPC hook set");
	}
};

struct LoadNPC
{
	// Revert NPC data when loading NPC. Erase and treat NPC as brand new
	static void thunk(RE::TESNPC* a_self, std::uint64_t unkLoadStruct)
	{
		logger::info("Erasing NPC for load game: {:x}", a_self->formID);
		auto appearance = NPCAppearance::GetNPCAppearance(a_self);
		if (appearance && appearance->isNPCSwapped) {
			appearance->RevertNewAppearance();
		}

		NPCAppearance::EraseNPCAppearance(a_self);
		func(a_self, unkLoadStruct);
		
		//appearance = NPCAppearance::GetOrCreateNPCAppearance(a_self);
		//if (appearance) {
		//	logger::info("Applying appearance to NPC for load game: {:x}", a_self->formID);
		//	appearance->ApplyNewAppearance();
		//}
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0xF;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 0, LoadNPC>();

		logger::info("LoadNPC hook set");
	}
};

struct RevertNPC
{
	// Revert all swaps, restore factory settings
	static void thunk(RE::TESNPC* a_self, std::uint64_t unkSaveStruct)
	{
		auto appearance = NPCAppearance::GetNPCAppearance(a_self);
		if (!appearance) {
			// No appearance data means no need to revert anything
			return func(a_self, unkSaveStruct);
		}
		bool appliedSwap = appearance->isNPCSwapped;
		if (appearance && appliedSwap) {
			logger::info("Reverting NPC for revert: {:x}", a_self->formID);
			appearance->RevertNewAppearance();
		}
		func(a_self, unkSaveStruct);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x12;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 0, RevertNPC>();

		logger::info("RevertNPC hook set");
	}
};

class HandleFormDelete : public RE::BSTEventSink<RE::TESFormDeleteEvent>
{
	RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>* a_eventSource) override
	{
		NPCAppearance::EraseNPCAppearance(a_event->formID);
		return RE::BSEventNotifyControl::kContinue;
	}
};

void hook::InstallHooks()
{
	GetTESModelHook::Install();
	GetFaceRelatedDataHook::Install();
	GetFaceRelatedDataHook2::Install();
	GetBodyPartDataHook::Install();
	GetBaseMoveTypes::Install();
	LoadTESObjectARMOHook::Install();
	LoadSkinHook::Install();
	PopulateGraphHook::Install();
	IsBeastRaceHook::Install();
	HeightHook::Install();
	SetRaceHook::Install();
	HasOverlaysHook::Install();
	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(new HandleFormDelete());
	CopyFromTemplate::Install();
	CopyNPC::Install();
	DtorNPC::Install();
	SaveNPC::Install();
	LoadNPC::Install();
	RevertNPC::Install();
}
