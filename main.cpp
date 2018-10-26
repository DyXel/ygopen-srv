#include <iostream>
#include <exception>
#include <asio.hpp>
#include "server_acceptor.hpp"

int main()
{
	asio::io_service ioService;
	asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 44444);

	YGOpen::Legacy::ServerAcceptor sa(ioService, endpoint);
	
	ioService.run();

	return 0;
}
