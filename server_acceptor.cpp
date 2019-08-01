#include "server_acceptor.hpp"
#include <fstream>
#include <sstream>
#include <csignal>
#include <algorithm>

#include <nlohmann/json.hpp>

namespace YGOpen
{
namespace Legacy
{

bool ServerAcceptor::LoadDatabases(std::vector<std::string>& databases)
{
	try
	{
		for(auto& s : databases)
			dbm.LoadDatabase(s.c_str());
	}
	catch(std::exception& e)
	{
		std::cout << "Exception occurred: " << e.what() << std::endl;
		return false;
	}
	
	return true;
}

bool ServerAcceptor::LoadBanlist()
{
	try
	{
		std::ifstream f("config/banlists/2018.9 TCG.json");
		nlohmann::json j;
		f >> j;
		
		return bl.FromJSON(j);
	}
	catch(std::exception& e)
	{
		std::cout << "Exception occurred: " << e.what() << std::endl;
		return false;
	}
}

std::shared_ptr<ServerRoom> ServerAcceptor::GetAvailableRoom()
{
	if (!rooms.empty())
	{
		// prune expired rooms
		rooms.erase(std::remove_if(rooms.begin(), rooms.end(), [](std::weak_ptr<ServerRoom> &room) -> bool {
						return room.expired();
					}),
					rooms.end());

		// search for room and return that if found
		auto search = std::find_if(rooms.begin(), rooms.end(), [](std::weak_ptr<ServerRoom> &room) -> bool {
			auto tmpPtr = room.lock();
			return tmpPtr->GetPlayersNumber() < tmpPtr->GetMaxPlayers();
		});
		if (search != rooms.end())
			return std::move(search->lock());
	}

	auto room = std::make_shared<ServerRoom>(dbm, ci, bl);
	rooms.push_back(room);
	return room;
}

void ServerAcceptor::DoSignalWait()
{
	signals.async_wait([this](std::error_code /*ec*/, int /*signo*/)
	{
		acceptor.close();
	});
}

void ServerAcceptor::DoAccept()
{
	acceptor.async_accept(tmpSocket, [this](std::error_code ec)
	{
		if(!acceptor.is_open()) // TODO: check if this is needed
			return;

		if(!ec)
		{
			auto room = GetAvailableRoom();
			std::make_shared<ServerRoomClient>(std::move(tmpSocket), room)->Connect();
		}

		DoAccept();
	});
}

ServerAcceptor::ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint, std::vector<std::string>& databases, std::vector<std::string>& banlists) :
	signals(ioService),
	acceptor(ioService, endpoint),
	tmpSocket(ioService),
	ci(false)
{
	signals.add(SIGINT);
	signals.add(SIGTERM);
#if defined(SIGQUIT)
	signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

	DoSignalWait();

	// TODO: load config

	CoreAuxiliary::SetCore(&ci);
	CoreAuxiliary::SetDatabaseManager(&dbm);
	
	if(!LoadDatabases(databases))
	{
		acceptor.close();
		ioService.stop();
		return;
	}

	if(!ci.LoadCore())
	{
		acceptor.close();
		ioService.stop();
		return;
	}

	//ci.set_script_reader(script_reader);
	ci.set_card_reader(&CoreAuxiliary::CoreCardReader);
	ci.set_message_handler(&CoreAuxiliary::CoreMessageHandler);

	DoAccept();
}

ServerAcceptor::~ServerAcceptor()
{
	ci.UnloadCore();
}

} // namespace Legacy
} // namespace YGOpen

