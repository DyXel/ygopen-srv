#include <iostream>
#include <exception>
#include <asio.hpp>
#include "server_acceptor.hpp"

int main()
{
	//std::puts("Creating duel");
	//Duel duel(&ci);
	//std::puts("Starting duel");
	//duel.SetPlayersInfo(8000, 0, 0);
	//duel.Start(0x2810); // DUEL_PZONE + DUEL_EMZONE + NoShuffleDeck
	//duel.Process();

	asio::io_service ioService;
	asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 44444);

	ServerAcceptor sa(ioService, endpoint);
	
	ioService.run();

	return 0;
}
