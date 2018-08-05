#include "server_room_client.hpp"
#include "server_room.hpp"

#include <sstream>

#include "string_utils.hpp"

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
		case CTOS_SURRENDER:
			OnSurrender(&bm);
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

void ServerRoomClient::OnSurrender(BufferManipulator* bm)
{
	room->Surrender(shared_from_this());
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

