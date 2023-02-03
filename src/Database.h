#pragma once

class DataBase
{
public:

	struct SwapConfiguration
	{
		std::map<RE::TESRace*, RE::TESRace*> raceRaceSwap;
		std::map<RE::TESFaction*, RE::TESRace*> factionRaceSwap;
		std::map<RE::TESNPC*, RE::TESRace*> npcRaceSwap;
	} swapConfiguration;

	struct RaceParts
	{
		RE::TESRace* race;

		std::vector<RE::TESObjectARMO*> skins;

	} raceParts;

	std::vector<RaceParts> races;

	inline static DataBase* & GetSingleton()
	{
		static DataBase* _this_database = nullptr;
		if (!_this_database)
			_this_database = new DataBase();
		return _this_database;
	}

	/*
	@brief Safely de-allocate the memory space used by DataBase.
	*/
	static void Dealloc() {
		delete GetSingleton();
		GetSingleton() = nullptr;
	}

private:
	DataBase()
	{
		auto const& [map, lock] = RE::TESForm::GetAllForms();

		lock.get().LockForWrite();
		try {
			_initialize(map);
		} catch (...) {
			logger::critical("Error occurred loading up database");
		}
		lock.get().UnlockForWrite();
	}

	void _initialize(RE::BSTHashMap<RE::FormID, RE::TESForm*>* formMap) {
		for (auto [formID, form] : *formMap) {
			
		}
	}
};
