#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct PopulateGraphHook
{
	static std::uint64_t thunk(RE::Actor* a_actor, std::uint64_t a_unk1, std::uint64_t a_unk2)
	{
		if (utils::IsRaceWerewolfOrVampire(a_actor->GetRace())) {
			// Ignore werewolve and vampire form
			return func(a_actor, a_unk1, a_unk2);
		}

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
