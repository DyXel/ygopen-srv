#include <iostream>
#include <random>

#include "server_room.hpp" 
#include "string_utils.hpp"

#include "database_manager.hpp"
#include "core_interface.hpp"
#include "banlist.hpp"

#include "enums/core_message.hpp"
#include "enums/location.hpp"
#include "enums/position.hpp"
#include "enums/type.hpp"

namespace YGOpen
{
namespace Legacy
{

bool ServerRoom::IsTag() const
{
	return duelInfo.mode == 0x02;
}

bool ServerRoom::IsRelay() const
{
	return duelInfo.mode == 0x03;
}

int ServerRoom::GetPlayersNumber() const
{
	return players.size();
}

int ServerRoom::GetMaxPlayers() const
{
	if(IsTag())
		return 4;
	else if(IsRelay())
		return 6;
	return 2;
}

int ServerRoom::GetSecondTeamCap() const
{
	if(IsRelay())
		return 3;
	else if(IsTag())
		return 2;
	else
		return 1;
}

int ServerRoom::GetNewPlayerPos(int except) const
{
	assert(!((int)players.size() == GetMaxPlayers()));

	const int maxPlayers = GetMaxPlayers();

	if(except == -1)
	{
		for(int i = 0; i < maxPlayers; ++i)
		{
			if(players.count(i) == 0)
				return i;
		}
	}

	int pos = except;
	while(players.count(pos) == 1)
		pos = (pos + 1) % maxPlayers;
	return pos;
}


void ServerRoom::SwapPlayers()
{
	const int secondTeamCap = GetSecondTeamCap();
	
	// TODO: handle relay variable players
	for(int i = 0; i < secondTeamCap; i++)
	{
		players[i]->pos = i + secondTeamCap;
		players[i + secondTeamCap]->pos = i;
	}
	
	std::set<Client> tmpPlayers;
	for(auto& player : players)
		tmpPlayers.insert(player.second);
	
	players.clear();
	for(auto& player : tmpPlayers)
		players.insert(std::make_pair(player->pos, player));
}

void ServerRoom::SendSpectatorNumber(Client except)
{
	STOCMessage msg(StoC::Msg::HsWatchChange);
	msg.GetBM()->Write<uint16_t>(spectators.size());
	if(except != nullptr)
		SendToAllExcept(except, msg);
	else
		SendToAll(msg);
}

void ServerRoom::SendTo(Client client, STOCMessage msg)
{
	msg.Encode();
	client->PushBackMsg(msg);
}

void ServerRoom::SendToTeam(int team, STOCMessage msg)
{
	if(!IsTag() && !IsRelay())
	{
		SendTo(players[team], msg);
		return;
	}

	msg.Encode();
	
	const int playersPerTeam = GetSecondTeamCap(); // TODO: handle relay better

	if(team == 0)
	{
		for(int i = 0; i < playersPerTeam; i++)
			players[i]->PushBackMsg(msg);
	}
	else if(team == 1)
	{
		for(int i = playersPerTeam; i < playersPerTeam + playersPerTeam; i++)
			players[i]->PushBackMsg(msg);
	}
}

void ServerRoom::SendToAll(STOCMessage msg)
{
	msg.Encode();

	for(auto& c : spectators)
		c->PushBackMsg(msg);

	for(auto& c : players)
		c.second->PushBackMsg(msg);
}

void ServerRoom::SendToAllExcept(Client client, STOCMessage msg)
{
	msg.Encode();

	for(auto& c : spectators)
	{
		if(c != client)
			c->PushBackMsg(msg);
	}

	for(auto& c : players)
	{
		if(c.second != client)
			c.second->PushBackMsg(msg);
	}
}

void ServerRoom::SendToSpectators(STOCMessage msg)
{
	msg.Encode();

	for(auto& c : spectators)
		c->PushBackMsg(msg);
}

void ServerRoom::SendRPS()
{
	state = STATE_RPS;
	
	STOCMessage msg(StoC::Msg::SelectHand);
	SendTo(players[0], msg);
	SendTo(players[GetSecondTeamCap()], msg);
}

void ServerRoom::StartDuel(bool result)
{
	state = STATE_DUEL;

	std::random_device rd;
	std::mt19937 rnd(rd());

	firstTeamObserver.Deinitialize();
	secondTeamObserver.Deinitialize();
	spectatorTeamObserver.Deinitialize();

	duel = std::make_shared<Duel>(ci, rnd());
	std::weak_ptr<Duel> weakDuel = duel;
	firstTeamObserver.SetDuel(weakDuel);
	secondTeamObserver.SetDuel(weakDuel);
	spectatorTeamObserver.SetDuel(weakDuel);

	if(result)
		SwapPlayers();

	const auto secondTeamCap = GetSecondTeamCap();
	for(int i = 0; i < secondTeamCap; i++)
	{
		firstTeamObserver.AddPlayer(i, players[i]);
		secondTeamObserver.AddPlayer(i + secondTeamCap, players[i + secondTeamCap]);
	}
	
	{
		size_t i = 0;
		for(auto& obs : spectators)
		{
			spectatorTeamObserver.AddPlayer(i, obs);
			i++;
		}
	}

	duel->AddObserver(this);

	duel->AddObserver(&firstTeamObserver);
	duel->AddObserver(&secondTeamObserver);
	duel->AddObserver(&spectatorTeamObserver);

	duel->AddObserver(&msgObserver);

	//for(auto& obs : spectators)
	//	duel->AddObserver(obs.get());

	duel->SetPlayerInfo(0, duelInfo.start_lp, duelInfo.start_hand, duelInfo.draw_count);
	duel->SetPlayerInfo(1, duelInfo.start_lp, duelInfo.start_hand, duelInfo.draw_count);
	
	// add cards..
	// TODO: check if tag and relay duels require special treatment
	if(duelInfo.no_shuffle_deck && !decksSorted)
	{
		for(auto& player : players)
			std::reverse(player.second->deck.main.begin(), player.second->deck.main.end());
		decksSorted = true;
	}
	else
	{
		for(auto& player : players)
			std::shuffle(player.second->deck.main.begin(), player.second->deck.main.end(), rnd);
	}

	for(auto& player : players)
	{
		for(auto& code : player.second->deck.main)
			duel->NewCard(code, player.first, player.first, LocationMainDeck, 0, PositionFaceDownDefense);
		
		for(auto& code : player.second->deck.extra)
			duel->NewCard(code, player.first, player.first, LocationExtraDeck, 0, PositionFaceDownDefense);
	}
	
	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();
	bm->Write<uint8_t>(0x4); // MSG_START
	bm->Write<uint8_t>(0);
	bm->Write<int32_t>(duelInfo.start_lp);
	bm->Write<int32_t>(duelInfo.start_lp);
	bm->Write<int16_t>(duel->QueryFieldCount(0, LocationMainDeck));
	bm->Write<int16_t>(duel->QueryFieldCount(0, LocationExtraDeck));
	bm->Write<int16_t>(duel->QueryFieldCount(1, LocationMainDeck));
	bm->Write<int16_t>(duel->QueryFieldCount(1, LocationExtraDeck));
	SendToTeam(0, msg);

	bm->ToStart();
	bm->Forward(1);
	bm->Write<uint8_t>(1);
	bm->Forward(16);
	SendToTeam(1, msg);

	bm->ToStart();
	bm->Forward(1);
	bm->Write<uint8_t>(0x10);
	bm->Forward(16);
	SendToSpectators(msg);

	firstTeamObserver.Start();
	secondTeamObserver.Start();
	spectatorTeamObserver.Start();

	duel->Start(duelInfo.duel_flag);

	duel->Process();
}

void ServerRoom::EndDuel()
{
	// TODO: If match, handle it here

	//duel = nullptr; // Implicitly ends duel

	// TODO: replay sending should be here

	Close();
}

void ServerRoom::Close()
{
	if(state == STATE_END)
		return;
	state = STATE_END;

	STOCMessage msg(StoC::Msg::DuelEnd);
	SendToAll(msg);

	for(auto& client : clients)
		client->Disconnect(false);

	firstTeamObserver.Deinitialize();
	secondTeamObserver.Deinitialize();
	spectatorTeamObserver.Deinitialize();
}

Client ServerRoom::GetHost() const
{
	return hostClient;
}

ServerRoom::ServerRoom(DatabaseManager& dbmanager, CoreInterface& corei, Banlist& bl) :
	dbm(dbmanager),
	ci(corei),
	banlist(bl),
	state(STATE_LOBBY),
	decksSorted(false),
	hostClient(nullptr),
	startPlayer(nullptr),
	firstTeamObserver(0),
	secondTeamObserver(1),
	spectatorTeamObserver(2)
{
	duelInfo = {};
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
	duelInfo.check = 2; // use lua64
	duelInfo.duel_flag = 0x2800;
	duelInfo.forbiddentypes = 0;
	duelInfo.extra_rules = 0;
}

ServerRoom::~ServerRoom()
{}

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

	// avoids endless recursion when calling Surrender
	clients.erase(client);
	
	if(spectators.find(client) != spectators.end())
	{
		spectators.erase(client);
		SendSpectatorNumber(client);
	}
	else
	{
		players.erase(client->pos);
		players_ready.erase(client->pos);
		
		if(state == STATE_DUEL)
			Surrender(client);

		STOCMessage msg(StoC::Msg::HsPlayerChange);
		msg.GetBM()->Write<uint8_t>((client->pos << 0x04) + PLAYERCHANGE_LEAVE);
		SendToAllExcept(client, msg);
	}

	if(hostClient == client)
	{
		if(players.begin() != players.end())
			hostClient = players.begin()->second;
		else if(spectators.begin() != spectators.end())
			hostClient = *spectators.begin();
		else
			hostClient = nullptr;

		if(hostClient != nullptr && state == STATE_LOBBY)
			SendTypeChange(hostClient);
	}

	if(startPlayer == client)
		startPlayer = nullptr;
}

void ServerRoom::OnNotify(void* buffer, size_t length)
{
	BufferManipulator bm(buffer, length);
	
	const auto msgType = (CoreMessage)bm.Read<uint8_t>();
	
	if(msgType == CoreMessage::Win)
	{
		STOCMessage msg(StoC::Msg::GameMsg);
		msg.GetBM()->Write(std::make_pair(buffer, length));
		SendToAll(msg);

		EndDuel();
	}
}

void ServerRoom::Response(Client client, void* buffer, size_t bufferLength)
{
	if(client->type == ServerRoomClient::TYPE_SPECTATOR)
		return;
	
	const bool secondTeamCap = GetSecondTeamCap();
	if(client->pos < secondTeamCap && !firstTeamObserver.IsReponseFlagSet())
		return;
	else if(client->pos >= secondTeamCap && !secondTeamObserver.IsReponseFlagSet())
		return;

	if(bufferLength > 64)
	{
		std::puts("Buffer too long!");
		return;
	}

	duel->SetResponseBuffer(buffer, bufferLength);
	duel->Process();
}

void ServerRoom::AddClient(Client client)
{
	switch(state)
	{
		case STATE_LOBBY:
			AddToLobby(client);
		break;
		case STATE_DUEL:
		case STATE_SIDE:
			AddToGame(client);
		break;
	}
}

void ServerRoom::Surrender(Client client)
{
	if(client->type != ServerRoomClient::TYPE_PLAYER)
		return;
	if(state != STATE_DUEL)
		return;
	
	// Send MSG_WIN
	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();
	bm->Write<uint8_t>(0x5); // MSG_WIN
	bm->Write<uint8_t>(1 - client->pos);
	bm->Write<uint8_t>(0); // REASON_SURRENDER
	SendToAll(msg);
	
	EndDuel();
}

void ServerRoom::UpdateDeck(Client client, std::vector<unsigned int>& mainExtra, std::vector<unsigned int>& side)
{
	std::vector<unsigned int> main;
	std::vector<unsigned int> extra;

	for(auto& code : mainExtra)
	{
		const CardData* cd = dbm.GetCardDataByCode(code);

		if(cd == nullptr)
		{
			printf("WARNING: %d card not found in database\n", code);
			continue;
		}

		if((cd->type & TypeFusion) || (cd->type & TypeSynchro)  ||
		   (cd->type & TypeXyz) || (cd->type & TypeLink))
		{
			extra.push_back(code);
		}
		else
		{
			main.push_back(code);
		}
	}

	client->deck.main = main;
	client->deck.extra = extra;
	client->deck.side = side;
}

void ServerRoom::AddToLobby(Client client)
{
	// The first client to enter will be the host
	if(hostClient == nullptr)
		hostClient = client;

	if((int)players.size() == GetMaxPlayers())
	{
		client->type = ServerRoomClient::TYPE_SPECTATOR;
		client->pos = -1;
		spectators.insert(client);

		SendSpectatorNumber(client);
	}
	else
	{
		int pos = GetNewPlayerPos();
		client->type = ServerRoomClient::TYPE_PLAYER;
		client->pos = pos;
		players.insert(std::make_pair(pos, client));
		players_ready.insert(std::make_pair(pos, false));

		STOCMessage msg(StoC::Msg::HsPlayerEnter);
		StoC::HS_PlayerEnter s = {};
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
		STOCMessage msg1(StoC::Msg::HsPlayerEnter);
		StoC::HS_PlayerEnter s = {};
		std::u16string tmpStr = su::stou16(c.second->GetName());
		for(int i = 0; i < (int)tmpStr.length(); ++i)
			s.name[i] = tmpStr[i];
		s.pos = c.first;
		msg1.GetBM()->Write(s);
		SendTo(client, msg1);
		
		// Refresh ready status
		STOCMessage msg2(StoC::Msg::HsPlayerChange);
		uint8_t val = c.second->GetType(false) << 4;
		val += (players_ready[c.first]) ? PLAYERCHANGE_READY : PLAYERCHANGE_NOTREADY;
		msg2.GetBM()->Write(val);
		SendTo(client, msg2);
	}

	if(spectators.size() > 0)
	{
		STOCMessage msg(StoC::Msg::HsWatchChange);
		msg.GetBM()->Write<uint16_t>(spectators.size());
		SendTo(client, msg);
	}
}

void ServerRoom::AddToGame(Client client)
{
	// TODO
	client->Disconnect(false);
}

void ServerRoom::Chat(Client client, std::string& chatMsg)
{
	STOCMessage msg(StoC::Msg::Chat);

	StoC::Chat s = {};
	s.player = client->GetType(false);
	std::u16string tmpStr = su::stou16(chatMsg);
	for(int i = 0; i < (int)tmpStr.length(); ++i)
		s.msg[i] = tmpStr[i];

	msg.GetBM()->Write(s);
	SendToAll(msg);
}

void ServerRoom::MoveToDuelist(Client client)
{
	if(state != ServerRoom::STATE_LOBBY)
		return;
	if((int)players.size() == GetMaxPlayers())
		return;

	if(client->type == ServerRoomClient::TYPE_PLAYER)
	{
		int pos = GetNewPlayerPos(client->pos);
		if(client->pos == pos)
			return;

		players.erase(client->pos);
		players_ready.erase(client->pos);

		players.insert(std::make_pair(pos, client));
		players_ready.insert(std::make_pair(pos, false));

		STOCMessage msg(StoC::Msg::HsPlayerChange);
		uint8_t val = client->pos << 4;
		val += pos;
		msg.GetBM()->Write(val);
		SendToAll(msg);

		client->pos = pos;
		SendTypeChange(client);
	}
	else
	{
		int pos = GetNewPlayerPos();
		players.insert(std::make_pair(pos, client));
		players_ready.insert(std::make_pair(pos, false));
		spectators.erase(client);

		STOCMessage msg(StoC::Msg::HsPlayerEnter);
		StoC::HS_PlayerEnter s = {};
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

	STOCMessage msg(StoC::Msg::HsPlayerChange);
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
	if(players_ready[client->pos] == ready)
		return;
	
	if(ready)
	{
		unsigned int result = client->deck.Verify(dbm);
		ready = client->deck.IsVerified();

		if(ready)
		{
			result = client->deck.CheckUsability(banlist);
			//ready = client->deck.CanBeUsed();
		}
		else
		{
			STOCMessage msg(StoC::Msg::ErrorMsg);
			auto bm = msg.GetBM();
			bm->Write<uint8_t>(2);
			bm->Write<uint16_t>(0); // Padding
			bm->Write<uint8_t>(0); // Padding

			bm->Write<uint32_t>(result);
			SendTo(client, msg);
		}
	}

	players_ready[client->pos] = ready;

	STOCMessage msg(StoC::Msg::HsPlayerChange);
	uint8_t val = client->GetType(false) << 4;
	val += (ready) ? PLAYERCHANGE_READY : PLAYERCHANGE_NOTREADY;
	msg.GetBM()->Write(val);
	SendToAll(msg);
}

void ServerRoom::RPSHand(Client client, int answer)
{
	if(state != ServerRoom::STATE_RPS)
		return;
	if(client->type == ServerRoomClient::TYPE_SPECTATOR)
		return;
	if(answer < 1 || answer > 3)
		return;
	if(players_rpshand.find(client->pos) != players_rpshand.end())
		return;

	const int stc = GetSecondTeamCap();

	if(client->pos != 0 && client->pos != stc)
		return;

	players_rpshand.insert(std::make_pair(client->pos, answer));

	if(players_rpshand.size() == 2)
	{
		// Send each other the hand result
		STOCMessage msg1(StoC::Msg::HandResult);
		auto bm = msg1.GetBM();
		bm->Write<uint8_t>(players_rpshand[0]);
		bm->Write<uint8_t>(players_rpshand[stc]);
		SendToTeam(0, msg1);
		
		STOCMessage msg2(StoC::Msg::HandResult);
		bm = msg2.GetBM();
		bm->Write<uint8_t>(players_rpshand[stc]);
		bm->Write<uint8_t>(players_rpshand[0]);
		SendToTeam(1, msg2);
		
		SendToSpectators(msg1);
		
		if(players_rpshand[0] == players_rpshand[stc])
		{
			players_rpshand.clear();
			SendRPS();
			return;
		}
		if ((players_rpshand[0] == 1 && players_rpshand[stc] == 2) ||
		    (players_rpshand[0] == 2 && players_rpshand[stc] == 3) ||
		    (players_rpshand[0] == 3 && players_rpshand[stc] == 1))
		{
			startPlayer = players[stc];
		}
		else
		{
			startPlayer = players[0];
		}

		STOCMessage msg3(StoC::Msg::SelectTp);
		SendTo(startPlayer, msg3);
	}
}

void ServerRoom::TPSelect(Client client, bool amIFirst)
{
	if(state != ServerRoom::STATE_RPS)
		return;
	if(client->type == ServerRoomClient::TYPE_SPECTATOR)
		return;
	if(client != startPlayer)
		return;
	
	bool secondTeamfirst = false;
	if(amIFirst && client->pos == GetSecondTeamCap())
		secondTeamfirst = true;
	else if(!amIFirst && client->pos == 0)
		secondTeamfirst = true;
	
	StartDuel(secondTeamfirst);
}

void ServerRoom::Kick(Client client, uint8_t pos)
{
	if(client != hostClient)
		return;
	if(pos > GetMaxPlayers())
		return;
	
	auto result = players.find(pos);
	if(result != players.end() && result->second != client)
		client->Disconnect(false);
}

void ServerRoom::Start(Client client)
{
	if(client != hostClient)
		return;

	for (auto& p : players_ready)
	{
		if(!p.second)
			return;
	}

	STOCMessage msg(StoC::Msg::DuelStart);
	SendToAll(msg);

	SendRPS();
}

void ServerRoom::SendJoinMsg(Client client)
{
	StoC::JoinGame s = {};
	s.info = duelInfo;

	STOCMessage msg(StoC::Msg::JoinGame);
	msg.GetBM()->Write(s);

	SendTo(client, msg);
}

void ServerRoom::SendTypeChange(Client client)
{
	STOCMessage msg(StoC::Msg::TypeChange);
	msg.GetBM()->Write<uint8_t>(client->GetType(true));
	SendTo(client, msg);
}

} // namespace Legacy
} // namespace YGOpen
