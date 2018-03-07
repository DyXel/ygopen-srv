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

	void SendTo(Client client, STOCMessage msg);
	void SendToAll(STOCMessage msg);
	void SendToAllExcept(Client client, STOCMessage msg);
public:
	enum { STATE_LOBBY, STATE_DUEL, STATE_SIDE};

	ServerRoom();
	void Join(Client client);
	void Leave(Client client);

	int GetPlayerPos();
	void AddToLobby(Client client);

	void SendJoinMsg(Client client);
	void SendTypeChange(Client client);
};
#endif // __SERVER_ROOM_HPP__
