#ifndef __SERVER_ROOM_HPP__
#define __SERVER_ROOM_HPP__
#include <set>
#include <map>
#include <memory>
#include <cstdint>
#include "server_room_client.hpp"
#include "server_message.hpp"

typedef std::shared_ptr<ServerRoomClient> Client;

class ServerRoom
{
	// Room State
	int state; // Lobby State
	ygo::HostInfo duelInfo;

	std::set<Client> clients;
	std::set<Client> observers;
	std::map<int, Client> players;
	std::map<int, bool> players_ready;
	Client hostClient;

	int GetPlayerPos() const;

	void SendTo(Client client, STOCMessage msg);
	void SendToAll(STOCMessage msg);
	void SendToAllExcept(Client client, STOCMessage msg);
public:
	enum { STATE_LOBBY, STATE_DUEL, STATE_SIDE};

	Client GetHost() const;

	ServerRoom();
	void Join(Client client);
	void Leave(Client client);

	void AddPlayer(Client client);
	void AddToGame(Client client);
	void AddToLobby(Client client);
	void Chat(Client client, std::string& chatMsg);

	void SendJoinMsg(Client client);
	void SendTypeChange(Client client);
};
#endif // __SERVER_ROOM_HPP__