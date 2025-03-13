#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct SetRaceHook
{
	// When actor's race is set, we will intercept and reapply appearances as if it was a brand new actor
	static std::uint64_t thunk(RE::TESNPC* a_npc, RE::TESRace* a_newRace)
	{
		logger::info("SetRace called on {} {:x}, re-doing appearance on new race {} {:x}",
			utils::GetFormEditorID(a_npc),
			a_npc->formID,
			utils::GetFormEditorID(a_newRace),
			a_newRace->formID);

		if (utils::IsRaceWerewolfOrVampire(a_newRace)) {
			// Ignore NPCs becoming werewolves/vampires
			logger::info("Ignoring as new race is vampire/werewolf");
			return func(a_npc, a_newRace);
		}

		if (utils::IsRaceWerewolfOrVampire(a_npc->GetRace())) {
			// Ignore NPCs reverting from werewolves/vampires
			logger::info("Ignoring as new race is reverting from vampire/werewolf");
			return func(a_npc, a_newRace);
		}

		auto appearance = NPCAppearance::GetOrCreateNPCAppearance(a_npc);
		if (appearance && appearance->isNPCSwapped) {
			appearance->RevertNewAppearance(false);
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
