#include "server_acceptor.hpp"
#include <fstream>
#include <sstream>

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
	ci(false),
	room(&dbm, &ci, &bl)
{
	CoreAuxiliary::SetDatabaseManager(&dbm);
	dbm.LoadDatabase("cards.cdb");

	if(!ci.LoadLibrary())
		return;
	std::cout << "Core loaded successfully\n";

	auto f = [](const char*, int*) -> unsigned char*
	{
		return (unsigned char*)"";
	};
	ci.set_script_reader(f);
	ci.set_card_reader(&CoreAuxiliary::CoreCardReader);
	ci.set_message_handler(&CoreAuxiliary::CoreMessageHandler);

	std::ifstream t("2018.5 TCG.json");
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string s = buffer.str();
	if(!bl.FromJSON(s))
		return;

	DoAccept();
}

ServerAcceptor::~ServerAcceptor()
{
	ci.UnloadLibrary();
}
