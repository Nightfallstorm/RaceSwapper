#pragma once
#include "configuration/Configuration.h"

class NPCAppearance
{
public:
	using pad = std::byte;
	// Appearance information of an NPC, used to swap and revert
	// Some variables are specifically padded so we can call TESNPC functions
	// with this data, such as "TESNPC::ChangeHeadPart", and treat this data
	// as the TESNPC
	struct NPCData
	{
		RE::TESNPC* baseNPC;          // 00
		RE::TESModel* skeletonModel;  // 08
		bool isFemale;                // 09
		////////////////// RACE DATA /////////////////////////////
		bool isBeastRace;                               // 0A
		RE::TESObjectARMO* skin;                        // 10
		RE::TESRace::FaceRelatedData* faceRelatedData;  // 18
		RE::BGSBodyPartData* bodyPartData;              // 20
		RE::BGSTextureModel* bodyTextureModel;          // 28
		RE::BGSBehaviorGraphModel* behaviorGraph;       // 30
		////////////////////////////////////////////////////
		pad pad18[0x188];                              // 38
		RE::TESNPC::HeadRelatedData* headRelatedData;  // 1C8
		pad pad1D0[0x18];                              // 1D0
		RE::TESRace* race;                             // 1E8 - originalRace
		RE::TESNPC* faceNPC;                           // 1F0
		float height;                                  // 1F8
		float weight;                                  // 1FC
		pad pad200[0x10];                              // 200
		RE::TESObjectARMO* farSkin;                    // 210
		pad pad218[0x20];                              // 218
		RE::BGSHeadPart** headParts;                   // 238
		std::uint8_t numHeadParts;                     // 240
		pad pad241[0x5];                               // 241
		RE::Color bodyTintColor;                       // 246
		pad pad247[0xE];                               // 24A
		RE::TESNPC::FaceData* faceData;                // 258
		RE::BSTArray<RE::TESNPC::Layer*>* tintLayers;  // 260
	};
	static_assert(sizeof(NPCData) == 0x268);
	static_assert(offsetof(NPCData, faceData) == 0x258);
	static_assert(offsetof(NPCData, tintLayers) == 0x260);
	static_assert(offsetof(NPCData, headParts) == 0x238);
	static_assert(offsetof(NPCData, numHeadParts) == 0x240);

	NPCData originalNPCData = { 0 };
	NPCData alteredNPCData = { 0 };

	bool isNPCSwapped = false;

	bool ApplyNewAppearance(bool updateLoadedActors);

	bool RevertNewAppearance(bool updateLoadedActors);

	static NPCAppearance* GetOrCreateNPCAppearance(RE::TESNPC* a_npc);

	static NPCAppearance* GetNPCAppearance(RE::TESNPC* a_npc);

	static void EraseNPCAppearance(RE::TESNPC* a_npc);

	static void EraseNPCAppearance(RE::FormID a_formID);

	TES_HEAP_REDEFINE_NEW();

private:
	RE::TESNPC* npc;

	AppearanceConfiguration* config;

	NPCAppearance(RE::TESNPC* a_npc, AppearanceConfiguration* a_config);

	void InitializeNPCData(NPCData* a_data);

	void SetupNewAppearance();

	void CopyFaceData(NPCData* a_data);

	void ApplyAppearance(NPCData* a_data);

	void dtor();

	static inline std::recursive_mutex appearanceMapLock;

	static inline std::map<RE::FormID, NPCAppearance*> appearanceMap;
};

// TODO: This is here to ensure size error in CLIB-NG fails this build unless corrected
// For some reason, this is needed to correctly store and use layers for this mod
static_assert(sizeof(RE::TESNPC::Layer) == 0xC);
