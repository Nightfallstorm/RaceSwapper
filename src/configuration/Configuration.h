#pragma once
#include <utility>
#include "ConfigurationEntry.h"

struct AppearanceConfiguration
{
	RE::TESNPC* otherNPC;
	RE::TESRace* otherRace;
	RE::SEX otherSex = RE::SEX::kNone;
	std::string entry;
	std::string file;
};

/* Example Config file
# Everything after a hashtag is a comment
# Matches accept the following: Race, NPC, Faction, Probability (default: 100%, max 100%)
# Swaps accept the following: Race, NPC, Weight (default 10, max 100)
# Weight is used when determining which swap to apply
# Ex: Nazeem matches a nord race swap AND a khajiit race swap. The higher weight entry is more
# likely to get the swap over the other, but not guaranteed


# Swap all nords to Khajiit with a 50% chance
match=0x13746~Skyrim.esm|50% swap=0x13745~Skyrim.esm
# Swap all nords to Khajiit with a 100% chance using editor IDs
match=NordRace|100% swap=KhajiitRace # Anything after the "#" are ignored (use as comment)
# Swap all nords to Argonians, using a high weight
match=NordRace|100% swap=ArgonianRace|100  # This entry very likely to win against the others with default weight of 10 

*/

// TODO: Add exclusions for headparts to never be included? (Needs consideration)
// TODO: Add exclusions for headparts to never be excluded from strict mode? (Needs consideration)

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

private:
	std::vector<ConfigurationEntry*> entries;
};

