#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

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

	static void ProcessBaseNPC(RE::TESNPC* NPC)
	{
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
