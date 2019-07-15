#ifndef __DECK_HPP__
#define __DECK_HPP__
#include <vector>
#include <cstdint>

namespace YGOpen
{

class DatabaseManager;
class Banlist;

class Deck
{
public:
	std::vector<unsigned int> main;
	std::vector<unsigned int> extra;
	std::vector<unsigned int> side;

	uint32_t Verify(const DatabaseManager& dbm);
	bool IsVerified() const;

	uint32_t CheckUsability(const Banlist& bl, const int minmd = 40, const int maxmd = 60, const int mined = 0, const int maxed = 15, const int minsd = 0, const int maxsd = 15);
	bool CanBeUsed() const;
private:
	bool verified{}; // The deck doesn't have any missing cards
	bool usable{}; // The deck doesn't have any banned/over limit card
};

} // namespace YGOpen

#endif // __DECK_HPP__
