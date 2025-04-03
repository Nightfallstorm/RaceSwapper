#pragma once

class Settings
{
public:
	[[nodiscard]] static Settings* GetSingleton();

	void Load();

	enum Features : std::uint32_t
	{
		kNone = 0,
		kPlaythroughRandomization = 1 << 0,
		kStrictHeadPartMatching = 2 << 0,
		kDebugLogging = 3 << 0,
	};

	stl::enumeration<Features, std::uint32_t> features;

private:
	void get_value(CSimpleIniA& a_ini, Features a_value, bool a_default, const char* a_section, const char* a_key, const char* a_comment)
	{
		auto value = a_ini.GetBoolValue(a_section, a_key, a_default);
		a_ini.SetBoolValue(a_section, a_key, value, a_comment);
		if (value) {
			features.set(a_value);
		} else {
			features.reset(a_value);
		}
	}
};
