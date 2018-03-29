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

	bool IsTag() const;
	bool IsRelay() const;
	int GetMaxPlayers() const;

	std::set<Client> clients;
	std::set<Client> spectators;
	std::map<int, Client> players;
	std::map<int, bool> players_ready;
	Client hostClient;

	int GetNewPlayerPos(int except = -1) const;

	void SendSpectatorNumber(Client except = nullptr);
	void SendTo(Client client, STOCMessage msg);
	void SendToAll(STOCMessage msg);
	void SendToAllExcept(Client client, STOCMessage msg);
public:
	enum { STATE_LOBBY, STATE_DUEL, STATE_RPS, STATE_SIDE};

	Client GetHost() const;

	ServerRoom();
	void Join(Client client);
	void Leave(Client client);

	void AddClient(Client client);
	void AddToLobby(Client client);
	void AddToGame(Client client);
	void Chat(Client client, std::string& chatMsg);
	void MoveToDuelist(Client client);
	void MoveToSpectator(Client client);
	void Ready(Client client, bool ready);

	void Kick(Client client, uint8_t pos);
	void Start(Client client);

	void SendJoinMsg(Client client);
	void SendTypeChange(Client client);
};
#endif // __SERVER_ROOM_HPP__
