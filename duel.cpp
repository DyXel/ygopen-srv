#include <cassert>
#include <cstdint>
#include <map>
#include <algorithm> //std::max

#include "duel.hpp"

#include "core_interface.hpp"

#include "duel_observer.hpp"

namespace YGOpen
{

// NOTE: commented messages are not sent by the core, but instead are crafted by the server
static const std::map<CoreMessage, DuelMessage> msgResults =
{
	{CoreMessage::Retry, DuelMessage::NeedResponse},
	{CoreMessage::Hint, DuelMessage::Continue},
	//{CoreMessage::Waiting, }, 
	//{CoreMessage::Start, }, 
	{CoreMessage::Win, DuelMessage::EndOfDuel},
	//{CoreMessage::UpdateData, DuelMessage::Continue},
	//{CoreMessage::UpdateCard, DuelMessage::Continue},
	//{CoreMessage::RequestDeck, DuelMessage::Continue},
	{CoreMessage::SelectBattleCmd, DuelMessage::NeedResponse},
	{CoreMessage::SelectIdleCmd, DuelMessage::NeedResponse},
	{CoreMessage::SelectEffectYn, DuelMessage::NeedResponse},
	{CoreMessage::SelectYesNo, DuelMessage::NeedResponse},
	{CoreMessage::SelectOption, DuelMessage::NeedResponse},
	{CoreMessage::SelectCard, DuelMessage::NeedResponse},
	{CoreMessage::SelectChain, DuelMessage::NeedResponse},
	{CoreMessage::SelectPlace, DuelMessage::NeedResponse},
	{CoreMessage::SelectPosition, DuelMessage::NeedResponse},
	{CoreMessage::SelectTribute, DuelMessage::NeedResponse},
	{CoreMessage::SortChain, DuelMessage::NeedResponse},
	{CoreMessage::SelectCounter, DuelMessage::NeedResponse},
	{CoreMessage::SelectSum, DuelMessage::NeedResponse},
	{CoreMessage::SelectDisfield, DuelMessage::NeedResponse},
	{CoreMessage::SortCard, DuelMessage::NeedResponse},
	{CoreMessage::ConfirmDecktop, DuelMessage::Continue},
	{CoreMessage::ConfirmCards, DuelMessage::Continue},
	{CoreMessage::ShuffleDeck, DuelMessage::Continue},
	{CoreMessage::ShuffleHand, DuelMessage::Continue},
	{CoreMessage::RefreshDeck, DuelMessage::Continue},
	{CoreMessage::SwapGraveDeck, DuelMessage::Continue},
	{CoreMessage::ShuffleSetCard, DuelMessage::Continue},
	{CoreMessage::ReverseDeck, DuelMessage::Continue},
	{CoreMessage::DeckTop, DuelMessage::Continue},
	{CoreMessage::NewTurn, DuelMessage::Continue},
	{CoreMessage::NewPhase, DuelMessage::Continue},
	{CoreMessage::Move, DuelMessage::Continue},
	{CoreMessage::PosChange, DuelMessage::Continue},
	{CoreMessage::Set, DuelMessage::Continue},
	{CoreMessage::Swap, DuelMessage::Continue},
	{CoreMessage::FieldDisabled, DuelMessage::Continue},
	{CoreMessage::Summoning, DuelMessage::Continue},
	{CoreMessage::Summoned, DuelMessage::Continue},
	{CoreMessage::SpSummoning, DuelMessage::Continue},
	{CoreMessage::SpSummoned, DuelMessage::Continue},
	{CoreMessage::FlipSummoning, DuelMessage::Continue},
	{CoreMessage::FlipSummoned, DuelMessage::Continue},
	{CoreMessage::Chaining, DuelMessage::Continue},
	{CoreMessage::Chained, DuelMessage::Continue},
	{CoreMessage::ChainSolving, DuelMessage::Continue},
	{CoreMessage::ChainSolved, DuelMessage::Continue},
	{CoreMessage::ChainEnd, DuelMessage::Continue},
	{CoreMessage::ChainNegated, DuelMessage::Continue},
	{CoreMessage::ChainDisabled, DuelMessage::Continue},
	{CoreMessage::CardSelected, DuelMessage::Continue},
	{CoreMessage::RandomSelected, DuelMessage::Continue},
	{CoreMessage::BecomeTarget, DuelMessage::Continue},
	{CoreMessage::Draw, DuelMessage::Continue},
	{CoreMessage::Damage, DuelMessage::Continue},
	{CoreMessage::Recover, DuelMessage::Continue},
	{CoreMessage::Equip, DuelMessage::Continue},
	{CoreMessage::LpUpdate, DuelMessage::Continue},
	{CoreMessage::Unequip, DuelMessage::Continue},
	{CoreMessage::CardTarget, DuelMessage::Continue},
	{CoreMessage::CancelTarget, DuelMessage::Continue},
	{CoreMessage::PayLpCost, DuelMessage::Continue},
	{CoreMessage::AddCounter, DuelMessage::Continue},
	{CoreMessage::RemoveCounter, DuelMessage::Continue},
	{CoreMessage::Attack, DuelMessage::Continue},
	{CoreMessage::Battle, DuelMessage::Continue},
	{CoreMessage::AttackDisabled, DuelMessage::Continue},
	{CoreMessage::DamageStepStart, DuelMessage::Continue},
	{CoreMessage::DamageStepEnd, DuelMessage::Continue},
	{CoreMessage::MissedEffect, DuelMessage::Continue},
	{CoreMessage::BeChainTarget, DuelMessage::Continue},
	{CoreMessage::CreateRelation, DuelMessage::Continue},
	{CoreMessage::ReleaseRelation, DuelMessage::Continue},
	{CoreMessage::TossCoin, DuelMessage::Continue},
	{CoreMessage::TossDice, DuelMessage::Continue},
	{CoreMessage::RockPaperScissors, DuelMessage::NeedResponse},
	{CoreMessage::HandResult, DuelMessage::Continue},
	{CoreMessage::AnnounceRace, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceAttrib, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceCard, DuelMessage::NeedResponse},
	{CoreMessage::AnnounceNumber, DuelMessage::NeedResponse},
	{CoreMessage::CardHint, DuelMessage::Continue},
	{CoreMessage::TagSwap, DuelMessage::Continue},
	{CoreMessage::ReloadField, DuelMessage::Continue},
	{CoreMessage::AiName, DuelMessage::Continue},
	{CoreMessage::ShowHint, DuelMessage::Continue},
	{CoreMessage::PlayerHint, DuelMessage::Continue},
	{CoreMessage::MatchKill, DuelMessage::Continue},
	//{CoreMessage::CustomMsg, DuelMessage::Continue},
	{CoreMessage::SelectUnselect, DuelMessage::NeedResponse}
};

// Fixed-length Core Messages
static const std::map<CoreMessage, unsigned int> msgLengths =
{ 
	{CoreMessage::Hint           , 10},
	{CoreMessage::ShuffleDeck    , 1},
	{CoreMessage::RefreshDeck    , 1},
	{CoreMessage::SwapGraveDeck  , 1},
	{CoreMessage::ReverseDeck    , 0},
	{CoreMessage::DeckTop        , 9},
	{CoreMessage::NewTurn        , 1},
	{CoreMessage::NewPhase       , 2},
	{CoreMessage::Move           , 28},
	{CoreMessage::PosChange      , 9},
	{CoreMessage::Set            , 14},
	{CoreMessage::Swap           , 28},
	{CoreMessage::FieldDisabled  , 4},
	{CoreMessage::Summoning      , 14},
	{CoreMessage::Summoned       , 0},
	{CoreMessage::SpSummoning    , 14},
	{CoreMessage::SpSummoned     , 0},
	{CoreMessage::FlipSummoning  , 14},
	{CoreMessage::FlipSummoned   , 0},
	{CoreMessage::Chaining       , 32},
	{CoreMessage::Chained        , 1},
	{CoreMessage::ChainSolving   , 1},
	{CoreMessage::ChainSolved    , 1},
	{CoreMessage::ChainEnd       , 0},
	{CoreMessage::ChainNegated   , 1},
	{CoreMessage::ChainDisabled  , 1},
	{CoreMessage::Damage         , 5},
	{CoreMessage::Recover        , 5},
	{CoreMessage::Equip          , 20},
	{CoreMessage::LpUpdate       , 5},
	{CoreMessage::Unequip        , 10},
	{CoreMessage::CardTarget     , 20},
	{CoreMessage::CancelTarget   , 20},
	{CoreMessage::PayLpCost      , 5},
	{CoreMessage::AddCounter     , 7},
	{CoreMessage::RemoveCounter  , 7},
	{CoreMessage::Attack         , 20},
	{CoreMessage::Battle         , 38},
	{CoreMessage::AttackDisabled , 0},
	{CoreMessage::DamageStepStart, 0},
	{CoreMessage::DamageStepEnd  , 0},
	{CoreMessage::MissedEffect   , 14},
	{CoreMessage::HandResult     , 1},
	{CoreMessage::CardHint       , 19},
	{CoreMessage::PlayerHint     , 10},
	{CoreMessage::MatchKill      , 1}
};

Duel::Duel(CoreInterface& core, unsigned int seed) :
	core(core),
	pduel(0)
{
	pduel = core.create_duel(seed);
}

Duel::~Duel()
{
	core.end_duel(pduel);
}

void Duel::SetPlayerInfo(int playerID, int startLP, int startHand, int drawCount)
{
	core.set_player_info(pduel, playerID, startLP, startHand, drawCount);
}

void Duel::Start(int options)
{
	core.start_duel(pduel, options);
}

void Duel::PreloadScript(const std::string& file)
{
	core.preload_script(pduel, (char*)file.c_str(), 0, 0, nullptr);
}

void Duel::Process()
{
	DuelMessage lastMessage = DuelMessage::Continue;
	while (true) 
	{
		/*const int processorFlag = */core.process(pduel);
		const int bufferLength = core.get_message(pduel, nullptr);

		if (bufferLength > 0)
		{
			buffer.resize(bufferLength);
			core.get_message(pduel, buffer.data());
			lastMessage = Analyze(bufferLength);
		}

		if(lastMessage != DuelMessage::Continue)
			return;
	}
}

void Duel::NewCard(int code, int owner, int playerID, int location, int sequence, int position, int duelist)
{
	core.new_card(pduel, code, owner, playerID, location, sequence, position, duelist);
}

std::pair<void*, size_t> Duel::QueryCard(int playerID, int location, int sequence, int queryFlag, bool useCache)
{
	const size_t bufferLength = core.query_card(pduel, playerID, location, sequence, queryFlag, nullptr, useCache, 0);
	queryBuffer.resize(bufferLength);
	core.get_cached_query(pduel, queryBuffer.data());
	return std::make_pair((void*)queryBuffer.data(), bufferLength);
}

int Duel::QueryFieldCount(int playerID, int location)
{
	return core.query_field_count(pduel, playerID, location);
}

std::pair<void*, size_t> Duel::QueryFieldCard(int playerID, int location, int queryFlag, bool useCache)
{
	const size_t bufferLength = core.query_field_card(pduel, playerID, location, queryFlag, nullptr, useCache, 0);
	queryBuffer.resize(bufferLength);
	core.get_cached_query(pduel, queryBuffer.data());
	return std::make_pair((void*)queryBuffer.data(), bufferLength);
}

std::pair<void*, size_t> Duel::QueryFieldInfo()
{
	const size_t bufferLength = core.query_field_info(pduel, nullptr);
	queryBuffer.resize(bufferLength);
	core.get_cached_query(pduel, queryBuffer.data());
	return std::make_pair((void*)queryBuffer.data(), bufferLength);
}

void Duel::SetResponseInteger(int val)
{
	core.set_responsei(pduel, val);
}

void Duel::SetResponseBuffer(void* buff, size_t length)
{
	size_t resLength = std::max(static_cast<size_t>(64), length);
	void* b = std::calloc(1, resLength);
	std::memcpy(b, buff, length);
	core.set_responseb(pduel, (unsigned char*)b, resLength);
	std::free(b);
}

DuelMessage Duel::Analyze(unsigned int bufferLen)
{
	BufferManipulator bm(buffer.data(), bufferLen);
	while(bm.CanAdvance())
	{
		// Notify all observers about a new message
		auto cb = bm.GetCurrentBuffer();
		Message((void*)cb.first, cb.second);

		const CoreMessage msgType = (CoreMessage)bm.Read<uint8_t>();

		const DuelMessage msgResult = HandleCoreMessage(msgType, &bm);

		if(msgResult != DuelMessage::Continue)
			return msgResult;
	}
	return DuelMessage::Continue;
}

DuelMessage Duel::HandleCoreMessage(CoreMessage msgType, BufferManipulator* bm)
{
	// Check the message response, and return if we need a user input
	auto search = msgResults.find(msgType);
	if(search != msgResults.end())
	{
		if(search->second == DuelMessage::NeedResponse ||
		   search->second == DuelMessage::EndOfDuel)
			return search->second;
	}
	else
	{
		puts("\tMessage not handled");
		std::abort();
	}
	
	// Its a Continue message,
	// Forward BufferManipulator to the next message, if any.
	auto search2 = msgLengths.find(msgType);
	if(search2 != msgLengths.end())
	{
		// Fixed size messages
		bm->Forward(search2->second);
	}
	else
	{
		// Non-fixed size messages
		switch(msgType)
		{
			case CoreMessage::ConfirmDecktop:
			case CoreMessage::ConfirmExtratop:
			case CoreMessage::ConfirmCards:
			case CoreMessage::RandomSelected:
				bm->Forward(1);
				bm->Forward(bm->Read<uint32_t>() * 10);
			break;
			case CoreMessage::ShuffleHand:
			case CoreMessage::ShuffleExtra:
			case CoreMessage::CardSelected:
			case CoreMessage::Draw:
				bm->Forward(1);
				bm->Forward(bm->Read<uint32_t>() * 4);
			break;
			case CoreMessage::ShuffleSetCard:
				bm->Forward(1);
				bm->Forward(bm->Read<uint8_t>() * 20);
			break;
			case CoreMessage::TossCoin:
			case CoreMessage::TossDice:
				bm->Forward(1);
				bm->Forward(bm->Read<uint8_t>());
			break;
			case CoreMessage::BecomeTarget:
				bm->Forward(bm->Read<uint32_t>() * 10);
			break;
			
			default:
				puts("\tNon-fixed forwarding not handled");
				std::abort();
			break;
		}
	}

	return DuelMessage::Continue;
}

// Messages functions
void Duel::Message(void* buff, size_t length)
{
	for(auto obs : observers)
		obs->OnNotify(buff, length);
}

void Duel::AddObserver(DuelObserver* duelObs)
{
	observers.push_back(duelObs);
}

} // namespace YGOpen
