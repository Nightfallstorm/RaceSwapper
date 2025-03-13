#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct GetBaseMoveTypes
{
	static RE::BGSMovementType* thunk(RE::TESRace* a_actor, std::uint64_t a_type)
	{
		// a_npc WAS the race, but we kept it as Actor for our purposes >:)
		auto actor = reinterpret_cast<RE::Actor*>(a_actor);

		if (utils::IsRaceWerewolfOrVampire(actor->GetRace())) {
			// Ignore werewolve and vampire form
			return func(actor->GetRace(), a_type);
		}

		auto appearance = NPCAppearance::GetNPCAppearance(actor->GetActorBase());
		if (appearance && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.race->baseMoveTypes[a_type];
		}

		return func(actor->GetRace(), a_type);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: Hook usages of 140386D90 (1.5.97) for race swapping
		// Mainly for swaps to creatures
	}
};
