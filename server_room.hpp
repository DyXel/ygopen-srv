#ifndef __SERVER_ROOM_HPP__
#define __SERVER_ROOM_HPP__
#include <set>
#include <map>
#include <memory>
#include <cstdint>
#include "server_room_client.hpp"
#include "server_message.hpp"

#include "core_interface.hpp"
#include "duel.hpp"

typedef std::shared_ptr<ServerRoomClient> Client;

class ServerRoom
{
	CoreInterface* ci;
	std::shared_ptr<Duel> duel;

	int state;
	ygo::HostInfo duelInfo;

	bool IsTag() const;
	bool IsRelay() const;
	int GetMaxPlayers() const;
	int GetSecondTeamCap() const;

	std::set<Client> clients;
	std::set<Client> spectators;
	std::map<int, Client> players;
	std::map<int, bool> players_ready;
	std::map<int, int> players_rpshand;
	Client startPlayer; // Player who decides the first turn duelist
	Client hostClient;

	int GetNewPlayerPos(int except = -1) const;

	void SendSpectatorNumber(Client except = nullptr);
	void SendTo(Client client, STOCMessage msg);
	void SendToTeam(int team, STOCMessage msg);
	void SendToAll(STOCMessage msg);
	void SendToAllExcept(Client client, STOCMessage msg);
	void SendToSpectators(STOCMessage msg);
	
	void SendRPS();
	void StartDuel(bool result);
public:
	enum { STATE_LOBBY, STATE_DUEL, STATE_RPS, STATE_SIDE};

	Client GetHost() const;

	ServerRoom(CoreInterface* corei);
	void Join(Client client);
	void Leave(Client client);

	void AddClient(Client client);
	void AddToLobby(Client client);
	void AddToGame(Client client);
	void Chat(Client client, std::string& chatMsg);
	void MoveToDuelist(Client client);
	void MoveToSpectator(Client client);
	void Ready(Client client, bool ready);
	void RPSHand(Client client, int answer);
	void TPSelect(Client client, bool amIFirst);
	
	void Kick(Client client, uint8_t pos);
	void Start(Client client);

	void SendJoinMsg(Client client);
	void SendTypeChange(Client client);
};
#endif // __SERVER_ROOM_HPP__
