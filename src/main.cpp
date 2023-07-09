#include "configuration/Configuration.h"
#include "swap/NPCAppearance.h"
#include "Hooks.h"
#include "swap/RaceSwapDatabase.h"

struct CEHelper
{
	// TODO: Remove when done debugging
	std::uint64_t CESignature = 0x123456789ABCDEF;
	RE::TESRace* KhajiitRace = nullptr;
	RE::TESRace* CowRace = nullptr;
} cehelper;
// Filter for only NPCs this swapping can work on
bool IsNPCValid(RE::TESNPC* a_npc)
{
	return !a_npc->IsPlayer() &&
	       !a_npc->IsPreset() &&
	       !a_npc->IsDynamicForm() &&

	       a_npc->race &&
	       a_npc->race->HasKeywordID(constants::Keyword_ActorTypeNPC);
}

void MessageInterface(SKSE::MessagingInterface::Message* msg) {
	switch (msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		ConfigurationDatabase::GetSingleton()->Initialize();

		logger::info("Starting appearance swaps");
		cehelper.KhajiitRace = RE::TESForm::LookupByID(constants::KhajiitRace)->As<RE::TESRace>();
		cehelper.CowRace = RE::TESForm::LookupByID(constants::CowRace)->As<RE::TESRace>();
		auto [map, lock] = RE::TESForm::GetAllForms();
		lock.get().LockForWrite();

		for (auto& [formID, form] : *map) {
			if (form->As<RE::TESNPC>() && IsNPCValid(form->As<RE::TESNPC>())) {
				logger::info("Start Swapping appearance of {:x}", formID);
				auto NPCAppearance = NPCAppearance::GetOrCreateNPCAppearance(form->As<RE::TESNPC>());
				if (NPCAppearance) {
					logger::info("Swapping appearance of {:x}", formID);
					NPCAppearance->ApplyNewAppearance(false);
				}
			}
		}

		lock.get().UnlockForWrite();

		hook::InstallHooks();
		logger::info("Appearance Swaps complete!");
		break;
	}
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("RaceSwapper");
	v.AuthorName("Nightfallstorm and Hanotak");
	v.UsesAddressLibrary(true);
	v.HasNoStructUse(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	SKSE::Init(a_skse);
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageInterface);
	logger::info("Loaded Plugin");
	return true;
}

extern "C" DLLEXPORT const char* APIENTRY GetPluginVersion()
{
	return Version::NAME.data();
}
