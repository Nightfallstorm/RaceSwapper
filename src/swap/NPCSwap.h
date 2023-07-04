#pragma once
#include "NPCAppearance.h"

class NPCSwap
{
public:
	static void applySwap(NPCAppearance::NPCData* a_data, RE::TESNPC* a_otherNPC);
};

// TODO: This is here to ensure size error in CLIB-NG fails this build unless corrected
static_assert(sizeof(RE::TESNPC::Layer) == 0xC); 
