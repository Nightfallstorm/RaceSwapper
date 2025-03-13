#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct CopyNPC
{
	// Maintain Swapper data when copying data between NPCs
	static void thunk(RE::TESNPC* a_self, RE::TESForm* a_other)
	{
		if (!a_other->As<RE::TESNPC>()) {
			func(a_self, a_other);  // This should just NOP, but invoke to be safe
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
