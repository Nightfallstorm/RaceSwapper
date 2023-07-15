#pragma once

#define NOMMNOSOUND

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#pragma warning(disable: 4100)

#pragma warning(push)
#include <SimpleIni.h>
#include <robin_hood.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>
#pragma warning(pop)

namespace logger = SKSE::log;

using namespace std::literals;

namespace constants
{
	inline RE::FormID Keyword_ActorTypeNPC = 0x13794;
	inline RE::FormID Keyword_IsBeastRace = 0xD61D1;

	// debug
	inline RE::FormID Maiq = 0x954BF;
	inline RE::FormID Nazeem = 0x13BBF;
	inline RE::FormID Urog = 0x1B078;
	inline RE::FormID MQ101Alduin = 0x32B94;
	inline RE::FormID DefaultRace = 0x19;
	inline RE::FormID DebugNPCToTest = 0x954BF;
	inline RE::FormID KhajiitRace = 0x13745;
	inline RE::FormID ArgonianRace = 0x13740;
	inline RE::FormID RedguardRace = 0x13748;
	inline RE::FormID NordRace = 0x13746;
	inline RE::FormID OrcRace = 0x13747;
	inline RE::FormID CowRace = 0x4E785;
	inline RE::FormID DragonRace = 0x12E82;
	inline RE::FormID BretonRace = 0x13741;
}

namespace stl
{
	using namespace SKSE::stl;

	void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);

	template <class T>
	void asm_replace(std::uintptr_t a_from)
	{
		asm_replace(a_from, T::size, reinterpret_cast<std::uintptr_t>(T::func));
	}

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class F, size_t offset, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[offset] };
		T::func = vtbl.write_vfunc(T::idx, T::thunk);
	}

	template <class F, class T>
	void write_vfunc()
	{
		write_vfunc<F, 0, T>();
	}

	inline std::string as_string(std::string_view a_view)
	{
		return { a_view.data(), a_view.size() };
	}
}

#ifdef SKYRIM_AE
#	define REL_ID(se, ae) REL::ID(ae)
#	define OFFSET(se, ae) ae
#	define OFFSET_3(se, ae, vr) ae
#elif SKYRIMVR
#	define REL_ID(se, ae) REL::ID(se)
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) vr
#else
#	define REL_ID(se, ae) REL::ID(se)
#	define OFFSET(se, ae) se
#	define OFFSET_3(se, ae, vr) se
#endif

#define DLLEXPORT __declspec(dllexport)

#include "Version.h"
