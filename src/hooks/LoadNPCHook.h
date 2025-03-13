#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

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
