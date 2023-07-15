#pragma once
#include "PCH.h"
#include <random>
#include <functional>
#include <string>
#include <unordered_map>
#include "Utils.h"

namespace raceutils
{
	template <class T>
	T random_pick(std::vector<T> item_list, size_t rand)
	{
		if (item_list.empty())
			return 0;
		return item_list[rand % item_list.size()];
	}

	template <class T>
	T random_pick(RE::BSTArray<T> item_list, size_t rand)
	{
		if (item_list.empty())
			return 0;
		return item_list[rand % item_list.size()];
	}

	template <class T>
	bool is_amongst(std::vector<T> item_list, T elem)
	{
		return std::find(item_list.begin(), item_list.end(), elem) != item_list.end();
	}

	template <class T>
	bool is_amongst(RE::BSTArray<T> item_list, T elem)
	{
		return std::find(item_list.begin(), item_list.end(), elem) != item_list.end();
	}

	template <class _First_T, class _Second_T>
	bool is_amongst(std::unordered_map<_First_T, _Second_T> item_map, _First_T elem)
	{
		return item_map.find(elem) != item_map.end();
	}

	using HDPTData = std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>;
		
	using _likelihood_t = uint16_t;

	HDPTData ExtractKeywords(RE::BGSHeadPart* hdpt);

	_likelihood_t _match(HDPTData dst, HDPTData src);

	std::vector<RE::BGSHeadPart*> MatchHDPTData(HDPTData dst, std::vector<RE::BGSHeadPart*> src_hdpts, std::vector<HDPTData*> src_data);

	/* 
	Class for random number generation based on a TESForm.
	Example:
		util::RandomGen<RE::TESForm> generator(some_form_ptr, util::UniqueStringFromForm);
		auto hash = generator(0)
		auto random1 = generator.GetNext();
		auto random2 = generator.GetStableRandom(1);
		return random1 == random2 //true
	*/
	class RandomGen
	{
	public:
		RandomGen(RE::TESForm* a_item_seed) :
			form_seed(a_item_seed)
		{
			_hash_seed = utils::HashForm(form_seed);
			_random_num = _hash_seed;
		}

		inline const size_t GetHashSeed() const {
			return _hash_seed;
		}

		//@brief Get the Nth random number generated from the hash seed.
		size_t GetStableRandom(std::uint32_t n_th_random = 1){
			_random_num = _hash_seed;
			for (std::uint32_t i = 0; i < n_th_random; i++) {
				GetNext();
			}
			return _random_num;
		}

		//@brief Get the next random number generated from the previous random number.
		size_t GetNext(){
			srand((int) _random_num);
			_random_num = rand();
			srand(clock());
			return _random_num;
		}

		//@brief Get the Nth random number generated from the hash seed.
		size_t operator()(unsigned int n_th_random) {
			return GetStableRandom(n_th_random);
		}

	private:
		RandomGen();

		RE::TESForm* form_seed;
		size_t _hash_seed;
		size_t _random_num;
	};
}
