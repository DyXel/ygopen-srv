#ifndef __DECK_HPP__
#define __DECK_HPP__
#include <vector>

class Deck
{
	bool verified; // The deck doesn't have any missing cards
	bool canBeUsed; // The deck doesn't have any banned/over limit card
	std::vector<uint32_t> main;
	std::vector<uint32_t> extra;
	std::vector<uint32_t> side;
public:
	void SetMainDeck(const std::vector<uint32_t>& deck)
	{
		main = deck;
	}
	void SetExtraDeck(const std::vector<uint32_t>& deck);
	void SetSideDeck(const std::vector<uint32_t>& deck)
	{
		side = deck;
	}

	bool Verify();
	bool isVerified() const;

	bool CheckBanlist();
	bool CanBeUsed() const;
};
#endif // __DECK_HPP__
