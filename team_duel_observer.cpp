#include "team_duel_observer.hpp" 
#include "server_room_client.hpp"

#include "enums/core_message.hpp"

#include <set>

static const std::set<CoreMessage> playerMsgs =
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

static const std::set<CoreMessage> knowledgeMsgs =
{
	CoreMessage::SelectCard,
	CoreMessage::SelectTribute,
	CoreMessage::SelectUnselect,
	CoreMessage::ShuffleHand,
	CoreMessage::ShuffleExtra,
	CoreMessage::Move,
	CoreMessage::Set,
	CoreMessage::Draw
};

TeamDuelObserver::TeamDuelObserver(short team) :
	team(team),
	responseFlag(false)
{}

void TeamDuelObserver::AddPlayer(int pos, std::shared_ptr<ServerRoomClient> player)
{
	players.insert(std::make_pair(pos, player));
}

void TeamDuelObserver::ClearPlayers()
{
	players.clear();
}

const bool TeamDuelObserver::IsReponseFlagSet() const
{
	return responseFlag;
}

bool TeamDuelObserver::IsMsgForThisTeam(void* buffer, size_t length)
{
	BufferManipulator bm(buffer, length);
	const auto msgType = (CoreMessage)bm.Read<uint8_t>();
	
	// Check for blacklisted message
	if(msgType == CoreMessage::Win)
		return false;

	// Check for messages to this team
	const auto search = playerMsgs.find(msgType) != playerMsgs.end();
	if(search)
	{
		if(bm.Read<uint8_t>() == team)
			return (responseFlag = true);
		else
			return false;
	}
	else
	{
		// Special cases here
		switch(msgType)
		{
			case CoreMessage::SelectSum:
			{
				bm.Forward(1);
				if(bm.Read<uint8_t>() == team)
					return (responseFlag = true);
				else
					return false;
			}
			break;
			case CoreMessage::Hint:
			{
				const auto type = bm.Read<uint8_t>();
				const auto forPlayer = bm.Read<uint8_t>();
				if((type == 1 || type == 2 || type == 3 || type == 5) && forPlayer != team)
					return false;
				else if ((type == 4 || type == 6 || type == 7 || type == 8 || type == 9) && forPlayer == team)
					return false;
				// type == 10 // always send the message
			}
			break;
			case CoreMessage::ConfirmCards:
			{
				const auto forPlayer = bm.Read<uint8_t>();
				bm.Forward(1 + 4);
				if (bm.Read<uint8_t>() == 0x01 && forPlayer != team) // 0x01 == LOCATION_DECK
					return false;
			}
			break;
			default:
				std::abort();
			break;
		}
	}

	return true;
}

bool TeamDuelObserver::StripKnowledge(void* buffer, size_t length, std::string& newMsg)
{
	newMsg.reserve((size_t)length);
	std::memcpy(&newMsg[0], buffer, length);
	BufferManipulator bm(&newMsg[0], length);

	const auto msgType = (CoreMessage)bm.Read<uint8_t>();

	if(knowledgeMsgs.find(msgType) == knowledgeMsgs.end())
		return false;

	auto DeleteNonPublicKnowledge = [&bm](const uint8_t forPlayer)
	{
		const auto count = bm.Read<uint32_t>();
		for(uint32_t i = 0; i < count; i++)
		{
			bm.Forward(4); // code
			const auto controler = bm.Read<uint8_t>();
			if(forPlayer != controler)
			{
				bm.Backward(4 + 1);
				bm.Write<uint32_t>(0);
				bm.Forward(1); // controler
			}
			bm.Forward(9); // loc_info - controler
		}
	};
	
	switch(msgType)
	{
		case CoreMessage::SelectCard:
		{
			const auto forPlayer = bm.Read<uint8_t>();
			bm.Forward(3);
			DeleteNonPublicKnowledge(forPlayer);
		}
		break;
		case CoreMessage::SelectTribute:
		{
			const auto forPlayer = bm.Read<uint8_t>();
			bm.Forward(3);
			const auto count = bm.Read<uint32_t>();
			for(uint32_t i = 0; i < count; i++)
			{
				bm.Forward(4); // code
				const auto controler = bm.Read<uint8_t>();
				if(forPlayer != controler)
				{
					bm.Backward(4 + 1);
					bm.Write<uint32_t>(0);
					bm.Forward(1); // controler
				}
				bm.Forward(6); // ???
			}
		}
		break;
		case CoreMessage::SelectUnselect:
		{
			const auto forPlayer = bm.Read<uint8_t>();
			bm.Forward(4);
			DeleteNonPublicKnowledge(forPlayer);
			DeleteNonPublicKnowledge(forPlayer);
		}
		break;
		case CoreMessage::ShuffleHand:
		case CoreMessage::ShuffleExtra:
		{
			const auto forPlayer = bm.Read<uint8_t>();
			if(forPlayer != team)
			{
				const auto count = bm.Read<uint8_t>();
				for(uint32_t i = 0; i < count; i++)
					bm.Write<uint32_t>(0);
			}
		}
		break;
		case CoreMessage::Move:
		{
			// TODO: use enums for positions and locations
			auto IsCardPublic = [](const uint8_t location, const uint32_t position) -> const bool
			{
				if(location & (0x10 + 0x80) && !(location & (0x01 + 0x02)))
					return true;
				else if(!(position & 0x0a))
					return true;
				return false;
			};

			bm.Forward(4); // code
			bm.Forward(10); // loc_info previous
			const auto currentControler = bm.Read<uint8_t>();
			const auto currentLocation = bm.Read<uint8_t>();
			bm.Forward(4); // loc_info.sequence
			const auto currentPosition = bm.Read<uint32_t>();
			bm.Forward(4); // ?

			if(currentControler != team && !IsCardPublic(currentLocation, currentPosition))
			{
				bm.Backward(4 + 10 + 10 + 4);
				bm.Write<uint32_t>(0);
			}
		}
		break;
		case CoreMessage::Set:
		{
			bm.Write<uint32_t>(0);
		}
		break;
		case CoreMessage::Draw:
		{
			const auto forPlayer = bm.Read<uint8_t>();
			if(forPlayer != team)
			{
				const auto count = bm.Read<uint8_t>();
				for(uint8_t i = 0; i < count; i++)
				{
					const auto code = bm.Read<uint32_t>();
					bm.Backward(4);
					if(code & 0x80000000) // if card is POS_FACEUP
						bm.Forward(4);
					else
						bm.Write<uint32_t>(0);
				}
			}
		}
		break;
		default:
			std::abort();
		break;
	}
	
	return true;
}

void TeamDuelObserver::OnNotify(void* buffer, size_t length)
{
	responseFlag = false;
	
	if(!IsMsgForThisTeam(buffer, length))
		return;

	STOCMessage msg(STOC_GAME_MSG);
	
	std::string newMsg;
	if(StripKnowledge(buffer, length, newMsg))
		msg.GetBM()->Write(std::make_pair((void*)&newMsg[0], length));
	else
		msg.GetBM()->Write(std::make_pair(buffer, length));
	msg.Encode();
	
	for(auto& player : players)
		player.second->PushBackMsg(msg);
}
