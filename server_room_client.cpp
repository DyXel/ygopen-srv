#include "server_room_client.hpp"
#include "server_room.hpp"

#include <sstream>
#include <exception>

#include "string_utils.hpp"

namespace YGOpen
{
namespace Legacy
{

void ServerRoomClient::DoReadHeader()
{
	auto self(shared_from_this());
	asio::async_read(socket, asio::buffer(receivedMsg.GetDataPtr(), CTOSMessage::HEADER_LENGTH),
	[this, self](std::error_code ec, std::size_t)
	{
		if(!ec && receivedMsg.DecodeHeader())
			DoReadBody();
		else if(ec != asio::error::operation_aborted)
			Disconnect(false);
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
		else if(ec != asio::error::operation_aborted)
			Disconnect(false);
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
			else if(closing)
				Disconnect(true);
		}
		else if(ec != asio::error::operation_aborted)
		{
			Disconnect(false);
		}
	});
}

bool ServerRoomClient::ParseMsg()
{
	BufferManipulator bm(receivedMsg.GetDataPtr(), receivedMsg.GetMsgLength());
	CtoS::Msg msgType = receivedMsg.GetMsgType();

	switch(msgType)
	{
		case CtoS::Msg::PlayerInfo:
			OnPlayerInfo(&bm);
		return true;
		case CtoS::Msg::JoinGame:
			OnJoinGame(&bm);
		return true;
		case CtoS::Msg::CreateGame:
			//OnCreateGame();
			auth = true;
		return true;
		default:
		break;
	}

	if(auth == false)
	{
		std::cout << "Received unexpected package before auth" << std::endl;
		return false;
	}

	switch(msgType)
	{
		case CtoS::Msg::Response:
			OnResponse(&bm);
		break;
		case CtoS::Msg::Chat:
			OnChat(&bm);
		break;
		case CtoS::Msg::Surrender:
			OnSurrender(&bm);
			// TODO: reason of surrender might be needed
		break;
		case CtoS::Msg::UpdateDeck:
			OnUpdateDeck(&bm);
		break;
		case CtoS::Msg::HsToDuelist:
			OnMoveToDuelist();
		break;
		case CtoS::Msg::HsToObserver:
			OnMoveToSpectator();
		break;
		case CtoS::Msg::HsReady:
			OnReady();
		break;
		case CtoS::Msg::HsNotReady:
			OnNotReady();
		break;
		case CtoS::Msg::HsKick:
			OnKickPlayer(&bm);
		break;
		case CtoS::Msg::HsStart:
			OnStart();
		break;
		case CtoS::Msg::HandResult:
			OnRPSHand(&bm);
			break;
		case CtoS::Msg::TpResult:
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

ServerRoomClient::ServerRoomClient(asio::ip::tcp::socket tmpSocket, std::shared_ptr<ServerRoom> room) :
	socket(std::move(tmpSocket)),
	room(room),
	closing(false)
{
	std::ostringstream out;
	asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();

	asio::ip::address addr = endpoint.address();
	auto port = endpoint.port();

	out << "Address: ";
	out << addr.to_string();
	out << ". Port: ";
	out << port;

	whoami = out.str();
}

ServerRoomClient::~ServerRoomClient()
{
	std::cout << "SRC: Calling destructor" << std::endl;
}

const std::string& ServerRoomClient::WhoAmI() const
{
	return whoami;
}

const std::string& ServerRoomClient::GetName() const
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

void ServerRoomClient::Disconnect(bool force)
{
	if(force || outgoingMsgs.empty())
	{
		room->Leave(shared_from_this());
		try
		{
			socket.shutdown(asio::ip::tcp::socket::shutdown_both);
			socket.close();
		}
		catch(std::exception& e)
		{
			std::printf("WARNING: exception when trying to close client socket: %s\n", e.what());
		}
	}
	else
	{
		closing = true;
	}
}

void ServerRoomClient::PushBackMsg(STOCMessage msg)
{
	const bool writeInProgress = !outgoingMsgs.empty();
	outgoingMsgs.push_back(msg);
	if(!writeInProgress)
		DoWrite();
}

} // namespace Legacy
} // namespace YGOpen

