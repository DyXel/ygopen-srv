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

nlohmann::json read_config(std::string filename);

nlohmann::json get_or_default(std::string filename);

template<typename... T, typename = typename std::enable_if<std::conjunction<std::is_convertible<T, std::string>...>::value>::type> 
    nlohmann::json get_or_default(std::string first, T const&... filename) {
    if(std::filesystem::exists(first)) {
		return read_config(first);
	} else {
        return get_or_default(filename...);
    }
}

std::shared_ptr<std::vector<std::string>> expand_directories(nlohmann::json j);

} // namespace ServerConfig

} // namespace Legacy
} // namespace YGOpen

#endif // __YGOPEN_LEGACY_SERVER_CONFIG_HPP__
