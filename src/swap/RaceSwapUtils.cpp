#include "RaceSwapUtils.h"
#include "PCH.h"
#include "RaceSwapDatabase.h"

namespace raceutils
{
	HDPTData ExtractKeywords(RE::BGSHeadPart* hdpt)
	{
		std::uint32_t types = 0;
		std::uint32_t characteristics = 0;
		std::uint32_t colors = 0;
		std::string str(utils::GetFormEditorID(hdpt));

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
	std::vector<RE::BGSHeadPart*> MatchHDPTData(HDPTData dst, std::vector<HeadpartData> src_hdpts)
	{
		std::map<_likelihood_t, std::vector<RE::BGSHeadPart*>> likelihood_map;

		_likelihood_t max = 0;
		for (int i = 0; i < src_hdpts.size(); i++) {
			auto likelihood = _match(dst, *(src_hdpts[i].second));
			if (likelihood > max)
				max = likelihood;
			likelihood_map[likelihood].push_back(src_hdpts[i].first);
		}
		return likelihood_map[max];
	}

	SkinTextureData ExtractKeywords(RE::BGSTextureSet* hdpt)
	{
		std::uint32_t characteristics = 0;
		std::string str(utils::GetFormEditorID(hdpt));

		if (str == "") {
			logger::info("{:x} has no editor ID!");
		}

		for (auto& [charString, characteristic] : raceswap::DataBase::SkinCharMap) {
			if (str.find(charString) != std::string::npos) {
				characteristics |= characteristic;
			}
		}

		SkinTextureData result = { characteristics };
		return result;
	}

	_likelihood_t _match(SkinTextureData dst, SkinTextureData src)
	{
		_likelihood_t likelihood = 0;

		auto charsMatch = std::bitset<32>(dst & src).count();
		likelihood += (_likelihood_t)charsMatch;
		likelihood = likelihood << 4;

		return likelihood;
	}

	std::vector<RE::BGSTextureSet*> MatchSkinTextureData(SkinTextureData dst, std::vector<RE::BGSTextureSet*> src_hdpts, std::vector<SkinTextureData> src_data)
	{
		if (src_hdpts.size() != src_data.size()) {
			return std::vector<RE::BGSTextureSet*>();
		}

		std::map<_likelihood_t, std::vector<RE::BGSTextureSet*>> likelihood_map;

		_likelihood_t max = 0;
		for (int i = 0; i < src_data.size(); i++) {
			auto likelihood = _match(dst, src_data[i]);
			if (likelihood > max)
				max = likelihood;
			likelihood_map[likelihood].push_back(src_hdpts[i]);
		}
		return likelihood_map[max];
	}

}
