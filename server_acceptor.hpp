#ifndef __SERVER_ACCEPTOR_HPP__
#define __SERVER_ACCEPTOR_HPP__
#include <asio.hpp>
#include <set>
#include <memory>
#include "server_room.hpp"

#include "database_manager.hpp"
#include "core_interface.hpp"
#include "core_auxiliary.hpp"
#include "banlist.hpp"

#include "duel.hpp"

class ServerAcceptor
{
	asio::signal_set signals;
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket tmpSocket;
	
	DatabaseManager dbm;
	CoreInterface ci;
	Banlist bl;

	void Close();
	
	std::set<std::shared_ptr<ServerRoom>> rooms;

	std::shared_ptr<ServerRoom> GetAvailableRoom();

	void DoSignalWait();
	void DoAccept();
public:
	void DeleteRoom(std::shared_ptr<ServerRoom> room);
	
	ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint);
	~ServerAcceptor();
};

#endif // __SERVER_ACCEPTOR_HPP__
