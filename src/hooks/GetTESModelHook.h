#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct GetTESModelHook
{
	static RE::TESModel* GetTESModel(RE::TESNPC* a_npc)
	{
		if (utils::IsRaceWerewolfOrVampire(a_npc->GetRace())) {
			// Ignore werewolve and vampire form
			return OriginalModel(a_npc);
		}

		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.skeletonModel;
		}

		return OriginalModel(a_npc);
	}

	static RE::TESModel* OriginalModel(RE::TESNPC* a_npc)
	{
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
			std::byte fixReturnValue[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xC3 };  // mov rbx, rax
			REL::safe_write(target.address() + 0x5, fixReturnValue, 3);
		}

		logger::info("GetTESModelHook hooked at address {:x}", target.address());
		logger::info("GetTESModelHook hooked at offset {:x}", target.offset());
	}
};
