#ifndef __SERVER_ACCEPTOR_HPP__
#define __SERVER_ACCEPTOR_HPP__
#include <asio.hpp>
#include "server_room.hpp"

#include "core_interface.hpp"
#include "core_auxiliary.hpp"

#include "database_manager.hpp"

#include "duel.hpp"

class ServerAcceptor
{
	asio::ip::tcp::socket tmpSocket;
	asio::ip::tcp::acceptor acceptor;

	DatabaseManager dbm;
	CoreInterface ci;
	Banlist bl;

	ServerRoom room;

	void DoAccept();
public:
	ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint);
	~ServerAcceptor();
};

#endif // __SERVER_ACCEPTOR_HPP__
