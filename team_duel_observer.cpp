#include "team_duel_observer.hpp" 
#include "server_room_client.hpp"

#include "core_messages.hpp"

#include <set>

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

static const std::set<int> knowledgeMsgs =
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

const bool TeamDuelObserver::IsReponseFlagSet() const
{
	return responseFlag;
}

void TeamDuelObserver::OnNotify(void* buffer, size_t length)
{
	responseFlag = false;
	BufferManipulator bm(buffer, length);

	const auto msgType = bm.Read<uint8_t>();
	
	// Check for blacklisted message
	if(msgType == CoreMessage::Win)
		return;

	// Check for messages to this team
	const auto search = playerMsgs.find((int)msgType) != playerMsgs.end();
	if(search)
	{
		if(bm.Read<uint8_t>() == team)
			responseFlag = true;
		else
			return;
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
					responseFlag = true;
				else
					return;
			}
			break;
			case CoreMessage::Hint:
			{
				const auto type = bm.Read<uint8_t>();
				const auto forPlayer = bm.Read<uint8_t>();
				if((type == 1 || type == 2 || type == 3 || type == 5) && forPlayer != team)
					return;
				else if ((type == 4 || type == 6 || type == 7 || type == 8 || type == 9) && forPlayer == team)
					return;
				// type == 10 // always send the message
			}
			break;
			case CoreMessage::ConfirmCards:
			{
				const auto forPlayer = bm.Read<uint8_t>();
				bm.Forward(1 + 4);
				if (bm.Read<uint8_t>() == 0x01 && forPlayer != team) // 0x01 == LOCATION_DECK
					return;
			}
			break;
		}
	}

	// Knowledge stripping
	std::string newMsg;
	const auto search2 = knowledgeMsgs.find((int)msgType) != knowledgeMsgs.end();
	if(search2)
	{
		newMsg.reserve((size_t)length);
		std::memcpy(&newMsg[0], buffer, length);
		BufferManipulator bm2(&newMsg[0], length);

		bm2.Forward(1); // msgType

		auto DeleteNonPublicKnowledge = [&bm2](const uint8_t forPlayer)
		{
			const auto count = bm2.Read<uint32_t>();
			for(uint32_t i = 0; i < count; i++)
			{
				bm2.Forward(4); // code
				const auto controler = bm2.Read<uint8_t>();
				if(forPlayer != controler)
				{
					bm2.Backward(4 + 1);
					bm2.Write<uint32_t>(0);
					bm2.Forward(1); // controler
				}
				bm2.Forward(9); // loc_info - controler
			}
		};
		
		switch(msgType)
		{
			case CoreMessage::SelectCard:
			{
				const auto forPlayer = bm2.Read<uint8_t>();
				bm2.Forward(3);
				DeleteNonPublicKnowledge(forPlayer);
			}
			break;
			case CoreMessage::SelectTribute:
			{
				const auto forPlayer = bm2.Read<uint8_t>();
				bm2.Forward(3);
				const auto count = bm2.Read<uint32_t>();
				for(uint32_t i = 0; i < count; i++)
				{
					bm2.Forward(4); // code
					const auto controler = bm2.Read<uint8_t>();
					if(forPlayer != controler)
					{
						bm2.Backward(4 + 1);
						bm2.Write<uint32_t>(0);
						bm2.Forward(1); // controler
					}
					bm2.Forward(6); // ???
				}
			}
			break;
			case CoreMessage::SelectUnselect:
			{
				const auto forPlayer = bm2.Read<uint8_t>();
				bm2.Forward(4);
				DeleteNonPublicKnowledge(forPlayer);
				DeleteNonPublicKnowledge(forPlayer);
			}
			break;
			case CoreMessage::ShuffleHand:
			case CoreMessage::ShuffleExtra:
			{
				const auto forPlayer = bm2.Read<uint8_t>();
				if(forPlayer != team)
				{
					const auto count = bm2.Read<uint8_t>();
					for(uint32_t i = 0; i < count; i++)
						bm2.Write<uint32_t>(0);
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

				bm2.Forward(4); // code
				bm2.Forward(10); // loc_info previous
				const auto currentControler = bm2.Read<uint8_t>();
				const auto currentLocation = bm2.Read<uint8_t>();
				bm2.Forward(4); // loc_info.sequence
				const auto currentPosition = bm2.Read<uint32_t>();
				bm2.Forward(4); // ?

				if(currentControler != team && !IsCardPublic(currentLocation, currentPosition))
				{
					bm2.Backward(4 + 10 + 10 + 4);
					bm2.Write<uint32_t>(0);
				}
			}
			break;
			case CoreMessage::Set:
			{
				bm2.Write<uint32_t>(0);
			}
			break;
			case CoreMessage::Draw:
			{
				const auto forPlayer = bm2.Read<uint8_t>();
				if(forPlayer != team)
				{
					const auto count = bm2.Read<uint8_t>();
					for(uint8_t i = 0; i < count; i++)
					{
						const auto code = bm2.Read<uint32_t>();
						bm2.Backward(4);
						if(code & 0x80000000) // if card is POS_FACEUP
							bm2.Forward(4);
						else
							bm2.Write<uint32_t>(0);
					}
				}
			}
			break;
		}
	}

	// Send message
	STOCMessage msg(STOC_GAME_MSG);
	if(!search2)
		msg.GetBM()->Write(std::make_pair(buffer, length));
	else
		msg.GetBM()->Write(std::make_pair((void*)&newMsg[0], length));
	msg.Encode();

	for(auto& player : players)
		player.second->PushBackMsg(msg);
}
