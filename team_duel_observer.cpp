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

const bool TeamDuelObserver::IsReponseFlagSet() const
{
	return responseFlag;
}

void TeamDuelObserver::OnNotify(void* buffer, size_t length)
{
	responseFlag = false;
	BufferManipulator bm(buffer, length);

	const auto msgType = bm.Read<uint8_t>();

	// Check for messages to this team
	auto search = playerMsgs.find((int)msgType);
	if(search != playerMsgs.end())
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

	// TODO: Strip any knowledge that is not meant for this team

	// Send message
	STOCMessage msg(STOC_GAME_MSG);
	msg.GetBM()->Write(std::make_pair(buffer, length));
	msg.Encode();

	for(auto& player : players)
	{
		player.second->outgoingMsgs.push_back(msg);
		player.second->Flush();
	}
}
