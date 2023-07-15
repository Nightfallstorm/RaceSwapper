

namespace utils
{
	using _GetFormEditorID = const char* (*)(std::uint32_t);

	std::string UniqueStringFromForm(RE::TESForm* a_form_seed);

	size_t HashForm(RE::TESForm* a_form_seed);

	RE::BSTArray<RE::TESNPC::Layer*>* CopyTintLayers(RE::BSTArray<RE::TESNPC::Layer*>* a_tintLayers);

	RE::TESNPC::HeadRelatedData* CopyHeadRelatedData(RE::TESNPC::HeadRelatedData* a_data);

	RE::BGSHeadPart** CopyHeadParts(RE::BGSHeadPart** a_parts, std::uint32_t a_numHeadParts);

	RE::TESNPC::FaceData* DeepCopyFaceData(RE::TESNPC::FaceData* a_faceData);

	std::vector<std::string> split_string(std::string& a_string, char a_delimiter);

	std::string GetFormEditorID(const RE::TESForm* a_form);
}

