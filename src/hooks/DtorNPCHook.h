#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

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
