#ifndef __CARD_HPP__
#define __CARD_HPP__
#include <string>

namespace YGOpen
{

struct CardData
{
	unsigned int code;
	unsigned int alias;
	unsigned long long setcode;
	unsigned int type;
	unsigned int level;
	unsigned int attribute;
	unsigned int race;
	int attack;
	int defense;
	unsigned int lscale;
	unsigned int rscale;
	unsigned int link_marker;
};

struct CardDataExtra
{
	unsigned int code;
	unsigned int ot;
	unsigned int category;
};

struct CardStrings
{
	unsigned int code;
	std::string name;
	std::string desc;
	std::string str[16];
};

} // namespace YGOpen

#endif // __CARD_HPP__
