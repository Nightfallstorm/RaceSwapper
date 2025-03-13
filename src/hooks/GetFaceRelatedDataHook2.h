#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct GetFaceRelatedDataHook2
{
	static std::uint64_t thunk(std::uint64_t a_unk, std::uint64_t a_unk1, std::uint64_t a_unk2, RE::TESNPC* a_npc)
	{
		if (utils::IsRaceWerewolfOrVampire(a_npc->race)) {
			// Ignore werewolve and vampire form
			return func(a_unk, a_unk1, a_unk2, a_npc);
		}

		// Swap faceRelatedData for the duration of this function call
		auto oldFaceRelatedData = a_npc->race->faceRelatedData[a_npc->GetSex()];

		NPCAppearance* appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance != nullptr && appearance->isNPCSwapped) {
			auto faceRelatedData = appearance->alteredNPCData.faceRelatedData;
			a_npc->race->faceRelatedData[a_npc->GetSex()] = faceRelatedData;
		}

		auto result = func(a_unk, a_unk1, a_unk2, a_npc);

		a_npc->race->faceRelatedData[a_npc->GetSex()] = oldFaceRelatedData;
		return result;
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: The function is used elsewhere, may need to hook that too?
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(26258, 26837), REL::VariantOffset(0x95, 0x7D, 0x95) };

		stl::write_thunk_call<GetFaceRelatedDataHook2>(target.address());

		logger::info("GetFaceRelatedDataHook2 hooked at address {:x}", target.address());
		logger::info("GetFaceRelatedDataHook2 hooked at offset {:x}", target.offset());
	}
};
