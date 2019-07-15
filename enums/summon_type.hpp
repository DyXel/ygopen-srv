#ifndef __REASON_TYPE_HPP__
#define __REASON_TYPE_HPP__

enum SummonType
{
	SummonTypeNormal   = 0x10000000,
	SummonTypeAdvance  = 0x11000000, // Tribute Summon
	SummonTypeDual     = 0x12000000, // Gemini Summon (Normal Summon again)
	SummonTypeFlip     = 0x20000000,
	SummonTypeSpecial  = 0x40000000,
	SummonTypeFusion   = 0x43000000,
	SummonTypeRitual   = 0x45000000,
	SummonTypeSynchro  = 0x46000000,
	SummonTypeXyz      = 0x49000000,
	SummonTypePendulum = 0x4A000000,
	SummonTypeLink     = 0x4C000000
};

#endif // __REASON_TYPE_HPP__
