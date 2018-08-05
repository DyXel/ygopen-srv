#ifndef __SERVER_ROOM_CLIENT_HPP__
#define __SERVER_ROOM_CLIENT_HPP__
#include <memory>
#include <string>
#include <string>
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

	asio::ip::tcp::socket socket;
	ServerRoom* room;
	CTOSMessage receivedMsg;

	void DoReadHeader();
	void DoReadBody();

	void DoWrite();
public:
	enum { TYPE_PLAYER, TYPE_SPECTATOR };
	Deck deck;

	bool flushing;
	bool leaved;
	// NOTE: i think the flags above are a mistake

	int type; // Player or Spectator?
	int pos; // Player position, index start from 0

	ServerRoomClient(asio::ip::tcp::socket, ServerRoom*);
	~ServerRoomClient();

	std::string WhoAmI() const;
	std::string GetName() const;
	int GetType(bool getHost) const;

	void Connect();
	void Disconnect();

	std::deque<STOCMessage> outgoingMsgs;

	void Flush();
};

#endif // __SERVER_ROOM_CLIENT_HPP__
