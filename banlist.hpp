#ifndef __BANLIST_HPP__
#define __BANLIST_HPP__
#include <set>
#include <string>

#include <nlohmann/json.hpp>

namespace YGOpen
{

class Banlist
{
public:
	enum {MODE_BLACKLIST, MODE_WHITELIST};
	std::set<uint32_t> whitelist;
	std::set<uint32_t> semilimited;
	std::set<uint32_t> limited;
	std::set<uint32_t> forbidden;

	int GetMode() const;
	bool FromJSON(const nlohmann::json& j);
private:
	int mode{MODE_BLACKLIST};
};

} // namespace YGOpen

#endif // __BANLIST_HPP__
