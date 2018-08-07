#ifndef __SERVER_ROOM_CLIENT_HPP__
#define __SERVER_ROOM_CLIENT_HPP__
#include <memory>
#include <string>
#include <memory>
#include <asio.hpp>
#include <deque>

#include "server_message.hpp"
#include "deck.hpp"

class ServerRoom;

class ServerRoomClient : public std::enable_shared_from_this<ServerRoomClient>
{
	// Client State
	std::string name;
	std::string pass;
	bool auth;

	bool ParseMsg();
	void OnResponse(BufferManipulator* bm);
	void OnPlayerInfo(BufferManipulator* bm);
	void OnUpdateDeck(BufferManipulator* bm);
	void OnJoinGame(BufferManipulator* bm);
	void OnSurrender(BufferManipulator* bm);
	void OnChat(BufferManipulator* bm);
	void OnMoveToDuelist();
	void OnMoveToSpectator();
	void OnReady();
	void OnNotReady();
	void OnRPSHand(BufferManipulator* bm);
	void OnTPSelect(BufferManipulator* bm);

	// Host commands
	void OnKickPlayer(BufferManipulator* bm);
	void OnStart();

	// Asio related
	asio::ip::tcp::socket socket;
	std::shared_ptr<ServerRoom> room;
	CTOSMessage receivedMsg;
	bool closing;
	std::string whoami;

	void DoReadHeader();
	void DoReadBody();

	void DoWrite();

	std::deque<STOCMessage> outgoingMsgs;
public:
	enum { TYPE_PLAYER, TYPE_SPECTATOR };
	Deck deck;

	int type; // Player or Spectator?
	int pos; // Player position, index start from 0

	ServerRoomClient(asio::ip::tcp::socket, std::shared_ptr<ServerRoom>);
	~ServerRoomClient();

	const std::string& WhoAmI() const;
	const std::string& GetName() const;
	int GetType(bool getHost) const;

	void Connect();
	void Disconnect(bool force);

	void PushBackMsg(STOCMessage msg);
};

#endif // __SERVER_ROOM_CLIENT_HPP__
