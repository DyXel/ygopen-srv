#include "server_room_client.hpp"
#include "server_room.hpp"

#include <sstream>

#include "string_utils.hpp"

#include "core_messages.hpp"

void ServerRoomClient::DoReadHeader()
{
	auto self(shared_from_this());
	asio::async_read(socket, asio::buffer(receivedMsg.GetDataPtr(), CTOSMessage::HEADER_LENGTH),
	[this, self](std::error_code ec, std::size_t)
	{
		if(!ec && receivedMsg.DecodeHeader())
			DoReadBody();
		else
			room->Leave(shared_from_this());
	});
}

void ServerRoomClient::DoReadBody()
{
	auto self(shared_from_this());
	asio::async_read(socket, asio::buffer(receivedMsg.GetDataPtr(), receivedMsg.GetMsgLength()),
	[this, self](std::error_code ec, std::size_t)
	{
		if(!ec && ParseMsg())
			DoReadHeader();
		else
			room->Leave(shared_from_this());
	});
}

void ServerRoomClient::DoWrite()
{
	auto self(shared_from_this());
	asio::async_write(socket, asio::buffer(outgoingMsgs.front().GetDataPtr(), outgoingMsgs.front().GetLength()),
	[this, self](std::error_code ec, std::size_t)
	{
		if(!ec)
		{
			outgoingMsgs.pop_front();
			if(!outgoingMsgs.empty())
				DoWrite();
			else
				flushing = false;
		}
		else
		{
			room->Leave(shared_from_this());
			flushing = false;
		}
	});
}

bool ServerRoomClient::ParseMsg()
{
	BufferManipulator bm(receivedMsg.GetDataPtr(), receivedMsg.GetMsgLength());
	uint8_t msgType = receivedMsg.GetMsgType();

	switch(msgType)
	{
		case CTOS_PLAYER_INFO:
			OnPlayerInfo(&bm);
		return true;
		case CTOS_JOIN_GAME:
			OnJoinGame(&bm);
		return true;
		case CTOS_CREATE_GAME:
			//OnCreateGame();
			auth = true;
		return true;
	}

	if(auth == false)
	{
		std::cout << "Received unexpected package before auth" << std::endl;
		return false;
	}

	switch(msgType)
	{
		case CTOS_RESPONSE:
			OnResponse(&bm);
		break;
		case CTOS_CHAT:
			OnChat(&bm);
		break;
		case CTOS_UPDATE_DECK:
			OnUpdateDeck(&bm);
		break;
		case CTOS_HS_TODUELIST:
			OnMoveToDuelist();
		break;
		case CTOS_HS_TOOBSERVER:
			OnMoveToSpectator();
		break;
		case CTOS_HS_READY:
			OnReady();
		break;
		case CTOS_HS_NOTREADY:
			OnNotReady();
		break;
		case CTOS_HS_KICK:
			OnKickPlayer(&bm);
		break;
		case CTOS_HS_START:
			OnStart();
		break;
		case CTOS_HAND_RESULT:
			OnRPSHand(&bm);
			break;
		case CTOS_TP_RESULT:
			OnTPSelect(&bm);
			break;
		default:
			std::cout << "Unhandled message: " << (int)msgType << std::endl;
			return false;
		break;
	}

	return true;
}

void ServerRoomClient::OnResponse(BufferManipulator* bm)
{
	const auto b = bm->GetCurrentBuffer();
	std::cout << pos << ": Setting response..." << std::endl;
	room->Response(shared_from_this(), b.first, b.second);
}

void ServerRoomClient::OnPlayerInfo(BufferManipulator* bm)
{
	name = su::u16tos(std::u16string((const char16_t*)bm->GetCurrentBuffer().first));
}

void ServerRoomClient::OnUpdateDeck(BufferManipulator* bm)
{
	const int mainCount = bm->Read<uint32_t>();
	const int sideCount = bm->Read<uint32_t>();
	
	std::vector<uint32_t> main;
	std::vector<uint32_t> side;

	for(int i = 0; i < mainCount; i++)
		main.push_back(bm->Read<uint32_t>());
	for(int i = 0; i < sideCount; i++)
		side.push_back(bm->Read<uint32_t>());

	room->UpdateDeck(shared_from_this(), main, side);
}

void ServerRoomClient::OnJoinGame(BufferManipulator* bm)
{
	/*
	uint16_t version = bm->Read<uint16_t>();
	bm->Foward(2); // Needed because of struct packing issues
	uint32_t gameid  = bm->Read<uint32_t>();
	*/
	bm->Forward(8);

	pass = su::u16tos(std::u16string((const char16_t*)bm->GetCurrentBuffer().first));
	
	room->AddClient(shared_from_this());
	auth = true;
}

void ServerRoomClient::OnChat(BufferManipulator* bm)
{
	std::string chatMsg = su::u16tos(std::u16string((const char16_t*)bm->GetCurrentBuffer().first));
	room->Chat(shared_from_this(), chatMsg);
}

void ServerRoomClient::OnMoveToDuelist()
{
	room->MoveToDuelist(shared_from_this());
}

void ServerRoomClient::OnMoveToSpectator()
{
	room->MoveToSpectator(shared_from_this());
}

void ServerRoomClient::OnReady()
{
	room->Ready(shared_from_this(), true);
}

void ServerRoomClient::OnNotReady()
{
	room->Ready(shared_from_this(), false);
}

void ServerRoomClient::OnKickPlayer(BufferManipulator* bm)
{
	room->Kick(shared_from_this(), bm->Read<uint8_t>());
}

void ServerRoomClient::OnStart()
{
	room->Start(shared_from_this());
}

void ServerRoomClient::OnRPSHand(BufferManipulator* bm)
{
	room->RPSHand(shared_from_this(), bm->Read<uint8_t>());
}

void ServerRoomClient::OnTPSelect(BufferManipulator* bm)
{
	room->TPSelect(shared_from_this(), (bool)bm->Read<uint8_t>());
}

ServerRoomClient::ServerRoomClient(asio::ip::tcp::socket tmpSocket, ServerRoom* room) :
	socket(std::move(tmpSocket)),
	room(room),
	flushing(false),
	leaved(false)
{}

ServerRoomClient::~ServerRoomClient()
{
	std::cout << "SRC: Calling destructor" << std::endl;
}

static const std::set<int> playerMsgs =
{
	CoreMessage::SelectBattleCmd,
	CoreMessage::SelectIdleCmd,
	CoreMessage::SelectEffectYn,
	CoreMessage::SelectYesNo,
	CoreMessage::SelectOption,
	CoreMessage::SelectCard,
	CoreMessage::SelectTribute,
	CoreMessage::SelectUnselect,
	CoreMessage::SelectChain,
	CoreMessage::SelectPlace,
	CoreMessage::SelectPosition,
	CoreMessage::SelectDisfield,
	CoreMessage::SelectCounter,
	CoreMessage::SelectSum,
	CoreMessage::SortCard,
	CoreMessage::SortChain,
	CoreMessage::MissedEffect,
	CoreMessage::RockPaperScissors,
	CoreMessage::AnnounceRace,
	CoreMessage::AnnounceAttrib,
	CoreMessage::AnnounceCard,
	CoreMessage::AnnounceNumber,
	CoreMessage::AnnounceCardFilter
};

void ServerRoomClient::OnNotify(void* buffer, size_t length)
{
	BufferManipulator bm(buffer, length);

	if(type == -1) // Do not handle spectators here
		return;

	const auto msgType = bm.Read<uint8_t>();

	// First check for messages to this player
	auto search = playerMsgs.find((int)msgType);
	if(search != playerMsgs.end())
	{
		if(bm.Read<uint8_t>() == pos)
			room->WaitforResponse(shared_from_this());
		else
			return;
	}
	else
	{
		// Special cases here
		switch(msgType)
		{
			case CoreMessage::Hint:
			{
				const auto type = bm.Read<uint8_t>();
				const auto forPlayer = bm.Read<uint8_t>();
				if((type == 1 || type == 2 || type == 3 || type == 5) && forPlayer != pos)
					return;
				else if ((type == 4 || type == 6 || type == 7 || type == 8 || type == 9) && forPlayer == pos)
					return;
				// type == 10 // always send the message
			}
			break;
			case CoreMessage::ConfirmCards:
			{
				const auto forPlayer = bm.Read<uint8_t>();
				bm.Forward(1 + 4);
				if (bm.Read<uint8_t>() == 0x01 && forPlayer != pos) // 0x01 == LOCATION_DECK
					return;
			}
			break;
		}
	}

	// Strip any knowledge that is not meant for this player
	//TODO

	// Send message
	STOCMessage msg(STOC_GAME_MSG);
	msg.GetBM()->Write(std::make_pair(buffer, length));
	msg.Encode();
	outgoingMsgs.push_back(msg);
	Flush();
}

std::string ServerRoomClient::WhoAmI() const
{
	std::ostringstream out;
	asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();

	asio::ip::address addr = endpoint.address();
	unsigned short port = endpoint.port();

	out << "Address: ";
	out << addr.to_string();
	out << ". Port: ";
	out << port;

	return out.str();
}

std::string ServerRoomClient::GetName() const
{
	return name;
}

int ServerRoomClient::GetType(bool getHost) const
{
	uint8_t ret;
	ret = (type == ServerRoomClient::TYPE_PLAYER) ? pos : NETPLAYER_TYPE_OBSERVER;
	if(getHost)
		ret += (shared_from_this() == room->GetHost()) ? 0x10 : 0x0;

	return ret;
}

void ServerRoomClient::Connect()
{
	room->Join(shared_from_this());
	DoReadHeader();
}

void ServerRoomClient::Disconnect()
{
	socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	socket.close();
}

void ServerRoomClient::Flush()
{
	if(flushing)
		return;

	if(!outgoingMsgs.empty())
	{
		flushing = true;
		DoWrite();
	}
}

