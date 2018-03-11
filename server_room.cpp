#include "server_room.hpp" 

#include <iostream>
#include "string_utils.hpp"

void ServerRoom::SendSpectatorNumber(Client except)
{
	STOCMessage msg(STOC_HS_WATCH_CHANGE);
	msg.GetBM()->Write<uint16_t>(spectators.size());
	if(except != nullptr)
		SendToAllExcept(except, msg);
	else
		SendToAll(msg);
}

void ServerRoom::SendTo(Client client, STOCMessage msg)
{
	msg.Encode();
	client->outgoingMsgs.push_back(msg);
	client->Flush();
}

void ServerRoom::SendToAll(STOCMessage msg)
{
	msg.Encode();

	for(auto& c : spectators)
	{
		c->outgoingMsgs.push_back(msg);
		c->Flush();
	}

	for(auto& c : players)
	{
		c.second->outgoingMsgs.push_back(msg);
		c.second->Flush();
	}
}

void ServerRoom::SendToAllExcept(Client client, STOCMessage msg)
{
	msg.Encode();

	for(auto& c : spectators)
	{
		if(c != client)
		{
			c->outgoingMsgs.push_back(msg);
			c->Flush();
		}
	}

	for(auto& c : players)
	{
		if(c.second != client)
		{
			c.second->outgoingMsgs.push_back(msg);
			c.second->Flush();
		}
	}
}

Client ServerRoom::GetHost() const
{
	return hostClient;
}

ServerRoom::ServerRoom() :
	state(STATE_LOBBY),
	hostClient(nullptr)
{
	// Defaults.
	duelInfo.lflist = 0;
	duelInfo.rule = 2;
	duelInfo.mode = 0;
	duelInfo.duel_rule = 4;
	duelInfo.no_check_deck = false;
	duelInfo.no_shuffle_deck = false;
	duelInfo.start_lp = 8000;
	duelInfo.start_hand = 5;
	duelInfo.draw_count = 1;
	duelInfo.time_limit = 240;
}

void ServerRoom::Join(Client client)
{
	std::cout << "Client (" << client->WhoAmI() << ") Joins\n";
	clients.insert(client);
}

void ServerRoom::Leave(Client client)
{
	std::cout << "Client (" << client->WhoAmI() << ") Leaves\n";

	if(clients.find(client) == clients.end())
		return;
	
	if(spectators.find(client) != spectators.end())
	{
		spectators.erase(client);
		SendSpectatorNumber(client);
	}
	else
	{
		players.erase(client->pos);
		players_ready.erase(client->pos);

		STOCMessage msg(STOC_HS_PLAYER_CHANGE);
		msg.GetBM()->Write<uint8_t>((client->pos << 0x04) + PLAYERCHANGE_LEAVE);
		SendToAllExcept(client, msg);
	}

	// TODO: properly handle host leaving
	// maybe choose a new host or close connection?
	if(hostClient == client)
		hostClient = nullptr;

	client->Disconnect();
	clients.erase(client);
}

int ServerRoom::GetPlayerPos() const
{
	//               < (isTag) ? 4 : 2
	for(int i = 0; i < 2; ++i)
	{
		if(players.count(i) == 0)
			return i;
	}

	return -1;
}

void ServerRoom::AddToLobby(Client client)
{
	// The first client to enter will be the host
	if(hostClient == nullptr)
		hostClient = client;

	int pos = GetPlayerPos();
	//if(players.size() >= (isTag) ? 4 : 2)
	if(pos == -1)
	{
		client->type = ServerRoomClient::TYPE_SPECTATOR;
		client->pos = pos;
		spectators.insert(client);

		SendSpectatorNumber(client);
	}
	else
	{
		client->type = ServerRoomClient::TYPE_PLAYER;
		client->pos = pos;
		players.insert(std::make_pair(pos, client));
		players_ready.insert(std::make_pair(pos, false));

		STOCMessage msg(STOC_HS_PLAYER_ENTER);
		ygo::STOC_HS_PlayerEnter s = {};
		std::u16string tmpStr = su::stou16(client->GetName());
		for(int i = 0; i < (int)tmpStr.length(); ++i)
			s.name[i] = tmpStr[i];
		s.pos = pos;
		msg.GetBM()->Write(s);
		SendToAllExcept(client, msg);
	}

	SendJoinMsg(client);
	SendTypeChange(client);

	// Update client with lobby info
	for(auto& c : players)
	{
		STOCMessage msg(STOC_HS_PLAYER_ENTER);
		ygo::STOC_HS_PlayerEnter s = {};
		std::u16string tmpStr = su::stou16(c.second->GetName());
		for(int i = 0; i < (int)tmpStr.length(); ++i)
			s.name[i] = tmpStr[i];
		s.pos = c.first;
		msg.GetBM()->Write(s);
		SendTo(client, msg);

		//TODO: Send ready status if player is ready.
	}

	if(spectators.size() > 0)
	{
		STOCMessage msg(STOC_HS_WATCH_CHANGE);
		msg.GetBM()->Write<uint16_t>(spectators.size());
		SendTo(client, msg);
	}
}

void ServerRoom::Chat(Client client, std::string& chatMsg)
{
	STOCMessage msg(STOC_CHAT);

	ygo::STOC_Chat s = {};
	s.player = client->GetType(false);
	std::u16string tmpStr = su::stou16(chatMsg);
	for(int i = 0; i < (int)tmpStr.length(); ++i)
		s.msg[i] = tmpStr[i];

	msg.GetBM()->Write(s);
	SendToAllExcept(client, msg);
}

void ServerRoom::MoveToDuelist(Client client)
{
	if(state != ServerRoom::STATE_LOBBY)
		return;
	if(client->type == ServerRoomClient::TYPE_PLAYER) // TODO: delete this when supporting Tag/Relay
		return;
	//                   (istag) ? 4 : 2)
	if(players.size() >= 2)
		return;

	int pos = GetPlayerPos();
	if(pos == -1) // This shouldn't be needed
		return;

	players.insert(std::make_pair(pos, client));
	players_ready.insert(std::make_pair(pos, false));
	spectators.erase(client);

	STOCMessage msg(STOC_HS_PLAYER_ENTER);
	ygo::STOC_HS_PlayerEnter s = {};
	std::u16string tmpStr = su::stou16(client->GetName());
	for(int i = 0; i < (int)tmpStr.length(); ++i)
		s.name[i] = tmpStr[i];
	s.pos = pos;
	msg.GetBM()->Write(s);
	SendToAll(msg);

	client->type = ServerRoomClient::TYPE_PLAYER;
	client->pos  = pos;
	SendTypeChange(client);

	SendSpectatorNumber();
}

void ServerRoom::MoveToSpectator(Client client)
{
	if(state != ServerRoom::STATE_LOBBY)
		return;
	if(client->type == ServerRoomClient::TYPE_SPECTATOR)
		return;
	//if(players_ready[client->pos])
	//	return;

	players.erase(client->pos);
	players_ready.erase(client->pos);
	spectators.insert(client);

	STOCMessage msg(STOC_HS_PLAYER_CHANGE);
	uint8_t val = client->GetType(false) << 4;
	val += NETPLAYER_TYPE_OBSERVER;
	msg.GetBM()->Write(val);
	SendToAll(msg);

	client->type = ServerRoomClient::TYPE_SPECTATOR;
	client->pos  = -1;
	SendTypeChange(client);

	SendSpectatorNumber();
}

void ServerRoom::Ready(Client client, bool ready)
{
	if(state != ServerRoom::STATE_LOBBY)
		return;
	if(client->type == ServerRoomClient::TYPE_SPECTATOR)
		return;

	//TODO: Check deck here

	players_ready[client->pos] = ready;

	STOCMessage msg(STOC_HS_PLAYER_CHANGE);
	uint8_t val = client->GetType(false) << 4;
	val += (ready) ? PLAYERCHANGE_READY : PLAYERCHANGE_NOTREADY;
	msg.GetBM()->Write(val);
	SendToAll(msg);
}

void ServerRoom::SendJoinMsg(Client client)
{
	ygo::STOC_JoinGame s = {};
	s.info = duelInfo;

	STOCMessage msg(STOC_JOIN_GAME);
	msg.GetBM()->Write(s);

	SendTo(client, msg);
}

void ServerRoom::SendTypeChange(Client client)
{
	STOCMessage msg(STOC_TYPE_CHANGE);
	//ygo::STOC_TypeChange s = {};
	msg.GetBM()->Write<uint8_t>(client->GetType(true));

	SendTo(client, msg);
}
