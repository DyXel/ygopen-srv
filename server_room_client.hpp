#ifndef __SERVER_ROOM_CLIENT_HPP__
#define __SERVER_ROOM_CLIENT_HPP__
#include <memory>
#include <string>
#include <string>
#include <asio.hpp>
#include <deque>

#include "server_message.hpp"

class ServerRoom;

class ServerRoomClient : public std::enable_shared_from_this<ServerRoomClient>
{
	// Client State
	std::string name;
	std::string pass;
	bool auth;

	void ParseMsg();
	void OnPlayerInfo(BufferManipulator* bm);
	void OnJoinGame(BufferManipulator* bm);

	asio::ip::tcp::socket socket;
	ServerRoom* room;
	CTOSMessage receivedMsg;
	bool flushing;

	void DoReadHeader();
	void DoReadBody();

	void DoWrite();
public:
	enum { TYPE_PLAYER, TYPE_SPECTATOR };
	int type; // Player or Spectator?
	int pos; // Player position

	ServerRoomClient(asio::ip::tcp::socket, ServerRoom*);
	~ServerRoomClient();

	std::string WhoAmI();
	std::string GetName();

	void Connect();
	void Disconnect();

	std::deque<STOCMessage> outgoingMsgs;

	void Flush();
};
#endif // __SERVER_ROOM_CLIENT_HPP__
