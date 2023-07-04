#include "PCH.h"
#include "RaceUtils.h"
#include "Database.h"

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
		return std::to_string(a_form_seed->GetFormID() & 0x00FFFFFF) + "_" + a_form_seed->GetFile()->fileName;
	}

		



	_likelihood_t _match(HDPTData dst, HDPTData src)
	{
		_likelihood_t likelihood = 0;
		likelihood = (_likelihood_t) ~((dst.first >> 1) ^ (src.first >> 1));
		likelihood = likelihood << 4;
		std::uint8_t count = 0;
		for (auto& str : dst.second) {
			if (util::is_amongst(src.second, str))
				count++;
		}
		likelihood += count;
		return likelihood;
	}
	std::vector<RE::BGSHeadPart*> MatchHDPTData(HDPTData dst, std::vector<RE::BGSHeadPart*> src_hdpts, std::vector<HDPTData> src_data)
	{
		if (src_hdpts.size() != src_data.size()) {
			return std::vector<RE::BGSHeadPart*>();
		}

		std::map<_likelihood_t, std::vector<RE::BGSHeadPart*>> likelihood_map;

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
