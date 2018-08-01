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
	CoreAuxiliary::SetCore(&ci);
	CoreAuxiliary::SetDatabaseManager(&dbm);
	dbm.LoadDatabase("cards.cdb");

	if(!ci.LoadLibrary())
		return;
	std::cout << "Core loaded successfully\n";

	auto script_reader = [](const char* file, int* slen) -> unsigned char*
	{
		std::cout << "attempt to load script: " << file << std::endl;
		
		std::ifstream f(file, std::ios::in | std::ios::binary);
		if(f.is_open())
		{
			std::string contents;
			f.seekg(0, std::ios::end);
			contents.resize(f.tellg());
			f.seekg(0, std::ios::beg);
			f.read(&contents.front(), contents.size());
			f.close();
			*slen = contents.size();
			return (unsigned char*)&contents.front();
		}
		
		std::cout << "file could not be loaded." << std::endl;
		return 0;
	};
	//ci.set_script_reader(script_reader);
	ci.set_card_reader(&CoreAuxiliary::CoreCardReader);
	ci.set_message_handler(&CoreAuxiliary::CoreMessageHandler);

	std::ifstream f("2018.5 TCG.json");
	std::stringstream buffer;
	buffer << f.rdbuf();
	std::string s = buffer.str();
	if(!bl.FromJSON(s))
		return;

	DoAccept();
}

ServerAcceptor::~ServerAcceptor()
{
	ci.UnloadLibrary();
}
