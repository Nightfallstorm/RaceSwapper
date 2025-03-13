#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct IsBeastRaceHook
{
	static bool thunk(RE::Actor* a_self, RE::BGSKeyword* a_keyword)
	{
		if (!a_keyword || a_keyword->formID != constants::Keyword_IsBeastRace) {
			return func(a_self, a_keyword);
		}
		auto appearance = NPCAppearance::GetOrCreateNPCAppearance(a_self->GetActorBase());
		auto race = a_self->GetRace();
		if (appearance && appearance->isNPCSwapped) {
			return appearance->alteredNPCData.isBeastRace;
		}
		return race->HasKeyword(a_keyword);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x48;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::Character, 0, IsBeastRaceHook>();

		logger::info("IsBeastRace hook set");
	}
};
