#pragma once
#include "PCH.h"
#include <random>
#include <functional>
#include <string>
#include <unordered_map>

namespace util
{
	template <class T>
	T random_pick(std::vector<T> item_list, int rand)
	{
		if (item_list.empty())
			return 0;
		return item_list[rand % item_list.size()];
	}

	template <class T>
	T random_pick(RE::BSTArray<T> item_list, int rand)
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
		
	template <class _Iteratable_T1, class _Iteratable_T2>
	void remove_intersection(_Iteratable_T1& main_list, _Iteratable_T2& black_list)
	{
		for (auto iter = main_list.begin(); iter != main_list.end();) {
			if (util::is_amongst(black_list, *iter)) {
				iter = main_list.erase(iter);
			} else {
				iter++;
			}
		}
	}

	template <class _Iteratable_T1, class _Iteratable_T2>
	void append_unique(_Iteratable_T1& main_list, _Iteratable_T2& white_list)
	{
		for (auto& elem : white_list) {
			if (!util::is_amongst(main_list, elem)) {
				main_list.emplace_back(std::move(elem));
			}
		}
	}

	template <class _First_T, class _Second_T>
	bool is_amongst(std::unordered_map<_First_T, _Second_T> item_map, _First_T elem)
	{
		return item_map.find(elem) != item_map.end();
	}

	using HDPTData = std::pair<std::uint32_t, std::vector<std::string>>;
		
	using _likelihood_t = uint8_t;

	size_t hash1(std::string str);

	size_t hash2(std::string data);

	std::string UniqueStringFromForm(RE::TESForm* a_form_seed);

	HDPTData ExtractKeywords(RE::BGSHeadPart* hdpt);

	_likelihood_t _match(HDPTData dst, HDPTData src);

	std::vector<RE::BGSHeadPart*> MatchHDPTData(HDPTData dst, std::vector<RE::BGSHeadPart*> src_hdpts, std::vector<HDPTData> src_data);

	/* 
	Class for random number generation.
	Example:
		util::RandomGen<RE::TESForm> generator(some_form_ptr, util::UniqueStringFromForm);
		auto hash = generator(0)
		auto random1 = generator.GetNext();
		auto random2 = generator.GetStableRandom(1);
		return random1 == random2 //true
	*/
	template<class T>
	class RandomGen
	{
	public:
		RandomGen(T* a_item_seed, std::function<std::string(T*)> _gen_unique_string_fn, std::function<size_t(std::string)> _hash_fn = util::hash2) :
			_this_item(a_item_seed), _seed_locked(false), _this_hash_fn(_hash_fn), _this_gen_unique_string_fn(_gen_unique_string_fn)
		{
			_unique_string = _this_gen_unique_string_fn(_this_item);
			_hash_seed = _this_hash_fn(_unique_string);
			_random_num = _hash_seed;
		}

		//Add extra items into the seed. Unusable if the first random number is generated.
		inline bool AppendSeed(T* a_another_form_seed) {
			if (_seed_locked)
				return false;
			_unique_string += _this_gen_unique_string_fn(a_another_form_seed);
			_hash_seed = _this_hash_fn(_unique_string);
			_this_append_item_list.push_back(a_another_form_seed);
			return true;
		}

		inline const std::string GetUniqueString() const {
			return _unique_string;
		}

		inline const size_t GetHashSeed() const {
			return _hash_seed;
		}

		//@brief Get the Nth random number generated from the hash seed.
		size_t GetStableRandom(unsigned int n_th_random = 1){
			_seed_locked = true;
			_random_num = _hash_seed;
			for (int i = 0; i < n_th_random; i++) {
				GetNext();
			}
			return _random_num;
		}

		//@brief Get the next random number generated from the previous random number.
		size_t GetNext(){
			_seed_locked = true;
			srand(_random_num);
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

		T* _this_item;
		std::vector<T*> _this_append_item_list;
		std::string _unique_string;
		size_t _hash_seed;
		bool _seed_locked;
		size_t _random_num;
		std::function<size_t(std::string)> _this_hash_fn;
		std::function<std::string(T*)> _this_gen_unique_string_fn;
	};
}
