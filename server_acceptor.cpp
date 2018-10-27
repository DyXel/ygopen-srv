#include "server_acceptor.hpp"
#include <fstream>
#include <sstream>
#include <csignal>
#include <algorithm>

namespace YGOpen
{
namespace Legacy
{

std::shared_ptr<ServerRoom> ServerAcceptor::GetAvailableRoom()
{
	if(rooms.empty())
	{
		auto room = std::make_shared<ServerRoom>(dbm, ci, bl);
		rooms.push_back(room);
		return room;
	}

	// prune expired rooms
	{
		auto it = rooms.begin();
		while(it != rooms.end())
		{
			if(it->expired())
			{
				it = rooms.erase(it);
				continue;
			}
			
			++it;
		}
	}

	auto search = std::find_if(rooms.begin(), rooms.end(), [](std::weak_ptr<ServerRoom> room) -> bool
	{
		auto tmpPtr = room.lock();
		return tmpPtr->GetPlayersNumber() < tmpPtr->GetMaxPlayers();
	});
	if(search != rooms.end())
		return search->lock();

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
		if(!acceptor.is_open())
			return;

		if(!ec)
		{
			auto room = GetAvailableRoom();
			std::make_shared<ServerRoomClient>(std::move(tmpSocket), room)->Connect();
		}

		DoAccept();
	});
}

ServerAcceptor::ServerAcceptor(asio::io_service& ioService, asio::ip::tcp::endpoint& endpoint) :
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
	dbm.LoadDatabase("cards.cdb");

	if(!ci.LoadLibrary())
	{
		acceptor.close();
		ioService.stop();
		return;
	}

	//ci.set_script_reader(script_reader);
	ci.set_card_reader(&CoreAuxiliary::CoreCardReader);
	ci.set_message_handler(&CoreAuxiliary::CoreMessageHandler);

	std::ifstream f("2018.5 TCG.json");
	std::stringstream buffer;
	buffer << f.rdbuf();
	std::string s = buffer.str();
	if(!bl.FromJSON(s))
	{
		acceptor.close();
		ioService.stop();
		return;
	}

	DoAccept();
}

ServerAcceptor::~ServerAcceptor()
{
	ci.UnloadLibrary();
}

} // namespace Legacy
} // namespace YGOpen
