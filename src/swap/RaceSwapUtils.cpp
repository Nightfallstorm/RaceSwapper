#include "RaceSwapUtils.h"
#include "PCH.h"
#include "RaceSwapDatabase.h"

namespace util
{
	size_t hash1(std::string str)
	{
		std::hash<std::string> hash_fn;
		return hash_fn(str);
	}

	size_t hash2(std::string data)
	{
		long p = 16777619;

		size_t hash = 2166136261L;

		for (int i = 0; i < data.length(); i++) {
			hash = (hash ^ data[i]) * p;

			hash += hash << 13;

			hash ^= hash >> 7;

			hash += hash << 3;

			hash ^= hash << 17;

			hash += hash >> 5;
		}
		return hash;
	}

	std::string UniqueStringFromForm(RE::TESForm* a_form_seed)
	{
		if (!a_form_seed)
			return std::string();
		auto fileName = "DynamicForm";
		if (!a_form_seed->IsDynamicForm()) {
			fileName = a_form_seed->GetFile()->fileName;
		}
		return std::to_string(a_form_seed->GetFormID() & 0x00FFFFFF) + "_" + fileName;
	}

	HDPTData ExtractKeywords(RE::BGSHeadPart* hdpt)
	{
		std::uint32_t types = 0;
		std::uint32_t characteristics = 0;
		std::uint32_t colors = 0;
		std::string str(hdpt->GetFormEditorID());

		if (str == "") {
			logger::info("{:x} has no editor ID!");
		}
		
		for (auto& [typeString, type] : raceswap::DataBase::HDPTTypeMap) {
			if (str.find(typeString) != std::string::npos) {
				types |= type;
			}
		}

		for (auto& [charString, characteristic] : raceswap::DataBase::HDPTCharMap) {
			if (str.find(charString) != std::string::npos) {
				characteristics |= characteristic;
			}
		}

		for (auto& [colorString, color] : raceswap::DataBase::HDPTColorMap) {
			if (str.find(colorString) != std::string::npos) {
				colors |= color;
			}
		}

		HDPTData result = { types, characteristics, colors };
		return result;
	}

	_likelihood_t _match(HDPTData dst, HDPTData src)
	{
		_likelihood_t likelihood = 0;

		auto typesMatch = std::bitset<32>(std::get<0>(dst) & std::get<0>(src)).count();
		likelihood += (_likelihood_t) typesMatch;
		likelihood = likelihood << 4;

		auto charsMatch = std::bitset<32>(std::get<1>(dst) & std::get<1>(src)).count();
		likelihood += (_likelihood_t) charsMatch;
		likelihood = likelihood << 4;

		auto colorsMatch = std::bitset<32>(std::get<2>(dst) & std::get<2>(src)).count();
		likelihood += (_likelihood_t) colorsMatch;

		return likelihood;
	}
	std::vector<RE::BGSHeadPart*> MatchHDPTData(HDPTData dst, std::vector<RE::BGSHeadPart*> src_hdpts, std::vector<HDPTData*> src_data)
	{
		if (src_hdpts.size() != src_data.size()) {
			return std::vector<RE::BGSHeadPart*>();
		}

		std::map<_likelihood_t, std::vector<RE::BGSHeadPart*>> likelihood_map;

		_likelihood_t max = 0;
		for (int i = 0; i < src_data.size(); i++) {
			auto likelihood = _match(dst, *(src_data[i]));
			if (likelihood > max)
				max = likelihood;
			likelihood_map[likelihood].push_back(src_hdpts[i]);
		}
		return likelihood_map[max];
	}
}
