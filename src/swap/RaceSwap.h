#pragma once
#include "PCH.h"
#include "RaceUtils.h"
#include "NPCAppearance.h"

class RaceSwap
{
public:
	enum ReturnFlag
	{
		Successful = 0,
		Failed = 1,
		Skipped = 2
	};

	static ReturnFlag SwapNPC(RE::TESNPC* a_npc, uint32_t a_flag);
	static void applySwap(NPCAppearance::NPCData* a_data, RE::TESRace* a_otherRace);

private:

	static bool DoHairstyle(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag);

	static bool DoHeadMorphs(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag);

	static bool DoEyes(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag);

	static bool DoScars(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag);

	static bool DoTints(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag);
};

