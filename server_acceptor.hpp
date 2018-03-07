#ifndef __SERVER_ACCEPTOR_HPP__
#define __SERVER_ACCEPTOR_HPP__
#include <asio.hpp>
#include "server_room.hpp"

class ServerAcceptor
{
	asio::ip::tcp::socket tmpSocket;
	asio::ip::tcp::acceptor acceptor;

	ServerRoom room;

	void DoAccept();
public:
	ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint);
};

#endif // __SERVER_ACCEPTOR_HPP__
