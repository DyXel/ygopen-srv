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
		if(!ec)
		{
			ParseMsg();
			DoReadHeader();
		}
		else
		{
			room->Leave(shared_from_this());
		}
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

void ServerRoomClient::ParseMsg()
{
	BufferManipulator bm(receivedMsg.GetDataPtr(), receivedMsg.GetMsgLength());
	uint8_t msgType = receivedMsg.GetMsgType();

	//TODO: map all of these functions
	switch(msgType)
	{
		case CTOS_PLAYER_INFO:
			OnPlayerInfo(&bm);
		return;
		case CTOS_JOIN_GAME:
			OnJoinGame(&bm);
		return;
		case CTOS_CREATE_GAME:
			//OnCreateGame();
			auth = true;
		return;
	}

	if(auth == false)
	{
		std::cout << "Received unexpected package before auth" << std::endl;
		return;
	}

	switch(msgType)
	{
		case CTOS_CHAT:
			OnChat(&bm);
		break;
		default:
			std::cout << "Unhandled message: " << msgType << std::endl;
		break;
	}
}

void ServerRoomClient::OnPlayerInfo(BufferManipulator* bm)
{
	name = su::u16tos(std::u16string((const char16_t*)bm->GetCurrentBuffer().first));
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

	room->AddToLobby(shared_from_this());
	auth = true;
}

ServerRoomClient::ServerRoomClient(asio::ip::tcp::socket tmpSocket, ServerRoom* room) :
	socket(std::move(tmpSocket)),
	room(room),
	flushing(false)
{}

ServerRoomClient::~ServerRoomClient()
{}

std::string ServerRoomClient::WhoAmI()
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

std::string ServerRoomClient::GetName()
{
	return name;
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

