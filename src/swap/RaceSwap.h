#pragma once
#include "PCH.h"
#include "RaceSwapUtils.h"
#include "NPCAppearance.h"

class RaceSwap
{
public:
	using HeadPartType = RE::BGSHeadPart::HeadPartType;

	static void applySwap(NPCAppearance::NPCData* a_data, RE::TESRace* a_otherRace);

private:
	static bool DoHeadMorphs(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data);

	static bool DoTints(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::TESRace* a_originalRace);

	static RE::BGSHeadPart* SwitchHeadPart(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::BGSHeadPart* a_part);

	static std::unordered_map<RE::BGSHeadPart*, raceutils::HDPTData*> _hdptd_cache;
};

