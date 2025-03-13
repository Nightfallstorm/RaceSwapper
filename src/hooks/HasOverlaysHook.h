#pragma once
#include "Utils.h"
#include "swap/NPCAppearance.h"

struct HasOverlaysHook
{
	struct ASMJmp : Xbyak::CodeGenerator
	{
		ASMJmp(std::uintptr_t func, std::uintptr_t jmpAddr)
		{
			Xbyak::Label funcLabel;

			if (REL::Module::IsAE()) {
				mov(rcx, rbx);  // Moves the TESNPC to first argument for our thunk
			} else {
				mov(rcx, rdi);  // Moves the TESNPC to first argument for our thunk
			}
			sub(rsp, 0x20);
			call(ptr[rip + funcLabel]);
			add(rsp, 0x20);
			mov(rcx, jmpAddr);
			jmp(rcx);

			L(funcLabel);
			dq(func);
		}
	};

	// When actor has an altered race (eg: Nord -> NordVampire) we will tell the game
	// the actor is NOT altered when it comes to RaceSwapper. This will ensure the actor
	// always uses the headparts we give to it and not from a pre-populated list
	static std::uint64_t thunk(RE::TESNPC* a_npc)
	{
		auto originalResult = a_npc->originalRace && a_npc->originalRace != a_npc->race;

		auto appearance = NPCAppearance::GetNPCAppearance(a_npc);
		if (appearance && appearance->isNPCSwapped) {
			return false;  // Tell Skyrim NPC is NOT altered, so it will use our headparts
		} else {
			return originalResult;
		}
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24274, 24790), REL::VariantOffset(0xD2, 0xC9, 0xD2) };
		auto range = 0x1D;
		if (REL::Module::IsAE()) {
			range = 0x1F;
		}
		std::uintptr_t start = target.address();
		std::uintptr_t end = target.address() + 0x1D;
		REL::safe_fill(start, REL::NOP, end - start);

		auto jmp = ASMJmp((std::uintptr_t)thunk, end);
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(jmp.getSize());
		auto result = trampoline.allocate(jmp);
		SKSE::AllocTrampoline(14);
		trampoline.write_branch<5>(start, (std::uintptr_t)result);

		logger::info("HasOverlaysHook hooked at address {:x}", target.address());
		logger::info("HasOverlaysHook hooked at offset {:x}", target.offset());
	}
};
