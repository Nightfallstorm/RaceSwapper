#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct HeightHook
{
	static float thunk(RE::TESNPC* a_self)
	{
		if (utils::IsRaceWerewolfOrVampire(a_self->GetRace())) {
			// Ignore werewolve and vampire form
			return OrigRaceLogic(a_self, a_self->GetRace());
		}

		auto race = a_self->GetRace();
		auto appearance = NPCAppearance::GetNPCAppearance(a_self);
		if (appearance && appearance->isNPCSwapped) {
			race = appearance->alteredNPCData.race;
		}

		return OrigRaceLogic(a_self, race);
	}

	static inline float OrigRaceLogic(RE::TESNPC* a_npc, RE::TESRace* a_appearanceRace)
	{
		if (!a_appearanceRace) {
			return a_npc->height;
		}

		const auto sex = a_npc->GetSex();
		switch (sex) {
		case RE::SEX::kMale:
		case RE::SEX::kFemale:
			return a_appearanceRace->data.height[sex] * a_npc->height;
		default:
			return 0.0;
		}
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ REL::VariantID(24256, 24763, 0x375F90) };

		SKSE::AllocTrampoline(0x14);
		SKSE::GetTrampoline().write_branch<5>(target.address(), thunk);

		logger::info("HeightHook set at address {:x}", target.address());
		logger::info("HeightHook set at offset {:x}", target.offset());
	}
};
