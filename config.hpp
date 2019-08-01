#ifndef __YGOPEN_LEGACY_SERVER_CONFIG_HPP__
#define __YGOPEN_LEGACY_SERVER_CONFIG_HPP__
#include <filesystem>
#include <nlohmann/json.hpp>

namespace YGOpen
{
namespace Legacy
{

namespace ServerConfig {

const nlohmann::json DEFAULT_CONFIG = R"(
	{
		"protocol": "tcp4",
		"port": 44444,
		"databases": [
			"databases/cards.cdb",
			"databases/cards-anime-vg-manga.cdb"
		],
		"banlists": []
	}
)"_json;

nlohmann::json Get(std::string filename);

nlohmann::json GetOrDefault(std::string filename);

template<typename... T, typename = typename std::enable_if<std::conjunction<std::is_convertible<T, std::string>...>::value>::type> 
	nlohmann::json GetOrDefault(std::string first, T const&... filename) {
	if(std::filesystem::exists(first)) {
		return Get(first);
	} else {
		return GetOrDefault(filename...);
	}
}

std::shared_ptr<std::vector<std::string>> ExpandDirectories(nlohmann::json j);

} // namespace ServerConfig

} // namespace Legacy
} // namespace YGOpen

#endif // __YGOPEN_LEGACY_SERVER_CONFIG_HPP__
