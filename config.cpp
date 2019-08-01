#include "config.hpp"
#include <fstream>

namespace YGOpen
{
namespace Legacy
{

namespace ServerConfig {

nlohmann::json Get(std::string filename) {
	std::ifstream config_file(filename);
	nlohmann::json config = nlohmann::json::parse(config_file);
	return config;
}

nlohmann::json GetOrDefault(std::string filename) {
	if (std::filesystem::exists(filename)) {
		return Get(filename);
	}
	else {
		std::ofstream config_file(filename);
		config_file << DEFAULT_CONFIG.dump(4);
		return DEFAULT_CONFIG;
	}
}

std::shared_ptr<std::vector<std::string>> ExpandDirectories(nlohmann::json j) {
	const auto& array = j.get<std::vector<std::string>>();
	auto result = std::make_shared<std::vector<std::string>>();
	for (const auto& str : array) {
		if (std::filesystem::exists(str)) {
			if (std::filesystem::is_directory(str)) {
				auto listing = std::filesystem::recursive_directory_iterator(str);
				std::transform(begin(listing), end(listing), std::back_inserter(*result), [](const std::filesystem::directory_entry& file) {
					return file.path().string();
				});
			}
			else {
				result->push_back(str);
			}
		}
	}
	return result;
}

} // namespace ServerConfig

} // namespace Legacy
} // namespace YGOpen