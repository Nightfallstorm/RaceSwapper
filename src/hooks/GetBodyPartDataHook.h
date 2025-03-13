#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct GetBodyPartDataHook
{
	static RE::BGSBodyPartData* thunk(RE::TESRace* a_actor)
	{
		// a_npc WAS the race, but we kept it as Actor for our purposes >:)
		auto actor = reinterpret_cast<RE::Actor*>(a_actor);

		if (utils::IsRaceWerewolfOrVampire(actor->GetRace())) {
			// Ignore werewolve and vampire form
			return func(actor->GetRace());
		}
		logger::debug("Getting body part data for {:x}", actor->formID);
		auto appearance = NPCAppearance::GetNPCAppearance(actor->GetActorBase());
		if (appearance && appearance->isNPCSwapped) {
			logger::debug("Using new NPC body part data");
			return appearance->alteredNPCData.bodyPartData;
		}

		if (!actor->GetRace()) {
			logger::debug("Returning no body part data for {:x}", actor->formID);
			return nullptr;
		}

		logger::debug("Returning default body part data for {:x}", actor->formID);
		return func(actor->GetRace());
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> load3DTarget{ RELOCATION_ID(36198, 37177), REL::VariantOffset(0x5A, 0x57, 0x5A) };
		// Remove call to replace RCX (actor) with actor's race. This lets our hook have access to the actor data
		if (REL::Module::IsAE()) {
			byte useActorInstructions[] = { 0x48, 0x89, 0xd9 };  // mov rcx, rbx
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
