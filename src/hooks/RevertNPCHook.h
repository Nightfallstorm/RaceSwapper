#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

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
