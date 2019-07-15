#include "banlist.hpp"

#include <cstdio>
#include <exception>
#include <nlohmann/json.hpp>

namespace YGOpen
{

int Banlist::GetMode() const
{
	return mode;
}

bool Banlist::FromJSON(const nlohmann::json& j)
{
	std::vector<uint32_t> v;
	try
	{
		std::string modeStr = j.at("mode").get<std::string>();
		if(modeStr == "blacklist")
			mode == MODE_BLACKLIST;
		else if (modeStr == "whitelist")
			mode == MODE_WHITELIST;
		v = j.at("semilimited").get<std::vector<uint32_t>>();
		semilimited = std::set<uint32_t>(v.begin(), v.end());
		v = j.at("limited").get<std::vector<uint32_t>>();
		limited = std::set<uint32_t>(v.begin(), v.end());
		v = j.at("forbidden").get<std::vector<uint32_t>>();
		forbidden = std::set<uint32_t>(v.begin(), v.end());
		if(mode == MODE_WHITELIST && j.count("whitelist") > 0)
		{
			v = j.at("whitelist").get<std::vector<uint32_t>>();
			whitelist = std::set<uint32_t>(v.begin(), v.end());
		}
	}
	catch(std::exception& e)
	{
		std::puts(e.what());
		return false;
	}
	return true;
}

} // namespace YGOpen
