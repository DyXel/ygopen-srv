#ifndef __SERVER_ACCEPTOR_HPP__
#define __SERVER_ACCEPTOR_HPP__
#include <asio.hpp>
#include <vector>
#include <memory>
#include "server_room.hpp"

#include "database_manager.hpp"
#include "core_interface.hpp"
#include "core_auxiliary.hpp"
#include "banlist.hpp"

#include "duel.hpp"

namespace YGOpen
{
namespace Legacy
{

class ServerAcceptor
{
	asio::signal_set signals;
	asio::ip::tcp::acceptor acceptor;
	asio::ip::tcp::socket tmpSocket;
	
	DatabaseManager dbm;
	CoreInterface ci;
	Banlist bl;
	
	bool LoadDatabases(std::vector<std::string>& databases);
	bool LoadBanlist();

	std::vector<std::weak_ptr<ServerRoom>> rooms;

	std::shared_ptr<ServerRoom> GetAvailableRoom();

	void DoSignalWait();
	void DoAccept();
public:
	ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint, std::vector<std::string>& databases, std::vector<std::string>& banlists);
	~ServerAcceptor();
};

} // namespace Legacy
} // namespace YGOpen

#endif // __SERVER_ACCEPTOR_HPP__
