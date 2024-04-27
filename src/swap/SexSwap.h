#pragma once
#include "PCH.h"
#include "RaceSwapUtils.h"
#include "NPCAppearance.h"

class SexSwap
{
public:
	using HeadPartType = RE::BGSHeadPart::HeadPartType;

	static void applySwap(NPCAppearance::NPCData* a_data, RE::SEX a_otherSex);

private:
	static bool DoHeadData(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data);

	static bool DoHeadParts(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data);

	static bool DoHeadMorphs(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data);

	static bool DoTints(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::SEX a_originalSex);

	static RE::BGSHeadPart* SwitchHeadPart(raceutils::RandomGen rand_gen, NPCAppearance::NPCData* a_data, RE::BGSHeadPart* a_part);

	static std::unordered_map<RE::BGSHeadPart*, raceutils::HDPTData*> _hdptd_cache;
};

