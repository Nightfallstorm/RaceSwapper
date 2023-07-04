#pragma once
#include <utility>

struct AppearanceConfiguration
{
	std::pair<RE::TESNPC*, std::uint32_t> otherNPC;
	std::pair<RE::TESRace*, std::uint32_t> otherRace;
	std::pair<RE::BGSOutfit*, std::uint32_t> otherOutfit;
};

class ConfigurationDatabase
{
public:

	inline static ConfigurationDatabase*& GetSingleton()
	{
		static ConfigurationDatabase* _this_database = nullptr;
		if (!_this_database)
			_this_database = new ConfigurationDatabase();
		return _this_database;
	}

	/*
	@brief Safely de-allocate the memory space used by DataBase.
	*/
	static void Dealloc()
	{
		delete GetSingleton();
		GetSingleton() = nullptr;
	}

	// Load entries from the various entry files
	void Initialize();

	AppearanceConfiguration* GetConfigurationForNPC(RE::TESNPC* a_npc);
};

class ConfigurationEntry
{
	struct EntryData
	{
		RE::TESNPC* npc;
		RE::TESRace* otherRace;
		RE::BGSOutfit* otherOutfit;
		std::uint32_t priority;
	};

	bool MatchesNPC(RE::TESNPC* a_npc);

	void ApplyEntry(AppearanceConfiguration* a_config);
};
