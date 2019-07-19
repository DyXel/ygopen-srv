#include "team_duel_observer.hpp" 

#include "network.hpp"
#include "server_room_client.hpp"

#include "enums/core_message.hpp"
#include "enums/location.hpp"
#include "enums/position.hpp"

#include <set>
namespace YGOpen
{
namespace Legacy
{

// Messages that are meant to set the response flag and be answered by the team
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
	CoreMessage::SelectSum,
	CoreMessage::SelectCounter,
	CoreMessage::SortCard,
	CoreMessage::SortChain,
	CoreMessage::MissedEffect,
	CoreMessage::RockPaperScissors,
	CoreMessage::AnnounceRace,
	CoreMessage::AnnounceAttrib,
	CoreMessage::AnnounceCard,
	CoreMessage::AnnounceNumber
};

// Messages that require removing knowledge in case they are not meant for the team
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

/*
NOTE: Not needed on current implementation, switch case used instead


// Messages that require performing queries before the actual message is sent
static const std::set<CoreMessage> beforeMsgQueries =
{
	CoreMessage::SelectBattleCmd,
	CoreMessage::SelectIdleCmd,
	CoreMessage::NewTurn,
	CoreMessage::FlipSummoning
};

// Messages that require performing queries after the actual message is sent
static const std::set<CoreMessage> afterMsgQueries =
{
	CoreMessage::ShuffleDeck,
	CoreMessage::ShuffleHand,
	CoreMessage::ShuffleExtra,
	CoreMessage::SwapGraveDeck,
	CoreMessage::ReverseDeck,
	CoreMessage::ShuffleSetCard,
	CoreMessage::NewPhase,
	CoreMessage::Move,
	CoreMessage::PosChange,
	CoreMessage::Swap,
	CoreMessage::Summoned,
	CoreMessage::SpSummoned,
	CoreMessage::FlipSummoned,
	CoreMessage::Chained,
	CoreMessage::ChainSolved,
	CoreMessage::ChainEnd,
	CoreMessage::DamageStepStart,
	CoreMessage::DamageStepEnd
};
*/

TeamDuelObserver::TeamDuelObserver(short team) :
	team(team),
	responseFlag(false)
{}

void TeamDuelObserver::AddPlayer(int pos, std::shared_ptr<ServerRoomClient> player)
{
	players.insert(std::make_pair(pos, player));
}

void TeamDuelObserver::SetDuel(std::weak_ptr<Duel> weakDuel)
{
	duel = weakDuel;
}

void TeamDuelObserver::Start()
{
	if(team < 2)
		QueryExtra(team);
}

void TeamDuelObserver::Deinitialize()
{
	players.clear();
	duel.reset();
}

bool TeamDuelObserver::IsReponseFlagSet() const
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
				bm.Forward(4 + 4);
				if (bm.Read<uint8_t>() == LocationMainDeck && forPlayer != team)
					return false;
			}
			break;
			default: break;
		}
	}

	return true;
}

bool TeamDuelObserver::IsCardPublic(const uint8_t location, const uint32_t position) const
{
	if(location & (LocationGraveyard + LocationOverlay) && !(location & (LocationMainDeck + LocationHand)))
		return true;
	else if(!(position & PositionFaceDown))
		return true;
	return false;
}

bool TeamDuelObserver::StripMessageKnowledge(void* buffer, size_t length, std::string& newMsg) const
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
			bm.Forward(9);
			DeleteNonPublicKnowledge(forPlayer);
		}
		break;
		case CoreMessage::SelectTribute:
		{
			const auto forPlayer = bm.Read<uint8_t>();
			bm.Forward(9);
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
			bm.Forward(10);
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
				const auto count = bm.Read<uint32_t>();
				for(uint32_t i = 0; i < count; i++)
					bm.Write<uint32_t>(0);
			}
		}
		break;
		case CoreMessage::Move:
		{
			bm.Forward(4); // code
			bm.Forward(10); // loc_info previous
			const auto currentControler = bm.Read<uint8_t>();
			const auto currentLocation = bm.Read<uint8_t>();
			bm.Forward(4); // loc_info.sequence
			const auto currentPosition = bm.Read<uint32_t>();
			bm.Forward(4); // reason

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
				const auto count = bm.Read<uint32_t>();
				for(uint8_t i = 0; i < count; i++)
				{
					const auto code = bm.Read<uint32_t>();
					bm.Backward(4);
					if(code & 0x80000000) // if card is PositionFaceUp
						bm.Forward(4);
					else
						bm.Write<uint32_t>(0);
				}
			}
		}
		break;
		default: break;
	}
	
	return true;
}

bool TeamDuelObserver::StripQueryKnowledge(void* buffer, size_t length, std::string& newMsg) const
{
	newMsg.reserve(length);
	std::memcpy(&newMsg[0], buffer, length);
	bool wasKnowledgeDeleted = false;
	
	BufferManipulator bm(&newMsg[0], length);
	while(bm.CanAdvance())
	{
		// On the first int32, the full length is written
		// only if card::get_infos() gets called successfully
		// otherwise this will be 4, as in, only the length
		// of the variable that stores the length was written
		const auto queryLength = bm.Read<uint32_t>();

		// If the card is null, then continue to the next card query
		if(queryLength == 4)
			continue;

		// On the second int32 the flags used on this query are written
		// plus
		// *p++ from QUERY_CODE
		bm.Forward(4 + 4);
		
		// The last byte is the position of the card, not the whole int32
		// supposedly done for compatibility with other servers
		bm.Forward(3);
		const auto position = bm.Read<uint8_t>();
		
		// Go back to just in front of queryLength
		bm.Backward(4 + 4 + 3 + 1);
		
		if(position & PositionFaceDown)
		{
			auto tmpBuffer = bm.GetCurrentBuffer();
			
			// since the queryLength variable is also included in the length we need to exclude it.
			std::memset(tmpBuffer.first, 0, queryLength - 4);
			wasKnowledgeDeleted = true;
		}
		
		// Go to next query
		bm.Forward(queryLength - 4);
	}
	
	return wasKnowledgeDeleted;
}

/************************/
void TeamDuelObserver::QueryLocation(int location, int flag, bool useCache)
{
	auto duelPtr = duel.lock();
	for(int i = 0; i <= 1; i++)
	{
		STOCMessage msg(StoC::Msg::GameMsg);
		auto bm = msg.GetBM();

		bm->Write((uint8_t)CoreMessage::UpdateData);
		bm->Write<uint8_t>(i); // Player
		bm->Write<uint8_t>(location);

		auto buffer = duelPtr->QueryFieldCard(i, location, flag, useCache);

		if(team == i)
		{
			bm->Write(buffer);
		}
		else
		{
			std::string newMsg;
			StripQueryKnowledge(buffer.first, buffer.second, newMsg);
			bm->Write(std::make_pair((void*)&newMsg[0], buffer.second));
		}
		
		msg.Encode();
		for(auto& player : players)
			player.second->PushBackMsg(msg);
	}
}

void TeamDuelObserver::QueryMonsterZones(int flag, bool useCache)
{
	if(!duel.expired())
		QueryLocation(LocationMonsterZone, flag, useCache);
}

void TeamDuelObserver::QuerySpellZones(int flag, bool useCache)
{
	if(!duel.expired())
		QueryLocation(LocationSpellZone, flag, useCache);
}

void TeamDuelObserver::QueryHand(int playerPos, int flag, bool useCache)
{
	if(duel.expired())
		return;
	auto duelPtr = duel.lock();

	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();

	bm->Write((uint8_t)CoreMessage::UpdateData);
	bm->Write<uint8_t>(playerPos); 
	bm->Write<uint8_t>(LocationHand);
	
	auto buffer = duelPtr->QueryFieldCard(playerPos, LocationHand, flag | QueryIsPublic, useCache);
	
	if(team == playerPos)
	{
		bm->Write(buffer);
	}
	else
	{
		// This is similar to StripQueryKnowledge but not quite the same
		std::string newMsg;
		newMsg.reserve(buffer.second);
		std::memcpy(&newMsg[0], buffer.first, buffer.second);
		
		BufferManipulator bm2(&newMsg[0], buffer.second);
		while(bm2.CanAdvance())
		{
			// Consideration for the reader: please dont try to
			// write code like this one anywhere else.
			const auto queryLength = bm2.Read<uint32_t>();

			auto tmpBuffer = bm2.GetCurrentBuffer();

			const auto queryFlags = bm2.Read<uint32_t>();
			bm2.Backward(4 + 4);

			int isPublicPos = queryLength - 4;

			if(queryFlags & QueryLScale)
				isPublicPos -= 4;
			if(queryFlags & QueryRSCale)
				isPublicPos -= 4;
			if(queryFlags & QueryLink)
				isPublicPos -= 8; // both link rating and link markers
			
			bm2.Forward(isPublicPos);
			
			const auto isPublicQuery = bm2.Read<uint32_t>();
			
			if(!isPublicQuery)
				std::memset(tmpBuffer.first, 0, queryLength - 4);
			
			bm2.Backward(isPublicPos + 4);
			bm2.Forward(queryLength);
		}
		
		bm->Write(std::make_pair((void*)&newMsg[0], buffer.second));
	}

	msg.Encode();
	for(auto& player : players)
		player.second->PushBackMsg(msg);
}

void TeamDuelObserver::QueryGrave(int playerPos, int flag, bool useCache)
{
	if(duel.expired())
		return;
	auto duelPtr = duel.lock();
	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();

	bm->Write((uint8_t)CoreMessage::UpdateData);
	bm->Write<uint8_t>(playerPos); 
	bm->Write<uint8_t>(LocationGraveyard);

	auto buffer = duelPtr->QueryFieldCard(playerPos, LocationGraveyard, flag, useCache);

	bm->Write(buffer);

	msg.Encode();
	for(auto& player : players)
		player.second->PushBackMsg(msg);
}

void TeamDuelObserver::QueryExtra(int playerPos, int flag, bool useCache)
{
	if(duel.expired())
		return;
	auto duelPtr = duel.lock();
	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();

	bm->Write((uint8_t)CoreMessage::UpdateData);
	bm->Write<uint8_t>(playerPos);
	bm->Write<uint8_t>(LocationExtraDeck);

	auto buffer = duelPtr->QueryFieldCard(playerPos, LocationExtraDeck, flag, useCache);

	bm->Write(buffer);

	msg.Encode();
	for(auto& player : players)
		player.second->PushBackMsg(msg);
}

void TeamDuelObserver::QuerySingle(int playerPos, int location, int sequence, int flag)
{
	if(duel.expired())
		return;
	auto duelPtr = duel.lock();
	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();

	bm->Write((uint8_t)CoreMessage::UpdateCard);
	bm->Write<uint8_t>(playerPos);
	bm->Write<uint8_t>(location);
	bm->Write<uint8_t>(sequence);
	
	auto buffer = duelPtr->QueryCard(playerPos, location, sequence, flag);
	
	if(team != playerPos)
	{
		BufferManipulator bm2(buffer);
		// queryLength = 4 bytes
		// queryFlag = 4 bytes
		// code = 4 bytes
		// 3 padding bytes
		bm2.Forward(4 + 4 + 4 + 3);
		const auto position = bm2.Read<uint8_t>();
		if(!IsCardPublic(location, position))
			return;
	}

	bm->Write(buffer);

	msg.Encode();
	for(auto& player : players)
		player.second->PushBackMsg(msg);
}

void TeamDuelObserver::QueryDeckPseudo(int playerPos, int flag)
{
	if(duel.expired())
		return;
	auto duelPtr = duel.lock();
	STOCMessage msg(StoC::Msg::GameMsg);
	auto bm = msg.GetBM();

	bm->Write((uint8_t)CoreMessage::UpdateData);
	bm->Write<uint8_t>(playerPos);
	bm->Write<uint8_t>(LocationMainDeck);

	// TODO: handle ignoreCache
	//auto buffer = duelPtr->QueryFieldCard(playerPos, LocationMainDeck, flag, false, true);

	/*
	bm->Write(buffer);

	msg.Encode();
	for(auto& player : players)
		player.second->PushBackMsg(msg);
	*/
	// TODO: this is only written to a replay
}
/************************/

void TeamDuelObserver::HandleBeforeMsgQueries(void* buffer, size_t length)
{
	BufferManipulator bm(buffer, length);
	const auto msgType = (CoreMessage)bm.Read<uint8_t>();

	switch(msgType)
	{
		case CoreMessage::SelectBattleCmd:
		case CoreMessage::SelectIdleCmd:
		case CoreMessage::NewTurn:
		{
			QueryMonsterZones();
			QuerySpellZones();
			QueryHand(0);
			QueryHand(1);
		}
		break;
		case CoreMessage::FlipSummoning:
		{
			bm.Forward(4); // probably card code
			const auto controler = bm.Read<uint8_t>();
			const auto location = bm.Read<uint8_t>();
			const auto sequence = bm.Read<uint32_t>();
			QuerySingle(controler, location, sequence);
		}
		break;
		default:
		break;
	}
}

void TeamDuelObserver::HandleAfterMsgQueries(void* buffer, size_t length)
{
	BufferManipulator bm(buffer, length);
	const auto msgType = (CoreMessage)bm.Read<uint8_t>();
	
	switch(msgType)
	{
		case CoreMessage::ShuffleDeck:
		{
			const auto player = bm.Read<uint8_t>();
			QueryDeckPseudo(player);
		}
		break;
		case CoreMessage::ShuffleHand:
		{
			const auto player = bm.Read<uint8_t>();
			QueryHand(player, handDefQueryFlag, false);
		}
		break;
		case CoreMessage::ShuffleExtra:
		{
			const auto player = bm.Read<uint8_t>();
			QueryExtra(player);
		}
		break;
		case CoreMessage::SwapGraveDeck:
		{
			const auto player = bm.Read<uint8_t>();
			QueryGrave(player);
		}
		break;
		case CoreMessage::ReverseDeck:
		{
			QueryDeckPseudo(0);
			QueryDeckPseudo(1);
		}
		break;
		case CoreMessage::ShuffleSetCard:
		{
			const auto location = bm.Read<uint8_t>();
			if(location == LocationMonsterZone)
				QueryMonsterZones(basicDefQueryFlag + QueryIsPublic, false);
			else
				QuerySpellZones(basicDefQueryFlag + QueryIsPublic, false);
		}
		break;
		case CoreMessage::NewPhase:
		case CoreMessage::Chained:
		case CoreMessage::ChainSolved:
		case CoreMessage::ChainEnd:
		{
			QueryMonsterZones();
			QuerySpellZones();
			QueryHand(0);
			QueryHand(1);
		}
		break;
		case CoreMessage::Move:
		{
			bm.Forward(4); // previousCode
			const auto previousControler = bm.Read<uint8_t>();
			const auto previousLocation = bm.Read<uint8_t>();
			bm.Forward(4 + 4); // previousSequence + previousPosition
			const auto currentControler = bm.Read<uint8_t>();
			const auto currentLocation = bm.Read<uint8_t>();
			const auto currentSequence = bm.Read<uint32_t>();
			
			if(currentLocation == 0)
				return;
			if((currentLocation & LocationOverlay) != 0)
				return;
			if(currentLocation == previousLocation && currentControler == previousControler)
				return;

			QuerySingle(currentControler, currentLocation, currentSequence);
		}
		break;
		case CoreMessage::PosChange:
		{
			bm.Forward(4); // code
			const auto currentControler = bm.Read<uint8_t>();
			const auto currentLocation = bm.Read<uint8_t>();
			const auto currentSequence = bm.Read<uint8_t>();
			const auto previousPosition = bm.Read<uint8_t>();
			const auto currentPosition = bm.Read<uint8_t>();
			if((previousPosition & PositionFaceDown) &&
			   (currentPosition & PositionFaceUp))
				QuerySingle(currentControler, currentLocation, currentSequence);
		}
		break;
		case CoreMessage::Swap:
		{
			bm.Forward(4); // previousCode
			const auto previousControler = bm.Read<uint8_t>();
			const auto previousLocation = bm.Read<uint8_t>();
			const auto previousSequence = bm.Read<uint32_t>();
			bm.Forward(4 + 4); // previousPosition + currentCode
			const auto currentControler = bm.Read<uint8_t>();
			const auto currentLocation = bm.Read<uint8_t>();
			const auto currentSequence = bm.Read<uint32_t>();
			QuerySingle(previousControler, previousLocation, previousSequence);
			QuerySingle(currentControler, currentLocation, currentSequence);
		}
		break;
		case CoreMessage::Summoned:
		case CoreMessage::SpSummoned:
		case CoreMessage::FlipSummoned:
		{
			QueryMonsterZones();
			QuerySpellZones();
		}
		break;
		case CoreMessage::DamageStepStart:
		case CoreMessage::DamageStepEnd:
		{
			QueryMonsterZones();
		}
		break;
		default:
		break;
	}
}

void TeamDuelObserver::OnNotify(void* buffer, size_t length)
{
	responseFlag = false;
	
	if(!IsMsgForThisTeam(buffer, length))
		return;

	STOCMessage msg(StoC::Msg::GameMsg);

	std::string newMsg;
	if(StripMessageKnowledge(buffer, length, newMsg))
		msg.GetBM()->Write(std::make_pair((void*)&newMsg[0], length));
	else
		msg.GetBM()->Write(std::make_pair(buffer, length));
	msg.Encode();

	HandleBeforeMsgQueries(buffer, length);

	for(auto& player : players)
		player.second->PushBackMsg(msg);

	HandleAfterMsgQueries(buffer, length);
}

} // namespace Legacy
} // namespace YGOpen
