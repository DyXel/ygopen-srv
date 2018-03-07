#include "server_acceptor.hpp"

void ServerAcceptor::DoAccept()
{
	acceptor.async_accept(tmpSocket, [this](std::error_code ec)
	{
		if(!ec)
		{
			std::make_shared<ServerRoomClient>(std::move(tmpSocket), &room)->Connect();
		}

		DoAccept();
	});
}

ServerAcceptor::ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint) :
	tmpSocket(ioService),
	acceptor(ioService, endpoint),
	room()
{
	DoAccept();
}
