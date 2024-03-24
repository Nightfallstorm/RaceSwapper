#include "Papyrus.h"
#include "swap/NPCAppearance.h"

namespace Papyrus
{
	bool IsRaceSwapperActive(VM* a_vm, StackID, RE::StaticFunctionTag*) {
		return true;
	}

	RE::TESRace* GetAppearanceRaceOfNPC(VM* a_vm, StackID a_ID, RE::StaticFunctionTag*, RE::TESNPC* a_npc) {
		if (!a_npc) {
			a_vm->TraceStack("Base NPC is null", a_ID);
			return nullptr;
		}
		auto appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (!appearance || !appearance->isNPCSwapped) {
			return a_npc->race;
		}
		
		return appearance->alteredNPCData.race;
	}

	RE::TESRace* GetAppearanceRaceOfActor(VM* a_vm, StackID a_ID, RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		if (!a_actor) {
			a_vm->TraceStack("Actor is null", a_ID);
			return nullptr;
		}
		return GetAppearanceRaceOfNPC(a_vm, a_ID, nullptr, a_actor->GetActorBase());
	}

	bool Bind(VM* a_vm)
	{
		if (!a_vm) {
			logger::critical("couldn't get VM State"sv);
			return false;
		}

		BIND(IsRaceSwapperActive);

		return true;
	}
}
