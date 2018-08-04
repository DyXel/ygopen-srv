#include "server_acceptor.hpp"
#include <fstream>
#include <sstream>
#include <csignal>

void ServerAcceptor::Close()
{
	acceptor.close();
	for(auto& room : rooms)
		room->Close();
}

std::shared_ptr<ServerRoom> ServerAcceptor::GetAvailableRoom()
{
	if(rooms.size() == 0)
	{
		auto room = std::make_shared<ServerRoom>(this, dbm, ci, bl);
		rooms.insert(room);
		return room;
	}

	auto search = std::find_if(rooms.begin(), rooms.end(), [](std::shared_ptr<ServerRoom> room) -> bool
	{
		return room->GetPlayersNumber() < room->GetMaxPlayers();
	});
	if(search != rooms.end())
		return *search;

	auto room = std::make_shared<ServerRoom>(this, dbm, ci, bl);
	rooms.insert(room);
	return room;
}

void ServerAcceptor::DoSignalWait()
{
	signals.async_wait([this](std::error_code /*ec*/, int /*signo*/)
	{
		Close();
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
			std::make_shared<ServerRoomClient>(std::move(tmpSocket), room.get())->Connect();
		}

		DoAccept();
	});
}

void ServerAcceptor::DeleteRoom(std::shared_ptr<ServerRoom> room)
{
	auto search = rooms.find(room);
	if(search != rooms.end())
		rooms.erase(room);
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
		Close();
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
		Close();
		return;
	}

	DoAccept();
}

ServerAcceptor::~ServerAcceptor()
{
	ci.UnloadLibrary();
}



