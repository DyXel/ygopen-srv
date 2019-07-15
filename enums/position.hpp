#ifndef __POSITION_HPP__
#define __POSITION_HPP__

enum Position
{
	PositionFaceUpAttack    = 0x1,
	PositionFaceDownAttack  = 0x2,
	PositionFaceUpDefense   = 0x4,
	PositionFaceDownDefense = 0x8,
	PositionFaceUp          = 0x5,
	PositionFaceDown        = 0xA,
	PositionAttack          = 0x3,
	PositionDefense         = 0xC
};

#endif // __POSITION_HPP__
