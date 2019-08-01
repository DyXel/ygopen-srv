#include <iostream>
#include <exception>
#include <asio.hpp>
#include "config.hpp"
#include "server_acceptor.hpp"

int main()
{
	nlohmann::json config = YGOpen::Legacy::ServerConfig::GetOrDefault("ygopen-srv.json", "ygopen.json", "config.json");
	asio::io_service ioService;
	auto protocol = asio::ip::tcp::v4();
	if(config["protocol"] == "tcp6") {
		protocol = asio::ip::tcp::v6();
	}
	asio::ip::tcp::endpoint endpoint(protocol, static_cast<unsigned short>(config["port"]));
	std::shared_ptr<std::vector<std::string>> databases = YGOpen::Legacy::ServerConfig::ExpandDirectories(config["databases"]);
	std::shared_ptr<std::vector<std::string>> banlists = YGOpen::Legacy::ServerConfig::ExpandDirectories(config["banlists"]);
	YGOpen::Legacy::ServerAcceptor sa(ioService, endpoint, *databases, *banlists);
	
	ioService.run();

	return 0;
}
