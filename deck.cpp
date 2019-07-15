#include <unordered_map>

#include "deck.hpp"
#include "database_manager.hpp"
#include "banlist.hpp"

namespace YGOpen
{

uint32_t Deck::Verify(const DatabaseManager& dbm)
{
	for(auto& card : main)
	{
		if(dbm.GetCardDataByCode(card) == nullptr)
		{
			verified = false;
			return card;
		}
	}

	for(auto& card : extra)
	{
		if(dbm.GetCardDataByCode(card) == nullptr)
		{
			verified = false;
			return card;
		}
	}

	for(auto& card : side)
	{
		if(dbm.GetCardDataByCode(card) == nullptr)
		{
			verified = false;
			return card;
		}
	}

	verified = true;
	return 0;
}

bool Deck::IsVerified() const
{
	return verified;
}

unsigned int Deck::CheckUsability(const Banlist& bl, const int minmd, const int maxmd, const int mined, const int maxed, const int minsd, const int maxsd)
{
	if((int)main.size() < minmd || (int)main.size() > maxmd)
	{
		usable = false;
		return 1;
	}

	if((int)extra.size() < mined || (int)extra.size() > maxed)
	{
		usable = false;
		return 1;
	}

	if((int)side.size() < minsd || (int)side.size() > maxsd)
	{
		usable = false;
		return 1;
	}

	std::unordered_map<unsigned int, int> deckCardCount;

	for(auto& code : main)
	{
		if(deckCardCount.find(code) != deckCardCount.end())
			deckCardCount.insert(std::make_pair(code, 0));
		else
			deckCardCount[code]++;
	}

	for(auto& code : extra)
	{
		if(deckCardCount.find(code) != deckCardCount.end())
			deckCardCount.insert(std::make_pair(code, 0));
		else
			deckCardCount[code]++;
	}

	for(auto& code : side)
	{
		if(deckCardCount.find(code) != deckCardCount.end())
			deckCardCount.insert(std::make_pair(code, 0));
		else
			deckCardCount[code]++;
	}

	for(auto& p : deckCardCount)
	{
		if(bl.forbidden.find(p.first) != bl.forbidden.end())
		{
			usable = false;
			return p.first;
		}

		if(bl.limited.find(p.first) != bl.limited.end() && p.second > 1)
		{
			usable = false;
			return p.first;
		}

		if(bl.semilimited.find(p.first) != bl.semilimited.end() && p.second > 2)
		{
			usable = false;
			return p.first;
		}

		if(p.second > 3)
		{
			usable = false;
			return p.first;
		}

		if(bl.GetMode() == Banlist::MODE_WHITELIST && bl.whitelist.find(p.first) == bl.whitelist.end())
		{
			usable = false;
			return p.first;
		}
	}

	usable = true;
	return 0;
}

bool Deck::CanBeUsed() const
{
	return usable;
}

} // namespace YGOpen
