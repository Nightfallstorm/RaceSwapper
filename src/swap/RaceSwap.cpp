#include "PCH.h"
#include "RaceUtils.h"
#include "RaceSwap.h"

void RaceSwap::applySwap(NPCAppearance::NPCData* a_data, RE::TESRace* a_otherRace) {
	if (!a_otherRace) {
		return;
	}

	a_data->race = a_otherRace;
	a_data->skeletonModel = &a_otherRace->skeletonModels[a_data->isFemale];
	a_data->isBeastRace = a_otherRace->HasKeywordID(constants::Keyword_IsBeastRace);
	// TODO:
	return;
}

bool RaceSwap::DoHairstyle(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag)
{
	// TODO
	return false;
}

bool RaceSwap::DoHeadMorphs(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag)
{
	// TODO
	return false;
}

bool RaceSwap::DoEyes(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag)
{
	// TODO
	return false;
}

bool RaceSwap::DoScars(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag)
{
	// TODO
	return false;
}

bool RaceSwap::DoTints(util::RandomGen<RE::TESForm> rand_gen, RE::TESNPC* a_npc, uint32_t a_flag)
{
	// TODO
	return false;
}
