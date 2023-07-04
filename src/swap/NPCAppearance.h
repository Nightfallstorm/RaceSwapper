#pragma once
#include "configuration/Configuration.h"

class NPCAppearance
{
public:
	// Appearance information of an NPC, used to swap and revert
	struct NPCData
	{
		RE::TESNPC* baseNPC;
		RE::TESNPC* faceNPC;
		RE::TESRace* race;
		float height;
		float weight;
		RE::TESModel* skeletonModel;
		bool isFemale;
		bool isBeastRace;
		RE::Color bodyTintColor;
		RE::BSTArray<RE::TESNPC::Layer*>* tintLayers;
		RE::TESObjectARMO* skin;
		RE::TESObjectARMO* farSkin;
		RE::TESNPC::FaceData* faceData;
		RE::TESRace::FaceRelatedData* faceRelatedData;
		RE::TESNPC::HeadRelatedData* headRelatedData;
		RE::BGSHeadPart** headParts;
		std::uint8_t numHeadParts;
	};

	NPCData originalNPCData = { 0 };
	NPCData alteredNPCData = { 0 };

	bool isNPCSwapped = false;

	bool ApplyNewAppearance(bool updateLoadedActors);

	bool RevertNewAppearance(bool updateLoadedActors);

	static NPCAppearance* GetOrCreateNPCAppearance(RE::TESNPC* a_npc);

	static NPCAppearance* GetNPCAppearance(RE::TESNPC* a_npc);

	static void EraseNPCAppearance(RE::TESNPC* a_npc);

	static void EraseNPCAppearance(RE::FormID a_formID);

private:
	RE::TESNPC* npc;

	AppearanceConfiguration* config;

	NPCAppearance(RE::TESNPC* a_npc, AppearanceConfiguration* a_config);

	void InitializeNPCData(NPCData* a_data);

	void SetupNewAppearance();

	void CopyFaceData(NPCData* a_data);

	void ApplyAppearance(NPCData* a_data);
	
	void dtor();

	static inline std::map<RE::FormID, NPCAppearance*> appearanceMap;
};

// TODO: This is here to ensure size error in CLIB-NG fails this build unless corrected
// For some reason, this is needed to correctly store and use layers for this mod
static_assert(sizeof(RE::TESNPC::Layer) == 0xC);
